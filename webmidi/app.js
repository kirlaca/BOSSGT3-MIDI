'use strict';

// GT-3 editor UI: the effects page. Patch data and SysEx come from gt3.js, MIDI
// from midi.js, the per-effect layout from effects.js. Layout and behaviour
// follow the ME-5 editor (D:\BOSSME5\webmidi\app.js).

const CONN_KEY = 'gt3.webmidi.conn';
const MIDI_CHANNELS = 16;
const BANKS_PER_PAGE = 8;
const USER_BANKS = GT3.USER_PATCHES / GT3.PATCHES_PER_BANK;                         // 35
const PRESET_BANKS = (GT3.ALL_PATCHES - GT3.USER_PATCHES) / GT3.PATCHES_PER_BANK;   // 50

const state = {
  library: new Array(GT3.ALL_PATCHES).fill(null), // patch per memory slot; null = not read yet
  source: 'Nothing read yet',
  current: 0,
  patch: null,      // the sound being edited (the GT-3's temporary patch)
  original: null,   // the patch as it was when selected - Revert returns here
  conn: null,       // {output, input, dev, channel} when talking to a real GT-3
  area: 'user',     // patch map: 'user' | 'preset'
  page: 0,          // patch map page within the area
  detail: null,     // chain code of the pedal shown in the detail panel
  view: 'effects',  // one of VIEWS
  controller: 0,    // assign page: index into CONTROLLERS (EXP, CTL, Assign 1-8)
  // User patches whose library copy is not on the GT-3 yet (file loads, copies, offline writes).
  unsynced: new Set(),
  lib: { area: 'user', filter: '', selected: new Set() },  // library page
  system: GT3.decodeSystem(GT3.systemSyx()),  // system settings, see GT3.decodeSystem
  systemSource: 'Defaults (system.syx) - not read from the GT-3',
  systemRead: false,  // read from the connected GT-3 since connecting
  // Connected, but the current slot has not been read (that needs BULK LOAD): the editor
  // shows the default patch as a placeholder. Edits still go out - they are heard - and the
  // Write dialog warns. Cleared when the slot is read, or a patch is loaded on purpose.
  needsRead: false,
  pedalPatch: null,   // the patch last picked on the GT-3 itself (its program change), null = not seen
};

const VIEWS = ['effects', 'assign', 'library', 'system'];

// ---------------------------------------------------------------- helpers

function h(tag, cls, text) {
  const el = document.createElement(tag);
  if (cls) el.className = cls;
  if (text !== undefined) el.textContent = text;
  return el;
}

const $ = sel => document.querySelector(sel);
const clamp = (n, lo, hi) => Math.min(hi, Math.max(lo, n));

let toastTimer;
function toast(message, kind = '') {
  const el = $('#toast');
  el.textContent = message;
  el.className = `toast show ${kind}`;
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => el.classList.remove('show'), kind === 'error' ? 6000 : 2800);
}

function arc(a0, a1, r = 42) {
  const pt = a => [50 + r * Math.sin((a * Math.PI) / 180), 50 - r * Math.cos((a * Math.PI) / 180)];
  const [x0, y0] = pt(a0);
  const [x1, y1] = pt(a1);
  return `M${x0.toFixed(2)} ${y0.toFixed(2)} A${r} ${r} 0 ${a1 - a0 > 180 ? 1 : 0} 1 ${x1.toFixed(2)} ${y1.toFixed(2)}`;
}

// Counts an action as a GoatCounter event (script in index.html, see README). Only the
// event name is sent. Silently does nothing when the script is blocked, offline or on localhost.
function track(event) {
  const send = () => { try { window.goatcounter?.count?.({ path: event, event: true }); } catch { /* not counted */ } };
  // The async counter script is in by the load event; earlier events wait for it.
  if (document.readyState === 'complete') send(); else window.addEventListener('load', send, { once: true });
}

const value = (block, offset) => GT3.getValue(state.patch, block, offset);
const optionLabel = (block, offset, v) => EFFECTS.valueText(block, offset, v);

// What "unchanged" means: the stored patch when it is known, else the patch as selected.
const basePatch = () => state.library[state.current] || state.original;
const isDirty = () => !GT3.samePatch(state.patch, basePatch());

// ---------------------------------------------------------------- MIDI out

// Live edits are coalesced per parameter and sent a moment later, so a fast
// knob drag sends the latest value instead of a backlog.
const pending = new Map();
let pendingTimer = null;

function queueSend(key, message) {
  if (!state.conn) return;
  pending.set(key, message);
  if (pendingTimer === null) pendingTimer = setTimeout(flushSends, 25);
}

function flushSends() {
  clearTimeout(pendingTimer);
  pendingTimer = null;
  if (!state.conn || !pending.size) { pending.clear(); return Promise.resolve(); }
  const messages = [...pending.values()];
  pending.clear();
  return Midi.send(state.conn.output, messages).catch(e => toast(`Send failed: ${e.message}`, 'error'));
}

// The whole edited patch into the GT-3's temporary patch (after Revert, Init, file load).
function sendTemp() {
  pending.clear();
  if (!state.conn) return Promise.resolve();
  return Midi.sendTemp(state.conn, state.patch).catch(e => toast(`Send failed: ${e.message}`, 'error'));
}

const dev = () => state.conn?.dev ?? 0;

// ---------------------------------------------------------------- edits

function edit(block, offset, v, { structural = false } = {}) {
  if (v === value(block, offset)) return;
  queueSend(`${block}/${offset}`, GT3.setValue(state.patch, block, offset, v, dev()));
  changed(structural);
}

// Something in state.patch changed: repaint. Structural changes (type, on/off, order) rebuild the pedals.
function changed(structural = true) {
  if (structural) renderView(); else repaint();
  refreshIndicators();
}

function setChain(order) {
  queueSend('chain', GT3.setChain(state.patch, order, dev()));
  changed();
}

function rename(name) {
  queueSend('name', GT3.setPatchName(state.patch, name, dev()));
  changed(false);
}

// ---------------------------------------------------------------- patch selection

let loadToken = 0;

// send: tell the GT-3 to go there (program change). Reading needs BULK LOAD, so it is
// left to Read (BULK); an unread patch shows the default patch as a placeholder.
async function selectPatch(n, { send = true } = {}) {
  n = (n + GT3.ALL_PATCHES) % GT3.ALL_PATCHES;
  await flushSends();
  const token = ++loadToken;
  state.current = n;
  followPage();
  state.needsRead = Boolean(state.conn && !state.library[n]);
  showPatch(state.library[n] || GT3.defaultPatch());
  if (!state.conn) return;
  if (state.needsRead) toast(`${GT3.patchLabel(n)} is not read yet: the editor shows the default patch, your edits are still heard. Read (BULK) shows what it holds.`);
  if (!send) return;
  try {
    await Midi.selectPatch(state.conn, n, state.conn.channel);
  } catch (e) {
    if (token === loadToken) toast(`${GT3.patchLabel(n)}: ${e.message}`, 'error');
  }
}

// The current slot's stored patch is known now (read, loaded, copied): show it, unless
// the user is editing - but the needsRead placeholder is never worth keeping.
function showStored() {
  if (!state.library[state.current] || (!state.needsRead && isDirty())) return;
  state.needsRead = false;
  showPatch(state.library[state.current]);
}

function showPatch(patch) {
  state.patch = GT3.clonePatch(patch);
  state.original = GT3.clonePatch(patch);
  renderAll();
}

// Keep the patch map on the page that holds the current patch.
function followPage() {
  const bank = Math.floor(state.current / GT3.PATCHES_PER_BANK);
  state.area = GT3.isUser(state.current) ? 'user' : 'preset';
  state.page = Math.floor((state.area === 'user' ? bank : bank - USER_BANKS) / BANKS_PER_PAGE);
}

// ---------------------------------------------------------------- write

// The GT-3 takes data for its memory only in BULK LOAD mode and stores it on EXIT, after
// checking it ("Checking...", about 5 s) - measured 2026-11-01. Guides the user through
// both; resolves false if they cancel before anything is sent.
async function inBulkLoad({ title, text, action, work }) {
  const ready = await Ask.confirm({
    icon: 'question', title,
    text: [...[].concat(text), 'Put the GT-3 in BULK LOAD first: UTILITY 4 times, then PARAMETER ▶ 10 times - the display shows "waiting".'],
    confirmText: action,
  });
  if (!ready) return false;
  await work();
  await Ask.alert({
    icon: 'warning', title: 'Press EXIT on the GT-3',
    text: ['It checks the data ("Checking…", about 5 seconds) and stores it.', 'When it is back on the patch display, press Done.'],
    confirmText: 'Done',
  });
  return true;
}

// Store the edited patch into user patch n (and on the GT-3 when connected).
async function writeTo(n) {
  const patch = GT3.clonePatch(state.patch);
  const label = GT3.patchLabel(n);
  try {
    await flushSends();
    if (state.conn) {
      const done = await inBulkLoad({
        title: `Write to ${label}`,
        text: `The sound in the editor replaces ${label}${state.library[n] ? ` "${GT3.patchName(state.library[n])}"` : ''} on the GT-3.`,
        action: 'Write',
        work: () => Midi.writePatch(state.conn, patch, n),
      });
      if (!done) return;
      // After storing, the GT-3 still plays its old temporary patch - load the written one.
      await Midi.selectPatch(state.conn, n, state.conn.channel);
    }
    state.library[n] = patch;
    state.needsRead = false; // the editor's sound is now what that slot holds
    if (state.conn) state.unsynced.delete(n); else state.unsynced.add(n);
    if (n !== state.current) {
      // Now editing the copy just stored; the GT-3 went to it too.
      state.current = n;
      state.original = GT3.clonePatch(patch);
      followPage();
    }
    renderPatchMap();
    renderDisplay();
    refreshIndicators();
    if (state.view === 'library') renderView();
    toast(`Written to ${state.conn ? label : `${label} (editor only - offline)`}`);
    track(state.conn ? 'write-patch' : 'write-patch-offline');
  } catch (e) {
    toast(`Write failed: ${e.message}`, 'error');
  }
}

function openWriteDialog() {
  const select = $('#write-target');
  const target = GT3.isUser(state.current) ? state.current : 0;
  select.replaceChildren(...Array.from({ length: GT3.USER_PATCHES }, (_, n) => {
    const p = state.library[n];
    const o = h('option', '', `${GT3.patchLabel(n)}  ${p ? GT3.patchName(p) : '-'}`);
    o.value = n;
    o.selected = n === target;
    return o;
  }));
  $('#write-msg').textContent = !state.conn
    ? 'Offline: the patch is only stored in the editor (save it to a file to keep it).'
    : state.needsRead
      ? 'This patch has not been read from the GT-3: it starts from the default patch, not from what the GT-3 holds.'
      // Measured: the GT-3's own WRITE stores its temporary patch, edits from the editor included.
      : 'Write here goes through BULK LOAD. Quicker for the patch the GT-3 is on: press WRITE on the GT-3 itself - '
        + 'it stores what you hear, the editor changes included.';
  $('#write-dialog').showModal();
  select.focus();
}

function revert() {
  state.patch = GT3.clonePatch(state.original);
  sendTemp();
  changed();
}

function loadInit() {
  state.needsRead = false;
  state.patch = GT3.defaultPatch();
  sendTemp();
  changed();
  toast('Init patch loaded - Write stores it');
}

// ---------------------------------------------------------------- reading the GT-3

function openReadDialog() {
  if (!state.conn) return;
  const n = state.current;
  const known = state.library[n];
  $('#read-this').textContent = `This patch - ${GT3.patchLabel(n)}${known ? ` "${GT3.patchName(known)}"` : ''}`;
  $('#read-msg').textContent = state.pedalPatch === null || state.pedalPatch === n
    ? 'The patch the GT-3 is on, as you hear it now, and as it is stored.'
    : `The GT-3 was last seen on ${GT3.patchLabel(state.pedalPatch)} - pick ${GT3.patchLabel(n)} on it before BULK LOAD to read what it plays.`;
  document.querySelector('input[name="read-what"][value="patch"]').checked = true;
  $('#read-system').checked = false;
  $('#read-dialog').showModal();
  $('#read-ok').focus();
}

async function runRead() {
  const what = document.querySelector('input[name="read-what"]:checked').value;
  const system = $('#read-system').checked;
  $('#read-dialog').close();
  if (!state.conn) return;
  const ok = what === 'all' ? await readUserPatches() : await readCurrent();
  if (ok && system) await readSystem();
  await Ask.alert({
    icon: ok ? 'success' : 'warning', title: 'Press EXIT on the GT-3',
    text: 'Back on the patch display, every edit is heard at once again.',
  });
}

// The current patch: what the GT-3 plays now (its temporary patch - becomes the edited sound)
// and what is stored in its slot (the library copy, Revert goes back to it).
async function readCurrent() {
  const n = state.current;
  const box = Ask.progress({ title: 'Reading the GT-3', text: `${GT3.patchLabel(n)}: what it plays now, and what is stored.` });
  try {
    const temp = (await Midi.readPatch(state.conn, null)).patch;
    box.update(0.5);
    const stored = (await Midi.readPatch(state.conn, n)).patch;
    box.update(1, 'Done');
    state.library[n] = stored;
    state.unsynced.delete(n);
    if (state.current === n) {
      state.needsRead = false;
      state.patch = temp;
      state.original = GT3.clonePatch(stored);
      renderAll();
    }
    toast(`Read ${GT3.patchLabel(n)} "${GT3.patchName(stored)}"`);
    track('read-patch');
    return true;
  } catch (e) {
    Ask.alert({
      icon: 'error', title: 'Read failed',
      text: [e.message, 'Is the GT-3 in BULK LOAD ("waiting")? Is the Device ID in the MIDI connection the one set on the GT-3?'],
    });
    return false;
  } finally {
    box.close();
  }
}

// The USB-MIDI interface tested with the GT-3 (DOREMiDi) stalled after about a minute of
// back-to-back patch dumps; with a rest after each patch all 140 came through.
const READ_REST_MS = 500;

// The 140 user patches (the GT-3 in BULK LOAD); true when all came.
async function readUserPatches() {
  if (!state.conn) return false;
  const box = Ask.progress({ title: 'Reading the GT-3', text: 'The 140 user patches, one after the other.' });
  let n = 0;
  try {
    for (; n < GT3.USER_PATCHES; n++) {
      box.update(n / GT3.USER_PATCHES, `${GT3.patchLabel(n)}  ${n + 1} / ${GT3.USER_PATCHES}`);
      state.library[n] = (await Midi.readPatch(state.conn, n)).patch;
      state.unsynced.delete(n);
      await new Promise(r => setTimeout(r, READ_REST_MS));
    }
    box.update(1, 'Done');
    state.source = 'Read from GT-3';
    toast(`Read ${GT3.USER_PATCHES} user patches`);
    track('read-gt3');
    return true;
  } catch (e) {
    Ask.alert({
      icon: 'error', title: 'Read failed',
      text: [`At ${GT3.patchLabel(n)}: ${e.message}`, `${n} patches were read before it.`,
        'Is the GT-3 still in BULK LOAD ("waiting")? If it is, unplug and replug the MIDI interface and read again.'],
    });
    track('read-gt3-failed');
    return false;
  } finally {
    box.close();
    showStored();
    renderAll();
  }
}

// ---------------------------------------------------------------- files

function loadFile(file) {
  const reader = new FileReader();
  reader.onload = () => {
    try {
      const bytes = new Uint8Array(reader.result);
      if (isSystemFile(bytes)) { loadSystemFile(bytes, file.name); return; }
      const results = GT3.decodeFile(bytes);
      if (results.length === 1) {
        // One patch: into the editor, like picking a sound; Write stores it.
        state.needsRead = false;
        state.patch = results[0].patch;
        sendTemp();
        changed();
        toast(`Loaded ${file.name} into the editor - Write stores it`);
        track('load-file');
        return;
      }
      // A bulk dump: into the user patches, at their own numbers when the file has them.
      let next = 0;
      let count = 0;
      for (const r of results) {
        const n = r.source ?? next;
        next = n + 1;
        if (!GT3.isUser(n)) continue;
        state.library[n] = r.patch;
        state.unsynced.add(n);
        count++;
      }
      state.source = `File · ${file.name}`;
      if (state.library[state.current]) state.needsRead = false;
      showPatch(state.library[state.current] || GT3.defaultPatch());
      toast(`Loaded ${count} patches from ${file.name} (editor only - Write stores them one by one)`);
      track('load-file');
    } catch (e) {
      toast(`Load failed: ${e.message}`, 'error');
    }
  };
  reader.onerror = () => toast(`Load failed: ${reader.error?.message}`, 'error');
  reader.readAsArrayBuffer(file);
}

// The browser downloads `bytes` as `filename` (no server, no folder to write into).
function download(bytes, filename) {
  const blob = new Blob([bytes], { type: 'application/octet-stream' });
  const a = h('a');
  a.href = URL.createObjectURL(blob);
  a.download = filename;
  a.click();
  setTimeout(() => URL.revokeObjectURL(a.href), 1000);
}

// A system dump has data in the system area (address 00-02) and none in the patch areas.
function isSystemFile(bytes) {
  const messages = GT3.splitSysex(bytes).filter(m => m[1] === 0x41 && m[5] === 0x12);
  return messages.length > 0 && messages.every(m => m[6] <= 0x02);
}

const saveWhat = () => document.querySelector('input[name="save-what"]:checked').value;

function renderFileDialog() {
  const unknown = state.library.slice(0, GT3.USER_PATCHES).filter(p => !p).length;
  $('#file-msg').textContent = saveWhat() === 'bank'
    ? `${GT3.USER_PATCHES} patches as one .syx dump${unknown ? ` - ${unknown} not read yet are saved as the init patch` : ''}.`
    : 'The sound in the editor, with its name, as a single-patch .syx file.';
  const name = $('#file-name');
  const base = saveWhat() === 'bank' ? 'gt3-user-patches' : (GT3.patchName(state.patch).trim() || 'patch').replace(/[^\w.-]+/g, '_');
  if (!name.dataset.touched) name.value = `${base}.syx`;
}

function openSaveDialog() {
  $('#file-name').dataset.touched = '';
  renderFileDialog();
  $('#file-dialog').showModal();
  $('#file-name').focus();
  $('#file-name').select();
}

function saveFile() {
  let filename = $('#file-name').value.trim();
  if (!filename) return;
  if (!/\.syx$/i.test(filename)) filename += '.syx';
  try {
    const patches = saveWhat() === 'bank'
      ? state.library.slice(0, GT3.USER_PATCHES).map(p => p || GT3.defaultPatch())
      : [state.patch];
    download(GT3.encodeFile(patches, { editor: true }), filename);
    $('#file-dialog').close();
    toast(`Saved ${filename}`);
    track('save-file');
  } catch (e) {
    toast(`Save failed: ${e.message}`, 'error');
  }
}

// ---------------------------------------------------------------- controls

// Every control registers a paint function; repaint() runs them after an edit.
let painters = [];
const repaint = () => painters.forEach(p => p());

// What a control shows and changes: {p, get(), set(v, options), text(v)}. The
// controls take (block, offset) for a patch parameter, or a binding object in
// place of the block (and no offset) for anything else - the system settings.
function patchBinding(block, offset) {
  return {
    p: GT3.param(block, offset),
    get: () => value(block, offset),
    set: (v, options) => edit(block, offset, v, options),
    text: v => optionLabel(block, offset, v),
  };
}
const bind = (block, offset) => (typeof block === 'object' ? block : patchBinding(block, offset));

// `range` / `text` override the parameter's own range and value texts - the
// assign Min / Max knobs take them from the target parameter.
function knob(block, offset, label, { size = 'normal', range: limits = null, text: textOf = null } = {}) {
  const b = bind(block, offset);
  const p = limits ? { min: limits[0], max: limits[1] } : b.p;
  const wrap = h('div', `knob-wrap${size === 'normal' ? '' : ` ${size}`}`);
  const dial = h('div', 'knob');
  const out = h('span', 'knob-value');
  dial.tabIndex = 0;
  dial.setAttribute('role', 'slider');
  dial.setAttribute('aria-label', label);
  dial.setAttribute('aria-valuemin', p.min);
  dial.setAttribute('aria-valuemax', p.max);
  dial.innerHTML = `<svg viewBox="0 0 100 100" aria-hidden="true">
    <path class="knob-track" d="${arc(-135, 135)}"/><path class="knob-fill"/>
    <circle class="knob-cap" cx="50" cy="50" r="31"/><circle class="knob-cap-top" cx="50" cy="50" r="25"/>
    <line class="knob-pointer" x1="50" y1="42" x2="50" y2="23"/></svg>`;
  wrap.append(h('span', 'knob-label', label), dial, out);
  const fill = dial.querySelector('.knob-fill');
  const pointer = dial.querySelector('.knob-pointer');
  const range = Math.max(1, p.max - p.min);

  function paint() {
    const v = b.get();
    const f = clamp((v - p.min) / range, 0, 1);
    const angle = -135 + 270 * f;
    fill.setAttribute('d', f > 0 ? arc(-135, angle) : '');
    pointer.setAttribute('transform', `rotate(${angle} 50 50)`);
    const text = textOf ? textOf(v) : b.text(v);
    out.textContent = text;
    dial.setAttribute('aria-valuenow', v);
    dial.setAttribute('aria-valuetext', text);
  }

  // Values outside the range (old garbage) show as they are and snap in on the first move.
  const set = n => b.set(clamp(Math.round(n), p.min, p.max));

  dial.addEventListener('pointerdown', e => {
    e.preventDefault();
    dial.focus();
    dial.setPointerCapture(e.pointerId);
    dial.classList.add('dragging');
    const startY = e.clientY;
    const startValue = clamp(b.get(), p.min, p.max);
    // Full sweep in roughly 180px whatever the range, but never finer than 2px per step.
    const pxPerStep = clamp(180 / range, 2, 22);
    const move = ev => set(startValue + (startY - ev.clientY) / pxPerStep);
    const up = () => {
      dial.classList.remove('dragging');
      dial.removeEventListener('pointermove', move);
      dial.removeEventListener('pointerup', up);
      dial.removeEventListener('pointercancel', up);
    };
    dial.addEventListener('pointermove', move);
    dial.addEventListener('pointerup', up);
    dial.addEventListener('pointercancel', up);
  });
  dial.addEventListener('wheel', e => {
    e.preventDefault();
    set(b.get() + (e.deltaY < 0 ? 1 : -1));
  }, { passive: false });
  dial.addEventListener('keydown', e => {
    const v = b.get();
    const big = Math.max(1, Math.ceil(range / 10));
    const next = {
      ArrowUp: v + 1, ArrowRight: v + 1, ArrowDown: v - 1, ArrowLeft: v - 1,
      PageUp: v + big, PageDown: v - big, Home: p.min, End: p.max,
    }[e.key];
    if (next === undefined) return;
    e.preventDefault();
    e.stopPropagation();
    set(next);
  });

  painters.push(paint);
  paint();
  return wrap;
}

function fader(block, offset, label) {
  const b = bind(block, offset);
  const wrap = h('label', 'fader-wrap');
  const out = h('span', 'knob-value');
  const input = h('input');
  input.type = 'range';
  input.min = b.p.min;
  input.max = b.p.max;
  input.setAttribute('aria-label', label);
  const paint = () => {
    input.value = b.get();
    out.textContent = b.text(b.get());
    input.setAttribute('aria-valuetext', out.textContent);
  };
  input.addEventListener('input', () => b.set(Number(input.value)));
  input.addEventListener('keydown', e => e.stopPropagation());
  input.addEventListener('wheel', e => {
    e.preventDefault();
    b.set(clamp(b.get() + (e.deltaY < 0 ? 1 : -1), b.p.min, b.p.max));
  }, { passive: false });
  painters.push(paint);
  paint();
  wrap.append(h('span', 'knob-label', label), input, out);
  return wrap;
}

// Horizontal slider with its value in the caption (assign ranges, pedal time / rate).
function slider(block, offset, label) {
  const b = bind(block, offset);
  const wrap = h('label', 'slider');
  const caption = h('span', 'slider-caption');
  const out = h('b');
  caption.append(`${label} · `, out);
  const input = h('input');
  input.type = 'range';
  input.min = b.p.min;
  input.max = b.p.max;
  const paint = () => {
    input.value = b.get();
    out.textContent = b.text(b.get());
    input.setAttribute('aria-valuetext', out.textContent);
  };
  input.addEventListener('input', () => b.set(Number(input.value)));
  input.addEventListener('keydown', e => e.stopPropagation());
  painters.push(paint);
  paint();
  wrap.append(caption, input);
  return wrap;
}

// Round on/off switch for a 0/1 parameter.
function onSwitch(block, offset, label) {
  const b = bind(block, offset);
  const btn = h('button', 'switch');
  btn.type = 'button';
  btn.setAttribute('aria-label', `${label} on/off`);
  btn.append(h('span'));
  btn.addEventListener('click', () => b.set(b.get() ? 0 : 1, { structural: true }));
  const paint = () => {
    const on = b.get() === 1;
    btn.classList.toggle('on', on);
    btn.setAttribute('aria-pressed', String(on));
  };
  painters.push(paint);
  paint();
  return btn;
}

// Segmented switch over the named values of a parameter.
function segmented(block, offset, { structural = false } = {}) {
  const b = bind(block, offset);
  const seg = h('div', 'seg');
  const buttons = GT3.options(b.p).map(([v, text]) => {
    const btn = h('button', '', EFFECTS.shortValue(text));
    btn.type = 'button';
    btn.addEventListener('click', () => b.set(v, { structural }));
    seg.append(btn);
    return [v, btn];
  });
  const paint = () => {
    const now = b.get();
    for (const [v, btn] of buttons) btn.setAttribute('aria-pressed', String(v === now));
  };
  painters.push(paint);
  paint();
  return seg;
}

function listBox(block, offset, label, { structural = false, cls = '' } = {}) {
  const b = bind(block, offset);
  const select = h('select', cls);
  select.setAttribute('aria-label', label);
  for (const [v, text] of GT3.options(b.p)) {
    const o = h('option', '', text);
    o.value = v;
    select.append(o);
  }
  select.addEventListener('change', () => b.set(Number(select.value), { structural }));
  select.addEventListener('keydown', e => e.stopPropagation());
  const paint = () => { select.value = b.get(); };
  painters.push(paint);
  paint();
  return select;
}

// Any parameter in the detail panel, as the control that fits it.
function control(pedal, offset) {
  const p = GT3.param(pedal.block, offset);
  const label = EFFECTS.caption(pedal, offset);
  const kind = EFFECTS.widgetFor(p);
  if (kind === 'knob') return knob(pedal.block, offset, label);
  const wrap = h('div', 'ctl');
  wrap.append(h('span', 'knob-label', label), kind === 'seg' ? segmented(pedal.block, offset) : listBox(pedal.block, offset, label));
  return wrap;
}

// The type picker: ◀ name ▶ on a little LCD window, the name being a list too.
function typeWindow(pedal) {
  const p = GT3.param(pedal.block, pedal.type);
  const win = h('div', 'typewin');
  const step = d => {
    const v = value(pedal.block, pedal.type);
    edit(pedal.block, pedal.type, v + d > p.max ? p.min : v + d < p.min ? p.max : v + d, { structural: true });
  };
  const prev = h('button', '', '◀');
  prev.type = 'button';
  prev.setAttribute('aria-label', `Previous ${pedal.abbr} type`);
  prev.addEventListener('click', () => step(-1));
  const next = h('button', '', '▶');
  next.type = 'button';
  next.setAttribute('aria-label', `Next ${pedal.abbr} type`);
  next.addEventListener('click', () => step(1));
  win.append(prev, listBox(pedal.block, pedal.type, `${pedal.abbr} type`, { structural: true }), next);
  return win;
}

// ---------------------------------------------------------------- pedals

// Pressing on any of these works the control; anywhere else on a pedal picks it up.
const PEDAL_CONTROLS = 'button, input, select, label, .knob';

function renderPedal(pedal, order, i) {
  const position = i + 1;
  const on = EFFECTS.isOn(pedal, state.patch);
  const { value: typeValue, def } = EFFECTS.typeOf(pedal, state.patch);
  const el = h('article', `pedal${on ? '' : ' off'}${pedal.wide ? ' wide' : ''}${state.detail === pedal.code ? ' selected' : ''}`);
  el.style.setProperty('--body', pedal.body);
  el.style.setProperty('--ink', pedal.ink);
  el.dataset.code = pedal.code;

  const top = h('div', 'pedal-top');
  const check = h('span', 'check', `${position} · CHECK`);
  top.append(check, h('span', `led${on ? ' on' : ''}`));
  el.append(top);

  if (pedal.type !== undefined) el.append(pedal.typeUi === 'lcd' ? typeWindow(pedal) : segmented(pedal.block, pedal.type, { structural: true }));
  if (pedal.seg !== undefined) el.append(segmented(pedal.block, pedal.seg));

  if (!def) {
    el.append(h('p', 'pedal-note', `Unknown type (${typeValue})`));
  } else if (pedal.faders) {
    const faders = h('div', 'faders');
    for (const o of pedal.faders) faders.append(fader(pedal.block, o, EFFECTS.caption(pedal, o)));
    el.append(faders);
  } else {
    const knobs = h('div', 'knobs');
    for (const o of EFFECTS.mainParams(pedal, def)) knobs.append(knob(pedal.block, o, EFFECTS.caption(pedal, o)));
    el.append(knobs);
  }

  el.append(h('div', 'spacer'));
  const plate = h('button', 'plate');
  plate.type = 'button';
  plate.title = 'Show every parameter';
  plate.setAttribute('aria-expanded', String(state.detail === pedal.code));
  plate.append(h('span', 'plate-model', pedal.abbr), h('span', 'plate-name', pedal.title), h('span', 'plate-more', '⋯'));
  plate.addEventListener('click', () => openDetail(state.detail === pedal.code ? null : pedal.code));
  el.append(plate);

  if (pedal.on === null) {
    el.append(h('div', 'treadle', 'EXP'));
  } else {
    const foot = h('button', 'footswitch');
    foot.type = 'button';
    foot.setAttribute('aria-label', `${pedal.abbr} on/off`);
    foot.setAttribute('aria-pressed', String(on));
    foot.addEventListener('click', () => edit(pedal.block, pedal.on, on ? 0 : 1, { structural: true }));
    el.append(foot);
  }
  if (on) el.append(h('span', 'on-bar'));

  // Drag the pedal by its body to move it in the chain, like the chips above.
  el.addEventListener('pointerdown', e => { el.draggable = !e.target.closest(PEDAL_CONTROLS); });
  el.addEventListener('dragstart', e => {
    e.dataTransfer.setData('text/plain', String(i));
    e.dataTransfer.effectAllowed = 'move';
    el.classList.add('dragging');
  });
  el.addEventListener('dragend', () => { el.classList.remove('dragging'); el.draggable = false; });
  el.addEventListener('dragover', e => { e.preventDefault(); el.classList.add('drop'); });
  el.addEventListener('dragleave', e => { if (!el.contains(e.relatedTarget)) el.classList.remove('drop'); });
  el.addEventListener('drop', e => {
    e.preventDefault();
    el.classList.remove('drop');
    const from = Number(e.dataTransfer.getData('text/plain'));
    if (Number.isInteger(from) && from !== i) moveInChain(order, from, i);
  });
  return el;
}

function renderMaster() {
  const el = h('div', 'module');
  el.append(h('span', 'module-title', 'Master'));
  for (const [block, offset, label] of EFFECTS.MASTER_KNOBS) el.append(knob(block, offset, label, { size: 'large' }));
  return el;
}

function renderChips(order) {
  const list = $('#chain-chips');
  list.replaceChildren();
  order.forEach((code, i) => {
    const pedal = EFFECTS.BY_CODE.get(code);
    const chip = h('li', `chip${EFFECTS.isOn(pedal, state.patch) ? '' : ' off'}`);
    chip.draggable = true;
    chip.tabIndex = 0;
    chip.title = `${pedal.title} - drag to move, click for its parameters`;
    const swatch = h('span', 'chip-swatch');
    swatch.style.background = pedal.body;
    chip.append(h('span', 'chip-n', String(i + 1)), swatch, h('span', '', pedal.abbr));
    chip.addEventListener('click', () => openDetail(code, { scroll: true }));
    chip.addEventListener('keydown', e => {
      if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); openDetail(code, { scroll: true }); return; }
      if (!e.altKey || (e.key !== 'ArrowLeft' && e.key !== 'ArrowRight')) return;
      e.preventDefault();
      e.stopPropagation();
      const to = i + (e.key === 'ArrowLeft' ? -1 : 1);
      if (to < 0 || to >= order.length) return;
      moveInChain(order, i, to);
      list.children[to]?.focus();
    });
    chip.addEventListener('dragstart', e => {
      e.dataTransfer.setData('text/plain', String(i));
      e.dataTransfer.effectAllowed = 'move';
      chip.classList.add('dragging');
    });
    chip.addEventListener('dragend', () => chip.classList.remove('dragging'));
    chip.addEventListener('dragover', e => { e.preventDefault(); chip.classList.add('drop'); });
    chip.addEventListener('dragleave', () => chip.classList.remove('drop'));
    chip.addEventListener('drop', e => {
      e.preventDefault();
      chip.classList.remove('drop');
      const from = Number(e.dataTransfer.getData('text/plain'));
      if (Number.isInteger(from) && from !== i) moveInChain(order, from, i);
    });
    list.append(chip);
  });
}

function moveInChain(order, from, to) {
  const next = order.slice();
  const [code] = next.splice(from, 1);
  next.splice(to, 0, code);
  setChain(next);
}

function renderBoard() {
  const order = EFFECTS.order(state.patch);
  const board = $('#chain');
  board.replaceChildren();
  order.forEach((code, i) => board.append(renderPedal(EFFECTS.BY_CODE.get(code), order, i)));
  board.append(renderMaster());
  renderChips(order);
  renderDetail();
}

// ---------------------------------------------------------------- detail panel

function openDetail(code, { scroll = false } = {}) {
  state.detail = code;
  renderView();
  if (scroll && code !== null) {
    document.querySelector(`.pedal[data-code="${code}"]`)?.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
  }
}

function renderDetail() {
  const panel = $('#detail');
  const pedal = state.detail === null ? null : EFFECTS.BY_CODE.get(state.detail);
  panel.hidden = !pedal;
  document.body.classList.toggle('detail-open', Boolean(pedal));
  panel.replaceChildren();
  if (!pedal) return;
  panel.style.setProperty('--body', pedal.body);

  const head = h('div', 'detail-head');
  const badge = h('span', 'detail-badge', pedal.abbr);
  badge.style.background = pedal.body;
  badge.style.color = pedal.ink;
  head.append(badge, h('h2', '', pedal.title));
  if (pedal.on !== null) {
    const on = EFFECTS.isOn(pedal, state.patch);
    const sw = h('button', `btn small onoff${on ? ' on' : ''}`, on ? 'ON' : 'OFF');
    sw.type = 'button';
    sw.setAttribute('aria-pressed', String(on));
    sw.addEventListener('click', () => edit(pedal.block, pedal.on, on ? 0 : 1, { structural: true }));
    head.append(sw);
  }
  if (pedal.type !== undefined) head.append(listBox(pedal.block, pedal.type, `${pedal.abbr} type`, { structural: true, cls: 'detail-type' }));
  const close = h('button', 'btn small detail-close', '✕');
  close.type = 'button';
  close.setAttribute('aria-label', 'Close the detail panel');
  close.addEventListener('click', () => openDetail(null));
  head.append(close);
  panel.append(head);

  const { value: typeValue, def } = EFFECTS.typeOf(pedal, state.patch);
  const note = pedal.notes?.[typeValue];
  if (note) panel.append(h('p', 'detail-note', note));
  const body = h('div', 'detail-body');
  for (const [title, offsets] of def ? def.groups : []) {
    const group = h('section', 'group');
    if (title) group.append(h('h3', '', title));
    const row = h('div', 'group-controls');
    for (const o of offsets) row.append(control(pedal, o));
    group.append(row);
    body.append(group);
  }
  panel.append(body);
}

// ---------------------------------------------------------------- display, map, status

function lcdInfo() {
  const type = (block, offset) => GT3.label(GT3.param(block, offset), value(block, offset));
  return `PRE:${type(0x04, 1)}  OD:${type(0x03, 1)}`;
}

function renderDisplay() {
  $('#patch-label').textContent = GT3.patchLabel(state.current);
  $('#source').textContent = state.source;
}

function refreshIndicators() {
  const dirty = isDirty();
  $('#edited').classList.toggle('on', dirty);
  $('#store').classList.toggle('primary', dirty);
  $('#revert').disabled = !dirty && GT3.samePatch(state.patch, state.original);
  $('#patch-name').textContent = GT3.patchName(state.patch) || ' ';
  $('#lcd-info').textContent = lcdInfo();
}

function renderPatchMap() {
  const user = state.area === 'user';
  const banks = user ? USER_BANKS : PRESET_BANKS;
  const pages = Math.ceil(banks / BANKS_PER_PAGE);
  state.page = clamp(state.page, 0, pages - 1);
  const first = state.page * BANKS_PER_PAGE;
  const last = Math.min(banks, first + BANKS_PER_PAGE) - 1;
  const bankNo = b => (user ? b : b + USER_BANKS) + 1;
  const prefix = user ? 'U' : 'P';
  $('#pm-range').textContent = `${prefix}${String(bankNo(first)).padStart(2, '0')} – ${prefix}${String(bankNo(last)).padStart(2, '0')}`;
  $('#pm-prev').disabled = state.page === 0;
  $('#pm-next').disabled = state.page === pages - 1;

  const area = $('#pm-area');
  area.replaceChildren(...[['user', 'User'], ['preset', 'Preset']].map(([key, text]) => {
    const b = h('button', '', text);
    b.type = 'button';
    b.setAttribute('aria-pressed', String(state.area === key));
    b.addEventListener('click', () => { state.area = key; state.page = 0; renderPatchMap(); });
    return b;
  }));

  const map = $('#patch-map');
  map.replaceChildren();
  for (let b = first; b <= last; b++) {
    const row = h('div', 'pm-bank');
    row.append(h('span', 'pm-bank-no', `${prefix}${String(bankNo(b)).padStart(2, '0')}`));
    for (let i = 0; i < GT3.PATCHES_PER_BANK; i++) {
      const n = (user ? b : b + USER_BANKS) * GT3.PATCHES_PER_BANK + i;
      const patch = state.library[n];
      const cell = h('button', `pm-cell${n === state.current ? ' current' : ''}${patch ? '' : ' unknown'}${state.unsynced.has(n) ? ' unsynced' : ''}`);
      cell.type = 'button';
      const name = patch ? GT3.patchName(patch) : '';
      cell.title = `${GT3.patchLabel(n)} ${name || '(not read yet)'}${state.unsynced.has(n) ? ' - only in the editor, not on the GT-3' : ''}`;
      cell.append(h('span', 'pm-num', String(i + 1)), h('span', 'pm-name', name));
      cell.addEventListener('click', () => selectPatch(n));
      row.append(cell);
    }
    map.append(row);
  }
}

function renderStatus() {
  const btn = $('#conn-status');
  btn.classList.toggle('online', Boolean(state.conn));
  renderConnText();
  $('#bulk-read').disabled = !state.conn;
  if (state.view === 'library' || state.view === 'system') renderView(); // their buttons depend on the connection
}

// Only the visible page is built; its controls register the painters.
function renderView() {
  painters = [];
  for (const view of VIEWS) $(`#view-${view}`).hidden = view !== state.view;
  document.body.classList.toggle('detail-open', state.view === 'effects' && state.detail !== null);
  for (const tab of document.querySelectorAll('.tab[data-view]')) {
    const on = tab.dataset.view === state.view;
    tab.classList.toggle('on', on);
    if (on) tab.setAttribute('aria-current', 'page'); else tab.removeAttribute('aria-current');
  }
  ({ effects: renderBoard, assign: renderAssign, library: renderLibrary, system: renderSystem })[state.view]();
}

function setView(view) {
  state.view = VIEWS.includes(view) ? view : 'effects';
  renderView();
}

function renderConnText() {
  // Read (BULK) stands out while the patch on screen is only the placeholder.
  $('#bulk-read').classList.toggle('wanted', Boolean(state.conn && state.needsRead));
  $('#conn-text').textContent = state.conn
    ? `${state.conn.output} · Dev ${state.conn.dev + 1}${state.needsRead ? ' · not read' : ''}`
    : 'Offline';
}

function renderAll() {
  renderConnText();
  renderDisplay();
  renderPatchMap();
  renderView();
  refreshIndicators();
}

// ---------------------------------------------------------------- patch name

function startRename() {
  const btn = $('#patch-name');
  const input = $('#name-input');
  input.value = GT3.patchName(state.patch);
  btn.hidden = true;
  input.hidden = false;
  input.focus();
  input.select();
}

function endRename(commit) {
  const btn = $('#patch-name');
  const input = $('#name-input');
  if (input.hidden) return;
  input.hidden = true;
  btn.hidden = false;
  if (commit && input.value !== GT3.patchName(state.patch)) rename(input.value);
  btn.focus();
}

// ---------------------------------------------------------------- connection

let stopFollowing = () => {};

// The editor follows patches picked on the GT-3 (it sends a program change for each).
function followPedal(ports) {
  stopFollowing();
  stopFollowing = () => {};
  if (!ports?.input) return;
  stopFollowing = Midi.watchPatchChanges(n => {
    state.pedalPatch = n;
    if (n !== state.current) selectPatch(n, { send: false });
  }, ports.input);
}

// `patch`: the patch the GT-3 is on, if seen - the editor goes there without sending a
// program change (it never moves the GT-3 away from the patch picked on it).
function connect(ports, { quiet = false, patch = null } = {}) {
  state.conn = ports;
  followPedal(ports);
  state.systemRead = false;
  try {
    if (ports) localStorage.setItem(CONN_KEY, JSON.stringify(ports)); else localStorage.removeItem(CONN_KEY);
  } catch { /* storage blocked */ }
  state.needsRead = false;
  renderStatus();
  $('#conn-dialog').close();
  if (ports) {
    if (patch === null) toast('Connected - pick the patch on the GT-3, the editor follows it');
    return selectPatch(patch ?? state.current, { send: false });
  }
  if (!quiet) toast('Offline - edits stay in the editor.');
  return Promise.resolve();
}

// Last session's ports - they preselect the connection dialog; the editor never connects by itself.
function savedPorts() {
  try { return JSON.parse(localStorage.getItem(CONN_KEY)) || null; } catch { return null; }
}

let watching = false;
function watchPorts() {
  if (watching) return;
  watching = true;
  Midi.onPortsChanged(() => {
    if (state.conn && !Midi.outputs().some(p => p.name === state.conn.output)) {
      state.conn = null; // keep the saved ports, so plugging back in + reload reconnects
      state.needsRead = false;
      renderStatus();
      toast('MIDI interface unplugged - offline', 'error');
    }
    if ($('#conn-dialog').open) fillPortLists();
  });
}

function fillPortLists() {
  const outputs = Midi.outputs();
  const inputs = Midi.inputs();
  const saved = state.conn || savedPorts();
  for (const [id, ports, current] of [['#port-out', outputs, saved?.output], ['#port-in', inputs, saved?.input]]) {
    const select = $(id);
    // Last time's port, else the first real one (Windows lists its software synth as an output).
    const keep = [select.value, current].find(name => ports.some(p => p.name === name))
      ?? ports.find(p => !/GS Wavetable|Microsoft/i.test(p.name))?.name;
    select.replaceChildren(...ports.map(p => {
      const o = h('option', '', p.name);
      o.selected = p.name === keep;
      return o;
    }));
  }
  $('#conn-msg').textContent = outputs.length ? '' : 'No MIDI ports found - is the interface plugged in?';
}

function numberOptions(select, count, selected) {
  select.replaceChildren(...Array.from({ length: count }, (_, i) => {
    const o = h('option', '', String(i + 1));
    o.value = i;
    o.selected = i === selected;
    return o;
  }));
}

async function openConnDialog() {
  const msg = $('#conn-msg');
  msg.textContent = 'Asking the browser for MIDI access…';
  const saved = state.conn || savedPorts();
  numberOptions($('#port-dev'), GT3.DEVICE_IDS, saved?.dev ?? 0);
  numberOptions($('#port-channel'), MIDI_CHANNELS, saved?.channel ?? 0);
  $('#port-out').replaceChildren();
  $('#port-in').replaceChildren();
  $('#conn-dialog').showModal();
  try {
    await Midi.init();
    watchPorts();
    fillPortLists();
    // Hear the patch picked on the GT-3 before BULK LOAD, so connecting can stay on it.
    const stop = Midi.watchPatchChanges(n => {
      state.pedalPatch = n;
      msg.textContent = `The GT-3 is on ${GT3.patchLabel(n)} - now put it in BULK LOAD and press Auto-detect.`;
    });
    $('#conn-dialog').addEventListener('close', stop, { once: true });
  } catch (e) {
    msg.textContent = e.message;
    if (navigator.requestMIDIAccess) track('midi-denied'); // no Web MIDI at all is counted on load
  }
}

// ---------------------------------------------------------------- wiring

function wire() {
  $('#prev').addEventListener('click', () => selectPatch(state.current - 1));
  $('#next').addEventListener('click', () => selectPatch(state.current + 1));
  $('#store').addEventListener('click', openWriteDialog);
  $('#write-ok').addEventListener('click', () => {
    $('#write-dialog').close();
    writeTo(Number($('#write-target').value));
  });
  $('#revert').addEventListener('click', revert);
  $('#lib-init').addEventListener('click', loadInit);
  $('#lib-load').addEventListener('click', () => $('#file-input').click());
  $('#file-input').addEventListener('change', e => {
    const [file] = e.target.files;
    e.target.value = ''; // picking the same file again still fires change
    if (file) loadFile(file);
  });
  $('#lib-save').addEventListener('click', openSaveDialog);
  for (const radio of document.querySelectorAll('input[name="save-what"]')) radio.addEventListener('change', renderFileDialog);
  $('#file-name').addEventListener('input', e => { e.target.dataset.touched = '1'; });
  $('#file-ok').addEventListener('click', saveFile);
  $('#file-name').addEventListener('keydown', e => {
    if (e.key === 'Enter') { e.preventDefault(); saveFile(); }
  });
  $('#bulk-read').addEventListener('click', openReadDialog);
  $('#read-ok').addEventListener('click', runRead);
  $('#pm-prev').addEventListener('click', () => { state.page--; renderPatchMap(); });
  $('#pm-next').addEventListener('click', () => { state.page++; renderPatchMap(); });

  $('#patch-name').addEventListener('click', startRename);
  const nameInput = $('#name-input');
  nameInput.addEventListener('keydown', e => {
    e.stopPropagation();
    if (e.key === 'Enter') { e.preventDefault(); endRename(true); }
    if (e.key === 'Escape') { e.preventDefault(); endRename(false); }
  });
  nameInput.addEventListener('blur', () => endRename(true));

  $('#conn-status').addEventListener('click', openConnDialog);
  $('#conn-use').addEventListener('click', () => {
    const output = $('#port-out').value;
    const input = $('#port-in').value;
    if (!output) return;
    connect({ output, input, dev: Number($('#port-dev').value), channel: Number($('#port-channel').value) }, { patch: state.pedalPatch });
    track('connect-manual');
  });
  $('#conn-disconnect').addEventListener('click', () => { stopFollowing(); connect(null); });

  // Drop a .syx file anywhere on the page to load it.
  document.addEventListener('dragover', e => { if (e.dataTransfer.types.includes('Files')) e.preventDefault(); });
  document.addEventListener('drop', e => {
    const file = e.dataTransfer.files[0];
    if (!file) return;
    e.preventDefault();
    loadFile(file);
  });

  document.addEventListener('keydown', e => {
    if (document.querySelector('dialog[open]') || e.target.closest('input, select, textarea, [role="slider"], .chip')) return;
    if (e.key === 'ArrowLeft') selectPatch(state.current - 1);
    if (e.key === 'ArrowRight') selectPatch(state.current + 1);
    if (e.key === 'Escape' && state.view === 'effects' && state.detail !== null) openDetail(null);
  });

  wireLibrary();
  wireSystem();

  // The page is in the address (#assign), so reload and the back button keep it.
  window.addEventListener('hashchange', () => setView(location.hash.slice(1)));
}

function init() {
  wire();
  followPage();
  state.view = VIEWS.includes(location.hash.slice(1)) ? location.hash.slice(1) : 'effects';
  showPatch(GT3.defaultPatch());
  renderStatus();
  // Visitors on Safari / iOS can never connect - worth knowing how many arrive.
  if (!navigator.requestMIDIAccess) track('no-webmidi');
}

init();
