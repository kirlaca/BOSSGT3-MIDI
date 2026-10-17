'use strict';

// EXP / CTL / Assign page. Every controller is patch data, a block of its own:
//     0D EXP pedal   on, target, min, max
//     0E CTL pedal   on, target, min, max, source mode
//     0F-16 Assign 1-8: also source, active range, internal pedal, wave pedal
// The target is a two-byte code into the target table (gt3-params.js); Min and
// Max are shown in the target parameter's own range and units, and reset to its
// full range when the target changes (customControlTarget.cpp).
// Uses the controls and helpers of app.js (h, knob, slider, edit, value, ...).

const CONTROLLERS = [
  { name: 'EXP Pedal', block: 0x0D, fixedSource: 'EXP pedal' },
  { name: 'CTL Pedal', block: 0x0E, fixedSource: 'CTL pedal', mode: 7 },
  ...Array.from({ length: 8 }, (_, i) => ({ name: `Assign ${i + 1}`, block: 0x0F + i, assign: true, mode: 8 })),
];

const CTL = { on: 0, target: 1, min: 3, max: 5, source: 7, lo: 9, hi: 10, trigger: 11, time: 12, curve: 13, rate: 14, wave: 15 };
const SOURCE_INTERNAL = 5;
const SOURCE_WAVE = 6;

// "CC#01 Midi Controller" -> "CC#01"; the pedal names in title case.
function sourceName(text) {
  const cc = /^CC#\s*(\d+)/.exec(text);
  if (cc) return `CC#${cc[1].padStart(2, '0')}`;
  return text.replace(/\bPDL\b/, 'PEDAL').toLowerCase().replace(/(^|\s)(\w)/g, (_, s, c) => s + c.toUpperCase())
    .replace(/\b(Exp|Ctl)\b/g, m => m.toUpperCase()).replace(/-([lmh])$/i, (_, c) => `-${c.toUpperCase()}`);
}

// The controller's target: {code, block, offset, name, param} or null for a code the table doesn't know.
const targetOf = c => GT3.target(value(c.block, CTL.target));

function sourceText(c) {
  if (c.fixedSource) return c.fixedSource;
  return sourceName(GT3.label(GT3.param(c.block, CTL.source), value(c.block, CTL.source)));
}

// Min / Max knob limits and texts: the target parameter's, else the raw stored numbers.
function targetScale(c) {
  const t = targetOf(c);
  if (!t) {
    const own = GT3.param(c.block, CTL.min);
    return { range: [own.min, own.max], text: v => String(v), name: `Unknown target (${value(c.block, CTL.target)})` };
  }
  return { range: [t.param.min, t.param.max], text: v => EFFECTS.valueText(t.block, t.offset, v), name: t.name, target: t };
}

// A new target: Min / Max go to the ends of its range, so it starts out doing something sensible.
function setTarget(c, code) {
  const t = GT3.target(code);
  edit(c.block, CTL.target, code);
  edit(c.block, CTL.min, t.param.min);
  edit(c.block, CTL.max, t.param.max, { structural: true });
}

// ---------------------------------------------------------------- list

function renderControllerList() {
  const list = $('#ctl-list');
  list.replaceChildren(...CONTROLLERS.map((c, i) => {
    const on = value(c.block, CTL.on) === 1;
    const t = targetOf(c);
    const b = h('button', `ctl-item${i === state.controller ? ' sel' : ''}`);
    b.type = 'button';
    b.setAttribute('aria-pressed', String(i === state.controller));
    const name = h('span', 'ctl-name');
    name.append(h('span', `dot${on ? ' on' : ''}`), c.name);
    const what = on ? `${t ? t.name : 'Unknown target'} · ${sourceText(c)}` : 'Off';
    b.append(name, h('span', `ctl-target${on ? ' live' : ''}`, what));
    b.addEventListener('click', () => { state.controller = i; renderView(); });
    return b;
  }));
}

// ---------------------------------------------------------------- cards

function card(title, note = '', dim = false) {
  const el = h('section', `card${dim ? ' dim' : ''}`);
  const head = h('h2', 'card-h', title);
  if (note) head.append(h('span', 'card-note', note));
  el.append(head);
  return el;
}

function field(label, control) {
  const el = h('div', 'field');
  el.append(h('span', '', label), control);
  return el;
}

// The target list, grouped by the effect block it belongs to.
function targetSelect(c) {
  const select = h('select');
  select.setAttribute('aria-label', `${c.name} target`);
  const code = value(c.block, CTL.target);
  if (!GT3.target(code)) {
    const o = h('option', '', `Unknown target (${code})`);
    o.value = code;
    select.append(o);
  }
  let group = null;
  let groupBlock = -1;
  for (const t of GT3.TARGETS) {
    if (t.block !== groupBlock) {
      groupBlock = t.block;
      group = h('optgroup');
      group.label = GT3.BLOCKS[t.block].name;
      select.append(group);
    }
    const o = h('option', '', t.name);
    o.value = t.code;
    group.append(o);
  }
  select.value = code;
  select.addEventListener('change', () => setTarget(c, Number(select.value)));
  select.addEventListener('keydown', e => e.stopPropagation());
  return select;
}

function targetCard(c) {
  const scale = targetScale(c);
  const el = card('Target', `${GT3.TARGETS.length} targets, grouped by block`);
  el.append(field('What this controller moves', targetSelect(c)));
  const knobs = h('div', 'minmax');
  const arrow = h('span', 'minmax-arrow', '→');
  arrow.setAttribute('aria-hidden', 'true');
  knobs.append(
    knob(c.block, CTL.min, 'Min', { size: 'xl', range: scale.range, text: scale.text }),
    arrow,
    knob(c.block, CTL.max, 'Max', { size: 'xl', range: scale.range, text: scale.text }),
  );
  el.append(knobs);
  const note = h('p', 'card-foot');
  note.append("Min and Max use the target's own range and units: ",
    h('b', '', `${scale.text(scale.range[0])} … ${scale.text(scale.range[1])}`));
  el.append(note);
  return el;
}

function sourceCard(c) {
  const el = card('Source');
  if (c.assign) el.append(field('What moves it', listBox(c.block, CTL.source, `${c.name} source`, { structural: true })));
  else el.append(h('p', 'card-foot', `Always the ${c.fixedSource}.`));
  if (c.mode !== undefined) el.append(field('Mode', segmented(c.block, c.mode)));
  if (c.assign) {
    const range = h('div', 'two');
    range.append(slider(c.block, CTL.lo, 'Active range low'), slider(c.block, CTL.hi, 'Active range high'));
    el.append(range);
  }
  return el;
}

// Source position 0-127 -> target value, as the pedal applies it.
function responseCard(c) {
  const el = card('Response', 'source → target');
  const svg = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
  svg.setAttribute('viewBox', '0 0 340 180');
  svg.setAttribute('role', 'img');
  svg.setAttribute('aria-label', 'How the source position maps onto the target value');
  svg.classList.add('response');
  svg.innerHTML = `<rect class="r-frame" x="30" y="10" width="300" height="140"/>
    <rect class="r-band" y="10" height="140"/>
    <path class="r-curve"/>
    <text class="r-min" x="24" text-anchor="end"/><text class="r-max" x="24" text-anchor="end"/>
    <text x="30" y="168">0</text><text x="330" y="168" text-anchor="end">127</text>
    <text class="r-src" x="180" y="168" text-anchor="middle"/>`;
  el.append(svg);
  const X = s => 30 + (s / 127) * 300;
  const scale = targetScale(c);
  const span = Math.max(1, scale.range[1] - scale.range[0]);
  const Y = v => 150 - clamp((v - scale.range[0]) / span, 0, 1) * 140;
  const q = sel => svg.querySelector(sel);
  painters.push(() => {
    const min = value(c.block, CTL.min);
    const max = value(c.block, CTL.max);
    const lo = c.assign ? value(c.block, CTL.lo) : 0;
    const hi = c.assign ? value(c.block, CTL.hi) : 127;
    const toggle = c.mode !== undefined && value(c.block, c.mode) === 1;
    const [a, b] = lo <= hi ? [lo, hi] : [hi, lo];
    q('.r-curve').setAttribute('d', toggle
      ? `M30 ${Y(min)} H${X((a + b) / 2)} V${Y(max)} H330`
      : `M30 ${Y(min)} H${X(a)} L${X(b)} ${Y(max)} H330`);
    q('.r-band').setAttribute('x', X(a));
    q('.r-band').setAttribute('width', Math.max(0, X(b) - X(a)));
    q('.r-min').setAttribute('y', Y(min) + 4);
    q('.r-min').textContent = scale.text(min);
    q('.r-max').setAttribute('y', Y(max) + 4);
    q('.r-max').textContent = scale.text(max);
    q('.r-src').textContent = `${sourceText(c)}${toggle ? ' (toggle)' : ''}`;
  });
  return el;
}

function internalCard(c) {
  const used = value(c.block, CTL.source) === SOURCE_INTERNAL;
  const el = card('Internal pedal', 'used when Source is Internal Pedal', !used);
  el.append(field('Trigger', listBox(c.block, CTL.trigger, 'Internal pedal trigger')));
  const row = h('div', 'two');
  row.append(slider(c.block, CTL.time, 'Time'), field('Curve', segmented(c.block, CTL.curve)));
  el.append(row);
  return el;
}

function waveCard(c) {
  const used = value(c.block, CTL.source) === SOURCE_WAVE;
  const el = card('Wave pedal', 'used when Source is Wave Pedal', !used);
  el.append(field('Waveform', segmented(c.block, CTL.wave)), slider(c.block, CTL.rate, 'Rate'));
  return el;
}

// ---------------------------------------------------------------- page

function renderAssign() {
  renderControllerList();
  const c = CONTROLLERS[state.controller];
  const main = $('#ctl-detail');
  const head = h('div', 'ctl-head');
  const title = h('div', 'ctl-title');
  const on = value(c.block, CTL.on) === 1;
  title.append(h('h1', '', c.name), onSwitch(c.block, CTL.on, c.name), h('span', 'muted', on ? 'On' : 'Off'));
  head.append(title, h('span', 'muted', `Patch data · block ${GT3.hex(c.block)}`));

  const cards = h('div', 'cards');
  cards.append(targetCard(c), sourceCard(c), responseCard(c));
  if (c.assign) cards.append(internalCard(c), waveCard(c));
  main.replaceChildren(head, cards);
  repaint();
}
