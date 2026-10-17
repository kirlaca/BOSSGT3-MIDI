'use strict';

// GT-3 SysEx protocol and patch model - the browser-side port of SysxIO.cpp /
// MidiTable.cpp. Parameter definitions come from gt3-params.js (generated from
// midi.xml by tools/gen_gt3_params.py).
//
// Message format (Roland DT1 / RQ1, 4-byte address, two-byte model ID):
//     F0 41 <dev> 00 1B 12 <a1 a2 a3 a4> <data...> <checksum> F7     (DT1, data set)
//     F0 41 <dev> 00 1B 11 <a1 a2 a3 a4> <s1 s2 s3 s4> <checksum> F7 (RQ1, data request)
// The checksum is the standard Roland one: it covers the address and the data
// (or size), so that (sum + checksum) % 128 === 0.
//
// A patch is one DT1 message per effect block: address <area> <area> <block> 00.
//     0C 00 bb 00   temporary patch - what the GT-3 is playing right now
//     07 nn bb 00   user patch nn (08 nn past the 128th); see patchAddress()
// Blocks 00-16 (23 messages, 650 bytes) are the GT-3's own. Blocks 17-19 hold
// the editor's patch description / creator / scratch pad: the FloorBoard keeps
// them in its .syx files behind a non-Roland header (F0 00 ...), the GT-3 never
// sees them.

const GT3 = (() => {
  const SOX = 0xF0;
  const EOX = 0xF7;
  const ROLAND_ID = 0x41;
  const MODEL_ID = [0x00, 0x1B];
  const CMD_RQ1 = 0x11;
  const CMD_DT1 = 0x12;
  const HEADER_LENGTH = 6;   // F0 41 dev 00 1B cmd
  const ADDRESS_LENGTH = 4;
  // The FloorBoard writes the editor-only blocks with manufacturer ID 00 and
  // area 0B 00, so no Roland unit ever takes them for its own data.
  const EDITOR_HEADER = [SOX, 0x00, 0x00, ...MODEL_ID, CMD_DT1];
  const EDITOR_AREA = [0x0B, 0x00];

  const DEVICE_IDS = 32;     // Roland device ID 00-1F
  const FILE_DEVICE_ID = 0;  // files are written with device ID 00, like default.syx

  const TEMP_AREA = [0x0C, 0x00];       // temporary patch: live edits go here
  const TEMP_READ_AREA = [0x0B, 0x00];  // temporary patch as read back (MidiTable::patchRequest)
  const USER_AREA_MSB = 0x07;

  const BLOCKS = GT3_PARAMS.blocks;
  const DEVICE_BLOCKS = BLOCKS.filter(b => !b.editorOnly);  // 00-16
  const BLOCK_COUNT = BLOCKS.length;                        // 26
  const PATCH_PACKETS = DEVICE_BLOCKS.length;               // 23
  const messageLength = b => HEADER_LENGTH + ADDRESS_LENGTH + b.size + 2;
  const PATCH_SIZE = DEVICE_BLOCKS.reduce((n, b) => n + messageLength(b), 0);  // 650
  const FILE_PATCH_SIZE = BLOCKS.reduce((n, b) => n + messageLength(b), 0);     // 878

  const NAME_BLOCK = 0x0C;
  const NAME_LENGTH = 11;
  const CHAIN_BLOCK = 0x0B;

  // Patch memory (globalVariables.h): 35 user banks + 50 preset banks, 4 patches each.
  const PATCHES_PER_BANK = 4;
  const USER_BANKS = 35;
  const ALL_BANKS = 85;
  const USER_PATCHES = USER_BANKS * PATCHES_PER_BANK;   // 140
  const ALL_PATCHES = ALL_BANKS * PATCHES_PER_BANK;     // 340
  // Presets start in a new 128-patch memory page: 128 - (140 - 128) unused slots after the users.
  const PRESET_GAP = 128 - (USER_PATCHES - 128);
  const PROGRAMS_PER_BANK_SELECT = 100;                 // bankSize: CC#0 steps every 100 patches

  console.assert(PATCH_PACKETS === 23 && PATCH_SIZE === 650 && FILE_PATCH_SIZE === 878);
  console.assert(BLOCKS.every((b, i) => b.id === i));

  class PatchError extends Error {}

  const hex = b => b.toString(16).toUpperCase().padStart(2, '0');
  const hexBytes = bytes => Array.from(bytes, hex).join(' ');

  const checksum = bytes => (128 - (Array.from(bytes).reduce((a, b) => a + b, 0) & 0x7F)) & 0x7F;

  function checkDevice(dev) {
    if (!(Number.isInteger(dev) && dev >= 0 && dev < DEVICE_IDS)) {
      throw new PatchError(`device ID must be 0-31, got ${dev}`);
    }
  }

  function dt1(dev, address, data) {
    checkDevice(dev);
    const body = [...address, ...data];
    return Uint8Array.from([SOX, ROLAND_ID, dev, ...MODEL_ID, CMD_DT1, ...body, checksum(body), EOX]);
  }

  // `size` is a byte count, sent as four 7-bit bytes like the address.
  function rq1(dev, address, size) {
    checkDevice(dev);
    const sizeBytes = [(size >> 21) & 0x7F, (size >> 14) & 0x7F, (size >> 7) & 0x7F, size & 0x7F];
    const body = [...address, ...sizeBytes];
    return Uint8Array.from([SOX, ROLAND_ID, dev, ...MODEL_ID, CMD_RQ1, ...body, checksum(body), EOX]);
  }

  // Split a byte stream into F0 ... F7 messages; bytes outside them are dropped.
  function splitSysex(bytes) {
    const messages = [];
    let start = -1;
    for (let i = 0; i < bytes.length; i++) {
      if (bytes[i] === SOX) start = i;
      else if (bytes[i] === EOX && start >= 0) {
        messages.push(Uint8Array.from(bytes.slice(start, i + 1)));
        start = -1;
      }
    }
    return messages;
  }

  const isGt3Header = m => m.length > HEADER_LENGTH && m[0] === SOX && m[1] === ROLAND_ID && m[2] < DEVICE_IDS
    && m[3] === MODEL_ID[0] && m[4] === MODEL_ID[1];
  const isEditorHeader = m => m.length > HEADER_LENGTH && EDITOR_HEADER.every((b, i) => m[i] === b);

  // {dev, address: [4], data, editor} of a DT1 message from or for a GT-3.
  function parseDt1(m, verifyChecksum = true) {
    const editor = isEditorHeader(m);
    if (!editor && !(isGt3Header(m) && m[5] === CMD_DT1)) throw new PatchError(`not a GT-3 DT1 message: ${hexBytes(m.slice(0, 6))}`);
    if (m.length < HEADER_LENGTH + ADDRESS_LENGTH + 2 || m[m.length - 1] !== EOX) throw new PatchError('truncated DT1 message');
    const body = m.slice(HEADER_LENGTH, m.length - 2);
    if (verifyChecksum && checksum(body) !== m[m.length - 2]) {
      throw new PatchError(`checksum mismatch at ${hexBytes(body.slice(0, 4))}: expected ${hex(checksum(body))}, got ${hex(m[m.length - 2])}`);
    }
    return { dev: editor ? null : m[2], address: Array.from(body.slice(0, 4)), data: body.slice(4), editor };
  }

  // ------------------------------------------------------------ patch memory

  // Patch number 0-339: 0-139 = user U1-1 .. U35-4, 140-339 = preset P36-1 .. P85-4.
  function checkPatch(n) {
    if (!(Number.isInteger(n) && n >= 0 && n < ALL_PATCHES)) throw new PatchError(`patch number out of range 0-339: ${n}`);
  }

  const isUser = n => n < USER_PATCHES;

  function patchLabel(n) {
    checkPatch(n);
    const bank = Math.floor(n / PATCHES_PER_BANK) + 1;
    return `${isUser(n) ? 'U' : 'P'}${String(bank).padStart(2, '0')}-${(n % PATCHES_PER_BANK) + 1}`;
  }

  // First two address bytes of patch n (MidiTable::patchRequest).
  function patchArea(n) {
    checkPatch(n);
    const slot = isUser(n) ? n : n + PRESET_GAP;
    return [USER_AREA_MSB + Math.floor(slot / 128), slot % 128];
  }

  // Inverse of patchArea; null for the temporary patch areas.
  function patchFromArea(a1, a2) {
    if ((a1 === TEMP_AREA[0] && a2 === TEMP_AREA[1]) || (a1 === TEMP_READ_AREA[0] && a2 === TEMP_READ_AREA[1])) return null;
    const slot = (a1 - USER_AREA_MSB) * 128 + a2;
    const n = slot < USER_PATCHES ? slot : slot - PRESET_GAP;
    if (a1 < USER_AREA_MSB || a2 > 0x7F || n < 0 || n >= ALL_PATCHES || (slot >= USER_PATCHES && n < USER_PATCHES)) {
      throw new PatchError(`not a patch address: ${hex(a1)} ${hex(a2)}`);
    }
    return n;
  }

  // Bank select + program change (SysxIO::getPatchChangeMsg): CC#0 = n / 100, CC#32 = 0, PC = n % 100.
  function programChange(n, channel = 0) {
    checkPatch(n);
    if (!(Number.isInteger(channel) && channel >= 0 && channel < 16)) throw new PatchError(`MIDI channel must be 0-15, got ${channel}`);
    return Uint8Array.from([
      0xB0 | channel, 0x00, Math.floor(n / PROGRAMS_PER_BANK_SELECT),
      0xB0 | channel, 0x20, 0x00,
      0xC0 | channel, n % PROGRAMS_PER_BANK_SELECT,
    ]);
  }

  // ------------------------------------------------------------ requests

  // The FloorBoard's "identity request" is a read of the first system byte (tuner
  // pitch); any GT-3 DT1 reply means a GT-3 is listening on that device ID.
  const identityRequest = dev => rq1(dev, [0x00, 0x00, 0x00, 0x00], 1);
  // The whole system area (globalVariables.h systemRequest: size 02 00 00 01).
  const systemRequest = dev => rq1(dev, [0x00, 0x00, 0x00, 0x00], (0x02 << 21) | 1);
  // All 23 blocks of a patch: size 00 00 1A 00 = blocks 00-19 (the GT-3 sends what it has).
  const PATCH_REQUEST_SIZE = BLOCK_COUNT << 7;
  const patchRequest = (dev, n = null) => rq1(dev, [...(n === null ? TEMP_READ_AREA : patchArea(n)), 0x00, 0x00], PATCH_REQUEST_SIZE);

  // Device ID of the first GT-3 DT1 message in `bytes`, or null.
  function replyDevice(bytes) {
    for (const m of splitSysex(bytes)) {
      if (isGt3Header(m) && m[5] === CMD_DT1) return m[2];
    }
    return null;
  }

  // ------------------------------------------------------------ patch model

  // A patch is {blocks: Uint8Array[26]} - the data bytes of every block, by block ID.
  const emptyPatch = () => ({ blocks: BLOCKS.map(b => new Uint8Array(b.size)) });
  const clonePatch = p => ({ blocks: p.blocks.map(b => b.slice()) });

  // Messages -> patch. Returns {patch, source, missing}: source is the patch number
  // the data came from (null = temporary patch / file without one), missing lists
  // the device blocks that were absent and taken from `fallback` (default: the init patch).
  function decodePatch(messages, { fallback = null, verifyChecksum = true } = {}) {
    const patch = clonePatch(fallback || defaultPatch());
    const seen = new Set();
    let source;
    for (const m of messages) {
      const { address, data, editor } = parseDt1(m, verifyChecksum);
      const [a1, a2, blockId, offset] = address;
      const block = BLOCKS[blockId];
      if (!block) throw new PatchError(`unknown block ${hex(blockId)}`);
      if (offset + data.length > block.size) {
        throw new PatchError(`block ${hex(blockId)}: ${data.length} bytes at offset ${offset} overflow its ${block.size} bytes`);
      }
      patch.blocks[blockId].set(data, offset);
      seen.add(blockId);
      if (!editor) {
        const n = patchFromArea(a1, a2);
        if (source === undefined) source = n;
        else if (source !== n) throw new PatchError('the messages belong to different patches');
      }
    }
    const missing = DEVICE_BLOCKS.filter(b => !seen.has(b.id)).map(b => b.id);
    return { patch, source: source === undefined ? null : source, missing };
  }

  // Patch -> DT1 messages. `to` = 'temp' (the patch the GT-3 is playing),
  // a patch number (store into that memory) or 'file' (temp address, device ID
  // 00, like default.syx). `editor` adds the editor-only blocks (only for files).
  function encodePatch(patch, { to = 'temp', dev = FILE_DEVICE_ID, editor = false } = {}) {
    const area = to === 'temp' || to === 'file' ? TEMP_AREA : patchArea(to);
    if (to === 'file') dev = FILE_DEVICE_ID;
    if (editor && to !== 'file') throw new PatchError('editor-only blocks belong in files, not on the GT-3');
    const messages = DEVICE_BLOCKS.map(b => dt1(dev, [...area, b.id, 0x00], patch.blocks[b.id]));
    if (editor) {
      for (const b of BLOCKS.filter(x => x.editorOnly)) {
        const body = [...EDITOR_AREA, b.id, 0x00, ...patch.blocks[b.id]];
        messages.push(Uint8Array.from([...EDITOR_HEADER, ...body, checksum(body), EOX]));
      }
    }
    return messages;
  }

  const concat = messages => {
    const out = new Uint8Array(messages.reduce((n, m) => n + m.length, 0));
    let i = 0;
    for (const m of messages) { out.set(m, i); i += m.length; }
    return out;
  };

  // A .syx file -> patches. One file may hold many patches back to back (a bulk
  // dump); a new patch starts whenever the block number stops increasing.
  function decodeFile(bytes) {
    const groups = [];
    let last = Infinity;
    for (const m of splitSysex(bytes)) {
      if (!isGt3Header(m) && !isEditorHeader(m)) continue;
      if (!isEditorHeader(m) && m[5] !== CMD_DT1) continue;
      const block = m[HEADER_LENGTH + 2];
      if (block <= last) groups.push([]);
      groups[groups.length - 1].push(m);
      last = block;
    }
    if (!groups.length) throw new PatchError('no GT-3 patch data in this file');
    return groups.map(g => decodePatch(g));
  }

  function encodeFile(patches, { editor = false } = {}) {
    return concat([].concat(patches).flatMap(p => encodePatch(p, { to: 'file', editor })));
  }

  // ------------------------------------------------------------ parameters

  const PARAM_INDEX = new Map();
  for (const b of BLOCKS) for (const p of b.params) PARAM_INDEX.set(b.id * 256 + p.offset, { ...p, block: b.id });

  function param(block, offset) {
    const p = PARAM_INDEX.get(block * 256 + offset);
    if (!p) throw new PatchError(`no parameter at block ${hex(block)} offset ${hex(offset)}`);
    return p;
  }

  // Two-byte values are MSB first, 7 bits each (SysxIO::getSourceValue).
  function getValue(patch, block, offset) {
    const p = param(block, offset);
    const d = patch.blocks[block];
    return p.size === 2 ? d[offset] * 128 + d[offset + 1] : d[offset];
  }

  const valueBytes = (p, value) => (p.size === 2 ? [value >> 7, value & 0x7F] : [value]);

  function checkValue(p, value) {
    if (!Number.isInteger(value) || value < p.min || value > p.max) {
      throw new PatchError(`${p.name || `${hex(p.block)}/${hex(p.offset)}`}: value ${value} out of range ${p.min}-${p.max}`);
    }
  }

  // Store `value` in the patch; returns the DT1 message that makes the GT-3's
  // temporary patch follow (MidiTable::dataChange: address 0C 00 block offset).
  function setValue(patch, block, offset, value, dev = FILE_DEVICE_ID) {
    const p = param(block, offset);
    checkValue(p, value);
    const bytes = valueBytes(p, value);
    patch.blocks[block].set(bytes, offset);
    return dt1(dev, [...TEMP_AREA, block, offset], bytes);
  }

  // Display text of a value, as MidiTable::rangeToValue builds it: a discrete
  // entry wins, else the first range that holds it maps it linearly.
  function label(p, value) {
    const table = GT3_PARAMS.tables[p.t];
    for (const e of table) if (Array.isArray(e) && e[0] === value) return e[1];
    for (const e of table) {
      if (Array.isArray(e) || value < e.lo || value > e.hi) continue;
      const base = e.base || 0;
      const start = e.lo - base;
      // Same arithmetic as the original (not min + (v - lo) * step), so labels match it exactly.
      const result = (value - base) * ((e.max - e.min) / (e.hi - e.lo)) + (e.min - start);
      return result.toFixed(e.dp) + (e.unit || '');
    }
    return String(value);
  }

  // Every valid value of a parameter with its label - for list boxes.
  function options(p) {
    const out = [];
    for (let v = p.min; v <= p.max; v++) {
      const table = GT3_PARAMS.tables[p.t];
      const known = table.some(e => (Array.isArray(e) ? e[0] === v : v >= e.lo && v <= e.hi));
      if (known) out.push([v, label(p, v)]);
    }
    return out;
  }

  // ------------------------------------------------------------ name, chain, targets

  function patchName(patch) {
    return String.fromCharCode(...patch.blocks[NAME_BLOCK].slice(0, NAME_LENGTH)).replace(/\s+$/, '');
  }

  // Returns the DT1 message for the temporary patch, like setValue.
  function setPatchName(patch, name, dev = FILE_DEVICE_ID) {
    const bytes = Array.from(name.slice(0, NAME_LENGTH).padEnd(NAME_LENGTH, ' '), c => {
      const code = c.charCodeAt(0);
      return code >= 0x20 && code <= 0x7F ? code : 0x3F; // '?' for anything the GT-3 can't show
    });
    patch.blocks[NAME_BLOCK].set(bytes, 0);
    return dt1(dev, [...TEMP_AREA, NAME_BLOCK, 0x00], bytes);
  }

  // Effect order: 12 slots, each holding an effect code (FX1, CS, WAH, OD, ... FV).
  const chain = patch => Array.from(patch.blocks[CHAIN_BLOCK]);
  const CHAIN_LENGTH = 12;
  const isValidChain = order => order.length === CHAIN_LENGTH && new Set(order).size === CHAIN_LENGTH
    && order.every(c => Number.isInteger(c) && c >= 0 && c < CHAIN_LENGTH);

  // A new effect order (every code exactly once); returns the DT1 message that
  // sends the whole chain block to the temporary patch.
  function setChain(patch, order, dev = FILE_DEVICE_ID) {
    if (!isValidChain(order)) throw new PatchError(`not an effect order: ${order.join(' ')}`);
    patch.blocks[CHAIN_BLOCK].set(order, 0);
    return dt1(dev, [...TEMP_AREA, CHAIN_BLOCK, 0x00], order);
  }

  const samePatch = (a, b) => a.blocks.every((d, i) => d.length === b.blocks[i].length && d.every((x, k) => x === b.blocks[i][k]));

  const TARGETS = GT3_PARAMS.targets.list.map(([code, block, offset, name]) => ({ code, block, offset, name }));
  const TARGET_BY_CODE = new Map(TARGETS.map(t => [t.code, t]));

  // What an Assign / EXP / CTL target code drives: {code, block, offset, name, param}.
  // The Min / Max values of that assign are shown in the target parameter's own units.
  function target(code) {
    const t = TARGET_BY_CODE.get(code);
    if (!t) return null;
    return { ...t, param: PARAM_INDEX.get(t.block * 256 + t.offset) || null };
  }

  // ------------------------------------------------------------ system settings

  // The system area (tuner, global, system, MIDI, manual) as it came in: one
  // {address: [4], data} chunk per DT1 message, keyed by address, so a dump read
  // from the GT-3 or a file goes back out byte for byte - parts midi.xml doesn't
  // describe (the program change map) included. Every system parameter is one byte.
  const SYSTEM_AREAS = GT3_PARAMS.system.filter(s => s.params.length);
  const SYSTEM_LAST_MSB = 0x02;  // system data lives below the patch areas (07 and up)

  function decodeSystem(bytes) {
    const chunks = new Map();
    for (const m of splitSysex(bytes)) {
      if (!isGt3Header(m) || m[5] !== CMD_DT1) continue;
      const { address, data } = parseDt1(m);
      if (address[0] > SYSTEM_LAST_MSB) continue;
      chunks.set(address.join(' '), { address, data: Uint8Array.from(data) });
    }
    if (!chunks.size) throw new PatchError('no GT-3 system data');
    return [...chunks.values()];
  }

  const cloneSystem = system => system.map(c => ({ address: c.address.slice(), data: c.data.slice() }));

  const systemArea = name => SYSTEM_AREAS.find(a => a.name === name);

  function systemParam(area, offset) {
    const p = area.params.find(q => q.offset === offset);
    if (!p) throw new PatchError(`no system parameter ${area.name}/${offset}`);
    return p;
  }

  const sameArea = (c, area) => c.address[0] === area.address[0] && c.address[1] === area.address[1] && c.address[2] === area.address[2];
  const findChunk = (system, area, offset) => system.find(c => sameArea(c, area) && offset >= c.address[3] && offset < c.address[3] + c.data.length);

  // null when the dump doesn't hold that byte.
  function getSystemValue(system, area, offset) {
    const c = findChunk(system, area, offset);
    return c ? c.data[offset - c.address[3]] : null;
  }

  // Store a value and return the DT1 message that sets it on the GT-3 (which stores system settings at once).
  function setSystemValue(system, area, offset, value, dev = FILE_DEVICE_ID) {
    const p = systemParam(area, offset);
    checkValue(p, value);
    let c = findChunk(system, area, offset);
    if (!c) {
      c = { address: [...area.address, offset], data: new Uint8Array(1) };
      system.push(c);
    }
    c.data[offset - c.address[3]] = value;
    return dt1(dev, [...area.address, offset], [value]);
  }

  const encodeSystem = (system, dev = FILE_DEVICE_ID) => system.map(c => dt1(dev, c.address, c.data));

  // ------------------------------------------------------------ factory data

  function base64Bytes(b64) {
    const bin = atob(b64);
    return Uint8Array.from(bin, c => c.charCodeAt(0));
  }

  let defaultCache = null;
  function defaultPatch() {
    if (!defaultCache) {
      const messages = splitSysex(base64Bytes(DEFAULT_PATCH_SYX_BASE64));
      defaultCache = decodePatch(messages, { fallback: emptyPatch() }).patch;
    }
    return clonePatch(defaultCache);
  }

  return {
    PatchError, DEVICE_IDS, BLOCKS, PATCH_PACKETS, PATCH_SIZE, FILE_PATCH_SIZE, NAME_LENGTH,
    USER_PATCHES, ALL_PATCHES, PATCHES_PER_BANK, TARGETS,
    hex, hexBytes, checksum, dt1, rq1, splitSysex, parseDt1, concat,
    patchLabel, patchArea, patchFromArea, programChange, isUser,
    identityRequest, systemRequest, patchRequest, replyDevice,
    emptyPatch, clonePatch, defaultPatch, decodePatch, encodePatch, decodeFile, encodeFile,
    param, getValue, setValue, label, options, patchName, setPatchName, chain, isValidChain, setChain, samePatch, target,
    systemSyx: () => base64Bytes(SYSTEM_SYX_BASE64),
    SYSTEM_AREAS, decodeSystem, cloneSystem, systemArea, systemParam, getSystemValue, setSystemValue, encodeSystem,
  };
})();
