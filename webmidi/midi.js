'use strict';

// Web MIDI access for the GT-3 editor - the browser-side replacement for
// midiIO.cpp (RtMidi). Taken over from the ME-5 editor (D:\BOSSME5\webmidi\midi.js)
// and adapted to the GT-3: 32 device IDs, 23-message patches, paced sends.
//
// SysEx needs explicit permission: requestMIDIAccess({ sysex: true }) makes the
// browser ask the user once. Supported in Chrome/Edge/Opera (desktop and
// Android) and in Firefox; not in Safari / iOS.

const Midi = (() => {
  const DEFAULT_TIMEOUT_MS = 300;
  // midiIO.cpp sleeps 5 ms after every message. Web MIDI queues sends, so the
  // gap is scheduled with timestamps instead: the time the bytes take on the
  // 31250 baud wire (320 us per byte) plus that pause.
  const WIRE_MS_PER_BYTE = 0.32;
  const GAP_MS = 5;
  let access = null;

  async function init() {
    if (!navigator.requestMIDIAccess) {
      throw new Error('This browser has no Web MIDI - use Chrome or Edge (Safari and iOS do not support it).');
    }
    if (!access) {
      try {
        access = await navigator.requestMIDIAccess({ sysex: true });
      } catch (e) {
        throw new Error(`MIDI access denied (${e.name}). Allow "MIDI devices" / SysEx for this page in the site settings.`);
      }
    }
    return access;
  }

  const outputs = () => [...access.outputs.values()];
  const inputs = () => [...access.inputs.values()];
  const byName = (ports, name) => ports.find(p => p.name === name);

  // Called with no arguments when ports come and go (interface plugged in or out).
  function onPortsChanged(handler) {
    if (access) access.onstatechange = () => handler();
  }

  function findPorts({ output, input }) {
    const out = byName(outputs(), output);
    if (!out) throw new Error(`output port not found: ${output}`);
    const inp = input ? byName(inputs(), input) : null;
    if (input && !inp) throw new Error(`input port not found: ${input}`);
    return { out, inp };
  }

  // Send messages one after the other with the GT-3's breathing gap between them.
  // Resolves when the last one should be on the wire.
  function sendPaced(out, messages) {
    let t = performance.now();
    for (const m of messages) {
      out.send(m, t);
      t += m.length * WIRE_MS_PER_BYTE + GAP_MS;
    }
    return new Promise(resolve => setTimeout(resolve, Math.max(0, t - performance.now())));
  }

  // Fire-and-forget send (program change, live edits, patch writes).
  function send(outputName, messages) {
    const { out } = findPorts({ output: outputName });
    return sendPaced(out, messages instanceof Uint8Array ? [messages] : messages);
  }

  // Send `messages` to `out` and collect the SysEx that arrives on `ins` until
  // nothing new has come for timeoutMs (the wait restarts on every reply), or
  // until stopWhen(bytesSoFar) says enough. Channel messages and realtime bytes
  // the GT-3 sends meanwhile are dropped.
  async function sendAndCollect(out, ins, messages, { timeoutMs = DEFAULT_TIMEOUT_MS, stopWhen } = {}) {
    await out.open();
    // An input another program holds open can't be opened on Windows - listen on the rest.
    const opened = await Promise.allSettled(ins.map(i => i.open()));
    ins = ins.filter((_, k) => opened[k].status === 'fulfilled');
    if (!ins.length) throw new Error('could not open any MIDI input - is another program using it?');
    const received = new Map(ins.map(i => [i, []]));
    return new Promise(resolve => {
      let timer;
      const finish = () => {
        clearTimeout(timer);
        for (const i of ins) i.removeEventListener('midimessage', handlers.get(i));
        resolve(received);
      };
      const restart = () => { clearTimeout(timer); timer = setTimeout(finish, timeoutMs); };
      // Web MIDI hands over whole SysEx messages; a long one may still come in
      // pieces on some systems, so continuation bytes (< 0x80) are kept too.
      const handlers = new Map(ins.map(i => [i, e => {
        const first = e.data[0];
        if (first !== 0xF0 && first >= 0x80) return;
        const bytes = received.get(i);
        for (const b of e.data) bytes.push(b);
        if (stopWhen && stopWhen(bytes)) finish(); else restart();
      }]));
      for (const i of ins) i.addEventListener('midimessage', handlers.get(i));
      sendPaced(out, messages).then(restart);
      restart();
    });
  }

  async function request(conn, messages, options) {
    const { out, inp } = findPorts(conn);
    if (!inp) throw new Error('no input port selected - SysEx replies need one');
    return (await sendAndCollect(out, [inp], [].concat(messages), options)).get(inp);
  }

  // Find the port pair and device ID the GT-3 answers on. It only answers SysEx
  // addressed to its own device ID, so the identity request goes out with all
  // 32, and the reply's device ID says which one it is.
  // Listens on every input at once: one wait per output, not per output x input pair.
  async function detectGt3() {
    const requests = Array.from({ length: GT3.DEVICE_IDS }, (_, dev) => GT3.identityRequest(dev));
    for (const out of outputs()) {
      let replies;
      try {
        replies = await sendAndCollect(out, inputs(), requests);
      } catch {
        continue; // port busy (e.g. held by another program) or vanished
      }
      for (const [inp, bytes] of replies) {
        const dev = GT3.replyDevice(bytes);
        if (dev !== null) return { output: out.name, input: inp.name, dev };
      }
    }
    return null;
  }

  // Count the complete GT-3 DT1 messages in `bytes`.
  const dt1Count = bytes => GT3.splitSysex(bytes).filter(m => m[1] === 0x41 && m[5] === 0x12).length;

  // One patch (null = the temporary patch) -> GT3.decodePatch result.
  async function readPatch(conn, n = null) {
    const bytes = await request(conn, GT3.patchRequest(conn.dev, n), {
      timeoutMs: 600,
      stopWhen: b => b[b.length - 1] === 0xF7 && dt1Count(b) >= GT3.PATCH_PACKETS,
    });
    const messages = GT3.splitSysex(bytes).filter(m => m[1] === 0x41 && m[5] === 0x12);
    if (messages.length < GT3.PATCH_PACKETS) {
      throw new Error(`expected ${GT3.PATCH_PACKETS} patch messages from the GT-3, got ${messages.length}`);
    }
    const result = GT3.decodePatch(messages);
    if (result.missing.length) throw new Error(`the GT-3 did not send blocks ${result.missing.map(GT3.hex).join(' ')}`);
    return result;
  }

  // The raw system-area dump (all DT1 messages the GT-3 sends back).
  async function readSystem(conn) {
    const bytes = await request(conn, GT3.systemRequest(conn.dev), { timeoutMs: 800 });
    const messages = GT3.splitSysex(bytes).filter(m => m[1] === 0x41 && m[5] === 0x12);
    if (!messages.length) throw new Error('the GT-3 sent no system data');
    return messages;
  }

  // Load a patch into the GT-3's temporary patch (what it plays, not stored).
  const sendTemp = (conn, patch) => send(conn.output, GT3.encodePatch(patch, { to: 'temp', dev: conn.dev }));

  // Store a patch into user memory n (0-139).
  function writePatch(conn, patch, n) {
    if (!GT3.isUser(n)) throw new Error(`${GT3.patchLabel(n)} is a preset - only user patches can be written`);
    return send(conn.output, GT3.encodePatch(patch, { to: n, dev: conn.dev }));
  }

  const selectPatch = (conn, n, channel = 0) => send(conn.output, GT3.programChange(n, channel));

  // Calls onPatch(n) whenever the GT-3 sends a patch change (picked on the unit): bank
  // select CC#0 then program change. Listens on one input by name, or on every input.
  // Returns a function that stops listening.
  function watchPatchChanges(onPatch, inputName = null) {
    const ins = inputs().filter(i => inputName === null || i.name === inputName);
    const banks = new Map();
    const handler = e => {
      const [status, a, b] = e.data;
      if ((status & 0xF0) === 0xB0 && a === 0x00) banks.set(e.currentTarget, b);
      else if ((status & 0xF0) === 0xC0) {
        const n = GT3.patchFromProgram(banks.get(e.currentTarget) ?? 0, a);
        if (n !== null) onPatch(n);
      }
    };
    for (const i of ins) {
      i.addEventListener('midimessage', handler);
      i.open().catch(() => {}); // held by another program: nothing to hear there
    }
    return () => { for (const i of ins) i.removeEventListener('midimessage', handler); };
  }

  return {
    init, outputs, inputs, onPortsChanged, send, detectGt3,
    readPatch, readSystem, sendTemp, writePatch, selectPatch, watchPatchChanges,
  };
})();
