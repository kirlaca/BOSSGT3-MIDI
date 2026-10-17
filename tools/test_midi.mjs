// Runs webmidi/midi.js against a simulated GT-3 behind a fake Web MIDI API:
// port detection, device ID detection, patch read, patch write, program change.
//
//     node tools/test_midi.mjs

import fs from 'node:fs';
import path from 'node:path';
import vm from 'node:vm';
import { fileURLToPath } from 'node:url';

const BASE = path.dirname(path.dirname(fileURLToPath(import.meta.url)));
const WEB = path.join(BASE, 'webmidi');

// ---------------------------------------------------------------- fake Web MIDI + GT-3

const GT3_DEV = 0x10;   // the simulated unit answers only on device ID 10 (17)
const sent = [];        // everything the GT-3 received

class FakeInput extends EventTarget {
  constructor(name) { super(); this.name = name; }
  async open() {}
  deliver(bytes) {
    const e = new Event('midimessage');
    e.data = Uint8Array.from(bytes);
    // Real Web MIDI is asynchronous; so is this.
    setTimeout(() => this.dispatchEvent(e), 2);
  }
}

class FakeOutput {
  constructor(name, onSend) { this.name = name; this.onSend = onSend; }
  async open() {}
  send(data) { this.onSend(Uint8Array.from(data)); }
}

let GT3;   // filled once the editor scripts are loaded
let stored = null;   // the user patch the fake GT-3 holds at U01-1
const gt3In = new FakeInput('GT-3 MIDI IN (from GT-3)');
const otherIn = new FakeInput('Other synth IN');
function gt3Receives(m) {
  sent.push(m);
  if (m[0] !== 0xF0) return;
  if (!(m[1] === 0x41 && m[3] === 0x00 && m[4] === 0x1B && m[2] === GT3_DEV)) return; // not for us
  if (m[5] === 0x11) {
    const address = Array.from(m.slice(6, 10));
    if (address.every(b => b === 0) && m[10] === 0x02) {
      // The whole system area: system.syx, sent with the unit's own device ID.
      for (const c of GT3.decodeSystem(GT3.systemSyx())) gt3In.deliver(GT3.dt1(GT3_DEV, c.address, c.data));
    } else if (address.every(b => b === 0) && m[13] === 1) {
      gt3In.deliver(GT3.dt1(GT3_DEV, [0, 0, 0, 0], [0x05]));        // tuner pitch 440 Hz
    } else if (address[2] === 0 && address[3] === 0) {
      const n = GT3.patchFromArea(address[0], address[1]);
      const patch = n === 0 && stored ? stored : GT3.defaultPatch();
      // The GT-3 answers with its own patch address in every message.
      const to = n === null ? 'temp' : n;
      for (const msg of GT3.encodePatch(patch, { to, dev: GT3_DEV })) gt3In.deliver(msg);
    }
  } else if (m[5] === 0x12) {
    // Collect a patch write into U01-1.
    const { address, data } = GT3.parseDt1(m);
    if (address[0] === 0x07 && address[1] === 0x00) {
      stored = stored || GT3.defaultPatch();
      stored.blocks[address[2]].set(data, address[3]);
    }
  }
}
const access = {
  outputs: new Map([
    ['o1', new FakeOutput('Other synth OUT', () => {})],
    ['o2', new FakeOutput('GT-3 MIDI OUT (to GT-3)', gt3Receives)],
  ]),
  inputs: new Map([['i1', otherIn], ['i2', gt3In]]),
};

const ctx = vm.createContext({
  console, atob, performance, setTimeout, clearTimeout, Event, EventTarget,
  navigator: { requestMIDIAccess: async opts => { if (!opts?.sysex) throw new Error('sysex not asked'); return access; } },
});
for (const f of ['gt3-params.js', 'factory.js', 'gt3.js', 'midi.js']) {
  vm.runInContext(fs.readFileSync(path.join(WEB, f), 'utf8'), ctx, { filename: f });
}
GT3 = vm.runInContext('GT3', ctx);
const Midi = vm.runInContext('Midi', ctx);

// ---------------------------------------------------------------- tests

let failures = 0;
let checks = 0;
function check(ok, what) {
  checks++;
  console.log(`${ok ? 'ok  ' : 'FAIL'}  ${what}`);
  if (!ok) failures++;
}

await Midi.init();
const t0 = performance.now();
const conn = await Midi.detectGt3();
check(conn && conn.output === 'GT-3 MIDI OUT (to GT-3)' && conn.input === 'GT-3 MIDI IN (from GT-3)' && conn.dev === GT3_DEV,
  `detect: ${JSON.stringify(conn)} in ${Math.round(performance.now() - t0)} ms`);

const temp = await Midi.readPatch(conn);
check(temp.source === null && GT3.patchName(temp.patch) === GT3.patchName(GT3.defaultPatch()), `read temp patch: "${GT3.patchName(temp.patch)}"`);

const edited = GT3.defaultPatch();
GT3.setValue(edited, 0x03, 0x02, 88);
GT3.setPatchName(edited, 'Web MIDI');
sent.length = 0;
const t1 = performance.now();
await Midi.writePatch(conn, edited, 0);
const sysex = sent.filter(m => m[0] === 0xF0);
check(sysex.length === 23 && sysex.every(m => m[6] === 0x07 && m[7] === 0x00 && m[2] === GT3_DEV),
  `write U01-1: ${sysex.length} DT1 messages to 07 00, paced over ${Math.round(performance.now() - t1)} ms`);

const back = await Midi.readPatch(conn, 0);
check(back.source === 0 && GT3.patchName(back.patch) === 'Web MIDI' && GT3.getValue(back.patch, 0x03, 0x02) === 88,
  `read U01-1 back: "${GT3.patchName(back.patch)}", OD drive ${GT3.getValue(back.patch, 0x03, 0x02)}`);

let refused = false;
try { await Midi.writePatch(conn, edited, 200); } catch { refused = true; }
check(refused, 'writing a preset (P51-1) is refused');

sent.length = 0;
await Midi.selectPatch(conn, 150);
check(GT3.hexBytes(sent[0]) === 'B0 00 01 B0 20 00 C0 32', `program change to ${GT3.patchLabel(150)}: ${GT3.hexBytes(sent[0])}`);

const sysRead = GT3.decodeSystem(GT3.concat(await Midi.readSystem(conn)));
const tunerArea = GT3.systemArea('Tuner');
check(sysRead.length === GT3.decodeSystem(GT3.systemSyx()).length && GT3.getSystemValue(sysRead, tunerArea, 0) === 5,
  `read system area: ${sysRead.length} chunks, tuner ${GT3.label(tunerArea.params[0], GT3.getSystemValue(sysRead, tunerArea, 0))}`);

console.log(`\n${checks - failures}/${checks} checks passed${failures ? ` - ${failures} FAILED` : ''}`);
process.exit(failures ? 1 : 0);
