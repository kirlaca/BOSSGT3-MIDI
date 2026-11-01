// Checks webmidi/gt3.js without a GT-3: against the Qt FloorBoard's own formulas
// and constants, default.syx / system.syx, and the 325 patch files in
// packager/init patches (dumped from real GT-3s).
//
//     node tools/test_gt3.mjs

import fs from 'node:fs';
import path from 'node:path';
import vm from 'node:vm';
import { fileURLToPath } from 'node:url';

const BASE = path.dirname(path.dirname(fileURLToPath(import.meta.url)));
const ROOT = path.join(BASE, 'gt-3fxfloorboard_source_code_20250420');  // the Qt FloorBoard source
const WEB = path.join(BASE, 'webmidi');

const ctx = vm.createContext({ console, atob, performance });
for (const f of ['gt3-params.js', 'factory.js', 'gt3.js']) {
  vm.runInContext(fs.readFileSync(path.join(WEB, f), 'utf8'), ctx, { filename: f });
}
const GT3 = vm.runInContext('GT3', ctx);
const P = vm.runInContext('GT3_PARAMS', ctx);

let failures = 0;
let checks = 0;
function check(ok, what) {
  checks++;
  if (!ok) { failures++; console.log(`FAIL  ${what}`); }
}
const hexStr = bytes => Array.from(bytes, b => b.toString(16).toUpperCase().padStart(2, '0')).join('');
const same = (a, b) => a.length === b.length && a.every((x, i) => x === b[i]);

// ---------------------------------------------------------------- constants (globalVariables.h)

check(hexStr(GT3.identityRequest(0)) === 'F04100001B1100000000000000017FF7', 'identity request = idRequestString');
check(hexStr(GT3.systemRequest(0)) === 'F04100001B1100000000020000017DF7', 'system request = systemRequest');
check(GT3.PATCH_PACKETS === 23 && GT3.PATCH_SIZE === 650, 'patchPackets 23 / patchSize 650');

// ---------------------------------------------------------------- MidiTable::patchRequest, re-done the C++ way

function qtChecksum(hex) {
  // MidiTable: sum of bytes from checksumOffset (6) on, then 128 - sum % 128 (00 stays 00).
  let sum = 0;
  for (let i = 6; i < hex.length / 2; i++) sum += parseInt(hex.substr(i * 2, 2), 16);
  const s = sum % 128;
  return (s ? 128 - s : 0).toString(16).toUpperCase().padStart(2, '0');
}
function qtPatchRequest(bank, patch) {
  let patchOffset = ((bank - 1) * 4 + patch) - 1;
  const memorySize = 128;
  const emptyAddresses = memorySize - (35 * 4 - memorySize);
  if (bank > 35) patchOffset += emptyAddresses;
  const n = Math.floor(patchOffset / 128);
  const a1 = (7 + n).toString(16).toUpperCase().padStart(2, '0');
  const a2 = (patchOffset - 128 * n).toString(16).toUpperCase().padStart(2, '0');
  const msg = 'F04100001B11' + a1 + a2 + '0000' + '00001A00';
  return msg + qtChecksum(msg) + 'F7';
}
let prOk = true;
for (let n = 0; n < GT3.ALL_PATCHES; n++) {
  const bank = Math.floor(n / 4) + 1;
  const patch = (n % 4) + 1;
  if (hexStr(GT3.patchRequest(0, n)) !== qtPatchRequest(bank, patch)) { prOk = false; console.log('  patch', n, hexStr(GT3.patchRequest(0, n)), qtPatchRequest(bank, patch)); break; }
  const area = GT3.patchArea(n);
  if (GT3.patchFromArea(area[0], area[1]) !== n) { prOk = false; console.log('  area round trip', n); break; }
}
check(prOk, 'patchRequest / patchArea for all 340 patches match MidiTable::patchRequest');
check(hexStr(GT3.patchRequest(0)) === 'F04100001B110B000000' + '00001A00' + qtChecksum('F04100001B110B00000000001A00') + 'F7', 'temp patch request');
check(GT3.patchLabel(0) === 'U01-1' && GT3.patchLabel(139) === 'U35-4' && GT3.patchLabel(140) === 'P36-1' && GT3.patchLabel(339) === 'P85-4', 'patch labels');

// ---------------------------------------------------------------- SysxIO::getPatchChangeMsg

let pcOk = true;
for (let n = 0; n < GT3.ALL_PATCHES; n++) {
  const msb = Math.floor(n / 100).toString(16).padStart(2, '0');
  const pc = (n % 100).toString(16).padStart(2, '0');
  if (hexStr(GT3.programChange(n)) !== ('B000' + msb + 'B02000C0' + pc).toUpperCase()) { pcOk = false; break; }
}
check(pcOk, 'program change matches getPatchChangeMsg for all patches');

// ---------------------------------------------------------------- default.syx round trip

const defaultSyx = fs.readFileSync(path.join(ROOT, 'default.syx'));
const def = GT3.defaultPatch();
check(same(Array.from(GT3.encodeFile(def, { editor: true })), Array.from(defaultSyx)), 'default.syx -> patch -> file is byte-identical (878 bytes)');
check(GT3.patchName(def) === 'INIT PATCH' || GT3.patchName(def).length > 0, `default patch name: "${GT3.patchName(def)}"`);
console.log(`      default patch name "${GT3.patchName(def)}", chain ${GT3.chain(def).join(',')}`);

// ---------------------------------------------------------------- system.syx

const sysMsgs = GT3.splitSysex(Array.from(fs.readFileSync(path.join(ROOT, 'system.syx'))));
let sysOk = true;
for (const m of sysMsgs) { try { GT3.parseDt1(m); } catch (e) { sysOk = false; console.log('  ', e.message); } }
check(sysOk && sysMsgs.length > 0, `system.syx: ${sysMsgs.length} DT1 messages, all checksums ok`);
check(same(Array.from(GT3.systemSyx()), Array.from(fs.readFileSync(path.join(ROOT, 'system.syx')))), 'factory.js system.syx = system.syx');

// System model: every parameter midi.xml describes is in the dump and in range,
// edits land in the right byte, and the dump survives encode -> decode.
const sys = GT3.decodeSystem(GT3.systemSyx());
let sysParams = 0;
let sysInRange = 0;
for (const area of GT3.SYSTEM_AREAS) {
  for (const p of area.params) {
    sysParams++;
    const v = GT3.getSystemValue(sys, area, p.offset);
    if (v !== null && v >= p.min && v <= p.max) sysInRange++;
  }
}
check(sysInRange === sysParams, `system.syx holds all ${sysParams} system parameters, in range (${sysInRange})`);
const tuner = GT3.systemArea('Tuner');
const midiArea = GT3.systemArea('Midi');
console.log(`      tuner ${GT3.label(tuner.params[0], GT3.getSystemValue(sys, tuner, 0))}, `
  + `MIDI rx channel ${GT3.label(midiArea.params[0], GT3.getSystemValue(sys, midiArea, 0))}`);
const sysEdit = GT3.cloneSystem(sys);
const sm = GT3.setSystemValue(sysEdit, midiArea, 4, 7, 3);
check(hexStr(sm) === `F04103001B120103000407${hexStr([GT3.checksum([1, 3, 0, 4, 7])])}F7`
  && GT3.getSystemValue(sysEdit, midiArea, 4) === 7 && GT3.getSystemValue(sys, midiArea, 4) !== null,
'system edit: DT1 01 03 00 <offset> with the device ID, stored in the copy only');
const sysBack = GT3.decodeSystem(GT3.concat(GT3.encodeSystem(sysEdit)));
check(sysBack.length === sysEdit.length && sysBack.every((c, i) => same(c.address, sysEdit[i].address) && same(Array.from(c.data), Array.from(sysEdit[i].data))),
  `system encode -> decode keeps all ${sysEdit.length} chunks`);

// ---------------------------------------------------------------- the 325 real patches

const dir = path.join(ROOT, 'packager', 'init patches');
const files = fs.readdirSync(dir).filter(f => f.toLowerCase().endsWith('.syx'));
let rt = 0;
const outOfRange = new Map();   // "block/offset name" -> count
const unlabeled = new Map();
const all = [];
for (const f of files) {
  const bytes = Array.from(fs.readFileSync(path.join(dir, f)));
  let res;
  try {
    res = GT3.decodeFile(bytes);
  } catch (e) {
    check(false, `${f}: ${e.message}`);
    continue;
  }
  if (res.length !== 1 || res[0].missing.length) { check(false, `${f}: ${res.length} patches, missing ${res[0]?.missing}`); continue; }
  const { patch, source } = res[0];
  all.push(patch);
  const again = GT3.concat(GT3.encodePatch(patch, { to: source === null ? 'temp' : source, dev: 0 }));
  if (same(Array.from(again), bytes)) rt++;
  else check(false, `${f}: re-encoded bytes differ`);
  for (const b of P.blocks) {
    if (b.editorOnly) continue;
    for (const p of b.params) {
      const v = GT3.getValue(patch, b.id, p.offset);
      const key = `${GT3.hex(b.id)}/${GT3.hex(p.offset)} ${p.name}`;
      if (v < p.min || v > p.max) outOfRange.set(key, (outOfRange.get(key) || 0) + 1);
      else if (GT3.label(GT3.param(b.id, p.offset), v) === String(v) && !/^\d+$/.test(GT3.label(GT3.param(b.id, p.offset), v + 0))) {
        unlabeled.set(key, (unlabeled.get(key) || 0) + 1);
      }
    }
  }
}
check(rt === files.length, `${rt}/${files.length} patch files decode and re-encode byte-identically`);
if (outOfRange.size) {
  console.log(`      values outside the midi.xml range in the files (${outOfRange.size} parameters) - informative, not a failure:`);
  for (const [k, n] of [...outOfRange].sort((a, b) => b[1] - a[1]).slice(0, 15)) console.log(`        ${n.toString().padStart(4)} x ${k}`);
}

// Bulk: several patches in one file.
const bulk = GT3.encodeFile(all.slice(0, 3));
const back = GT3.decodeFile(Array.from(bulk));
check(back.length === 3 && back.every((r, i) => r.patch.blocks.every((d, b) => P.blocks[b].editorOnly || same(Array.from(d), Array.from(all[i].blocks[b])))), 'bulk file of 3 patches splits back into 3');

// ---------------------------------------------------------------- labels (MidiTable::rangeToValue)

const lbl = (b, o, v) => GT3.label(GT3.param(b, o), v);
check(lbl(0x0A, 0x01, 0) === '40' && lbl(0x0A, 0x01, 127) === '167' && lbl(0x0A, 0x01, 128) === '168' && lbl(0x0A, 0x01, 210) === '250', `Master BPM labels: ${lbl(0x0A, 1, 0)} ${lbl(0x0A, 1, 127)} ${lbl(0x0A, 1, 128)} ${lbl(0x0A, 1, 210)}`);
check(lbl(0x03, 0x02, 50) === '50 %', `OD drive 50 -> "${lbl(0x03, 0x02, 50)}"`);
check(lbl(0x01, 0x04, 0) === '-50 dB' && lbl(0x01, 0x04, 100) === '50 dB', `CS tone -> "${lbl(0x01, 4, 0)}" / "${lbl(0x01, 4, 100)}"`);
check(lbl(0x09, 0x02, 0) === '0.1s' && lbl(0x09, 0x02, 99) === '10.0s', `reverb time -> "${lbl(0x09, 2, 0)}" / "${lbl(0x09, 2, 99)}"`);
check(lbl(0x03, 0x01, 0) === 'Natural OD', `OD type 0 -> "${lbl(0x03, 1, 0)}"`);
check(lbl(0x07, 0x02, 0x5B) === '4.0*bpm', `delay time 5B -> "${lbl(0x07, 2, 0x5B)}"`);

// ---------------------------------------------------------------- live edit messages (MidiTable::dataChange)

const p = GT3.defaultPatch();
const m1 = GT3.setValue(p, 0x03, 0x02, 77);
const want1 = 'F04100001B120C0003024D';
check(hexStr(m1) === want1 + qtChecksum(want1) + 'F7', `OD drive edit: ${hexStr(m1)}`);
const m2 = GT3.setValue(p, 0x0A, 0x01, 200);      // two-byte: 200 = 01 48
const want2 = 'F04100001B120C000A010148';
check(hexStr(m2) === want2 + qtChecksum(want2) + 'F7' && GT3.getValue(p, 0x0A, 0x01) === 200, `Master BPM edit: ${hexStr(m2)}`);
let threw = false;
try { GT3.setValue(p, 0x03, 0x02, 101); } catch { threw = true; }
check(threw, 'out-of-range value is refused');
const m3 = GT3.setPatchName(p, 'Hello');
check(GT3.patchName(p) === 'Hello' && hexStr(m3).startsWith('F04100001B120C000C00'), 'patch name edit');

// ---------------------------------------------------------------- assign targets

let tOk = 0;
for (const t of GT3.TARGETS) if (GT3.target(t.code).param) tOk++;
check(tOk === GT3.TARGETS.length, `${tOk}/${GT3.TARGETS.length} assign targets point at a known parameter`);
const t0 = GT3.target(0x35);
console.log(`      target 0x35 = "${t0.name}" -> block ${GT3.hex(t0.block)} offset ${GT3.hex(t0.offset)} (${t0.param?.name})`);

console.log(`\n${checks - failures}/${checks} checks passed${failures ? ` - ${failures} FAILED` : ''}`);
process.exit(failures ? 1 : 0);
