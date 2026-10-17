'use strict';

// System page: the GT-3's own settings - tuner, output, global EQ, controls,
// patch change, MIDI, manual mode (menuPage_system.cpp). They are not patch
// data: the GT-3 stores each change at once, so there is no Write here. The
// groups and their order follow the Qt page.
// Uses the helpers of app.js (h, $, state, knob, segmented, listBox, ...).

const SYSTEM_GROUPS = [
  ['Output', 'Global', [[0, 'Output select']]],
  ['Global EQ / NS / Reverb', 'Global', [[1, 'Low EQ'], [2, 'High EQ'], [3, 'NS Thresh'], [4, 'Reverb']]],
  ['Tuner', 'Tuner', [[0, 'Pitch'], [1, 'Output']]],
  ['Controls', 'System', [[0, 'LCD'], [1, 'Dial function'], [6, 'Assign hold'], [2, 'Sub CTL 1'], [3, 'Sub CTL 2']]],
  ['Patch change', 'System', [[4, 'Mode'], [5, 'Bank extent']]],
  ['MIDI', 'Midi', [[0, 'Rx channel'], [1, 'Omni'], [2, 'Tx channel'], [3, 'Program change out'], [8, 'Program map'],
    [4, 'EXP pedal out'], [5, 'CTL pedal out'], [6, 'Sub CTL 1 out'], [7, 'Sub CTL 2 out']]],
  ['Manual mode', 'Manual', [[0, 'Pedal 1'], [1, 'Pedal 2'], [2, 'Pedal 3'], [3, 'Pedal 4'], [5, 'Bank down'], [4, 'Bank up']]],
];

// A system parameter for the controls of app.js (see patchBinding there).
function systemBinding(area, offset) {
  const p = GT3.systemParam(area, offset);
  return {
    p,
    get: () => GT3.getSystemValue(state.system, area, offset) ?? p.min,
    set: v => {
      if (v === GT3.getSystemValue(state.system, area, offset)) return;
      const message = GT3.setSystemValue(state.system, area, offset, v, dev());
      queueSend(`sys/${area.name}/${offset}`, message);
      repaint();
    },
    text: v => EFFECTS.shortValue(GT3.label(p, v)).replace(/ Midi Controller$/, ''),
  };
}

function systemControl(area, offset, label) {
  const b = systemBinding(area, offset);
  const kind = EFFECTS.widgetFor(b.p);
  if (kind === 'knob') return knob(b, null, label);
  const wrap = h('div', 'ctl');
  wrap.append(h('span', 'knob-label', label), kind === 'seg' ? segmented(b, null) : listBox(b, null, label));
  return wrap;
}

function renderSystem() {
  const groups = $('#sys-groups');
  groups.replaceChildren(...SYSTEM_GROUPS.map(([title, areaName, params]) => {
    const area = GT3.systemArea(areaName);
    const group = h('section', 'group');
    group.append(h('h3', '', title));
    const row = h('div', 'group-controls');
    for (const [offset, label] of params) row.append(systemControl(area, offset, label));
    group.append(row);
    return group;
  }));
  $('#sys-source').textContent = state.conn ? state.systemSource : `${state.systemSource} · offline: changes stay in the editor`;
  $('#sys-read').disabled = !state.conn;
  $('#sys-send').disabled = !state.conn;
}

// ---------------------------------------------------------------- reading / files

async function readSystem() {
  const btn = $('#sys-read');
  btn.disabled = true;
  btn.textContent = 'Reading…';
  try {
    const messages = await Midi.readSystem(state.conn);
    state.system = GT3.decodeSystem(GT3.concat(messages));
    state.systemSource = 'Read from GT-3';
    state.systemRead = true;
    toast('System settings read from the GT-3');
  } catch (e) {
    toast(`System read failed: ${e.message}`, 'error');
  } finally {
    btn.textContent = 'Read from GT-3';
    if (state.view === 'system') renderView();
  }
}

// Opening the page while connected shows the GT-3's own settings, not the defaults.
function readSystemOnce() {
  if (state.conn && !state.systemRead) readSystem();
}

async function sendSystem() {
  if (!await Ask.confirm({
    title: 'Send all system settings?',
    text: 'Every system setting on the GT-3 is replaced by what this page shows. The GT-3 stores them at once.',
    confirmText: 'Send',
  })) return;
  try {
    await flushSends();
    await Midi.send(state.conn.output, GT3.encodeSystem(state.system, state.conn.dev));
    state.systemRead = true;
    toast('System settings sent to the GT-3');
  } catch (e) {
    toast(`Send failed: ${e.message}`, 'error');
  }
}

function loadSystemFile(bytes, name) {
  try {
    state.system = GT3.decodeSystem(bytes);
    state.systemSource = `File · ${name}`;
    location.hash = '#system';
    renderView();
    toast(state.conn ? `Loaded ${name} - "Send all to GT-3" puts it on the GT-3` : `Loaded ${name}`);
  } catch (e) {
    toast(`Load failed: ${e.message}`, 'error');
  }
}

function wireSystem() {
  $('#sys-read').addEventListener('click', readSystem);
  $('#sys-send').addEventListener('click', sendSystem);
  $('#sys-load').addEventListener('click', () => $('#sys-file').click());
  $('#sys-file').addEventListener('change', e => {
    const [file] = e.target.files;
    e.target.value = '';
    if (!file) return;
    file.arrayBuffer().then(buf => loadSystemFile(new Uint8Array(buf), file.name), err => toast(`Load failed: ${err.message}`, 'error'));
  });
  $('#sys-save').addEventListener('click', () => {
    download(GT3.concat(GT3.encodeSystem(state.system)), 'gt3-system.syx');
    toast('Saved gt3-system.syx');
  });
}
