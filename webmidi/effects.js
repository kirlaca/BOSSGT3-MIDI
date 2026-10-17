'use strict';

// What the GT-3 editor shows for each effect block: the pedal skin, which
// parameter switches it on, picks its type, and which parameters belong to
// each type. Parameter numbers are byte offsets in the block (gt3-params.js);
// the groups follow the edit pages of the Qt FloorBoard (stompbox_*.cpp).
//
// `code` is the effect's code in the FX Chain block (0B), so the pedals can be
// laid out in the patch's own order.

const EFFECTS = (() => {
  const MASTER = 0x0A;

  // Pedal colours: approximations of the matching BOSS compacts - tweak freely.
  const PEDALS = [
    {
      code: 0, block: 0x00, abbr: 'SFX', title: 'Special FX', body: '#6d57a6', ink: '#ffffff',
      on: 0, type: 1, typeUi: 'lcd',
      types: {
        0: { groups: [['Acoustic Sim', [2, 3, 4]]] },
        1: { groups: [['Slow Gear', [5, 6]]] },
        2: { groups: [['Anti-Feedbacker', [7, 8, 9]]] },
        3: { groups: [['FeedBacker', [10, 11, 13, 12, 14, 17, 18]], ['Vibrato', [15, 16]]], main: [11, 13, 12, 14] },
        4: { groups: [['Picking Sim', [19, 20, 21]]] },
        5: { groups: [['Tremolo / Pan', [22, 23, 24, 25]]], main: [24, 25, 23] },
      },
      labels: {
        2: 'Top', 3: 'Body', 4: 'Level', 5: 'Sens', 6: 'Rise', 7: 'Freq', 8: 'Depth', 9: 'Search',
        10: 'Mode', 11: 'Rise', 12: 'Rise ↑', 13: 'Level', 14: 'Level ↑', 15: 'Vib Rate', 16: 'Vib Depth',
        17: 'Depth', 18: 'Tone', 19: 'Type', 20: 'Tone', 21: 'Level', 22: 'Mode', 23: 'Wave', 24: 'Rate', 25: 'Depth',
      },
    },
    {
      code: 1, block: 0x01, abbr: 'CS', title: 'Compressor', body: '#2f6cc8', ink: '#ffffff',
      on: 0, type: 1, typeUi: 'seg',
      types: {
        0: { groups: [['Compressor', [2, 3, 4, 5]]] },
        1: { groups: [['Limiter', [6, 7, 8, 9]]] },
      },
      labels: { 2: 'Sustain', 3: 'Attack', 4: 'Tone', 5: 'Level', 6: 'Thresh', 7: 'Release', 8: 'Tone', 9: 'Level' },
    },
    {
      code: 2, block: 0x02, abbr: 'WAH', title: 'Wah', body: '#2a2b30', ink: '#e8e8ea',
      on: 0, type: 1, typeUi: 'seg',
      types: {
        0: { groups: [['Pedal Wah', [2, 3]]] },
        1: { groups: [['Auto Wah', [4, 5, 6, 7, 8, 9, 10, 11]]], main: [6, 7, 8, 11] },
      },
      labels: { 2: 'Pedal', 3: 'Level', 4: 'Mode', 5: 'Polarity', 6: 'Sens', 7: 'Freq', 8: 'Peak', 9: 'Rate', 10: 'Depth', 11: 'Level' },
    },
    {
      code: 3, block: 0x03, abbr: 'OD/DS', title: 'Overdrive / Distortion', body: '#f1c02c', ink: '#151515',
      on: 0, type: 1, typeUi: 'lcd',
      types: Object.fromEntries(Array.from({ length: 12 }, (_, t) => [t, { groups: [['', [2, 3, 4, 5]]] }])),
      labels: { 2: 'Drive', 3: 'Bass', 4: 'Treble', 5: 'Level' },
      // The OD/DS is an analog circuit; "External" swaps in the EXT OD/DS send/return loop.
      notes: {
        11: 'External: the rear EXT OD/DS SEND / RETURN loop takes the place of the built-in distortion. '
          + 'Not yet checked on the unit what Drive / Bass / Treble do in this mode.',
      },
    },
    {
      code: 4, block: 0x04, abbr: 'PRE', title: 'PreAmp + Speaker', body: '#c39b45', ink: '#151515', wide: true,
      on: 0, type: 1, typeUi: 'lcd',
      types: Object.fromEntries(Array.from({ length: 14 }, (_, t) => [t, {
        groups: [['Pre-amp', [2, 3, 4, 5, 6, 7, 8, 9]], ['Speaker cabinet', [10, 11, 12]]], main: [2, 3, 4, 5, 6, 7],
      }])),
      labels: {
        2: 'Volume', 3: 'Bass', 4: 'Middle', 5: 'Treble', 6: 'Pres.', 7: 'Master', 8: 'Bright', 9: 'Gain',
        10: 'Mic Pos', 11: 'Mic Level', 12: 'Direct',
      },
    },
    {
      code: 5, block: 0x05, abbr: 'EQ', title: 'Equalizer', body: '#d5d8dd', ink: '#151515',
      on: 0, faders: [1, 4, 7, 8, 9],
      types: { null: { groups: [['Low', [1]], ['Low-mid', [2, 3, 4]], ['High-mid', [5, 6, 7]], ['High', [8]], ['Output', [9]]] } },
      labels: { 1: 'Low', 2: 'Freq', 3: 'Q', 4: 'Lo-Mid', 5: 'Freq', 6: 'Q', 7: 'Hi-Mid', 8: 'High', 9: 'Level' },
    },
    {
      code: 6, block: 0x06, abbr: 'MOD', title: 'Modulation', body: '#2e9a8c', ink: '#ffffff',
      on: 0, type: 1, typeUi: 'lcd',
      types: {
        0: {
          groups: [['Harmony 1', [2, 3, 4, 5, 6, 7, 9, 10]], ['Harmony 2', [11, 12, 13, 14, 15, 17]], ['Output', [18, 19]]],
          main: [4, 10, 17, 19],
        },
        1: { groups: [['Flanger', [20, 21, 22, 23, 24, 25]]] },
        2: { groups: [['Phaser', [26, 27, 28, 29, 30, 31, 32, 33]]], main: [27, 28, 29, 30] },
        3: { groups: [['Auto Riff', [34, 35, 36, 37, 38, 39, 40]], ['Output', [41, 42]]], main: [36, 37, 39, 41] },
        4: {
          groups: [['Low', [43]], ['Low-mid', [44, 45, 46]], ['High-mid', [47, 48, 49]], ['High', [50]], ['Output', [51]]],
          main: [43, 46, 49, 50],
        },
        5: { groups: [['Crossover', [52]], ['Low band', [53, 54, 55, 56]], ['High band', [57, 58, 59, 60]]], main: [53, 54, 57, 58] },
        6: { groups: [['Short Delay', [61, 63]], ['Output', [64]]] },
        7: { groups: [['Humanizer', [65, 66, 67, 68, 69, 70, 71, 72]]], main: [69, 70, 71, 72] },
        8: { groups: [['Vibrato', [73, 74, 75, 76]]], main: [73, 74, 76] },
        9: {
          groups: [['Input', [77, 78]], ['Wave', [79, 80, 81, 82]], ['Filter', [83, 84, 85, 86, 87]],
            ['Envelope', [88, 89, 90, 91]], ['Output', [92, 93]]],
          main: [83, 84, 92, 93],
        },
        10: { groups: [['Ring Modulator', [94, 95]], ['Output', [96, 97]]] },
        11: { groups: [['Slicer', [98, 99, 100]]], main: [99, 100] },
      },
      labels: {
        2: 'Voices', 3: 'Mode', 4: 'Pitch', 5: 'Fine', 6: 'Harmony', 7: 'Pre Dly', 9: 'Fdbk', 10: 'Level',
        11: 'Mode', 12: 'Pitch', 13: 'Fine', 14: 'Harmony', 15: 'Pre Dly', 17: 'Level', 18: 'Key', 19: 'Direct',
        20: 'Rate', 21: 'Depth', 22: 'Manual', 23: 'Reso', 24: 'Separ.', 25: 'Level',
        26: 'Stages', 27: 'Rate', 28: 'Depth', 29: 'Manual', 30: 'Reso', 31: 'Step', 32: 'Step Rate', 33: 'Level',
        34: 'Phrase', 35: 'Loop', 36: 'Tempo', 37: 'Sens', 38: 'Key', 39: 'Attack', 40: 'Hold', 41: 'Level', 42: 'Direct',
        43: 'Low', 44: 'Freq', 45: 'Q', 46: 'Lo-Mid', 47: 'Freq', 48: 'Q', 49: 'Hi-Mid', 50: 'High', 51: 'Level',
        52: 'X-over', 53: 'Rate', 54: 'Depth', 55: 'Pre Dly', 56: 'Level', 57: 'Rate', 58: 'Depth', 59: 'Pre Dly', 60: 'Level',
        61: 'Time', 63: 'Fdbk', 64: 'Level',
        65: 'Mode', 66: 'Vowel 1', 67: 'Vowel 2', 68: 'Sens', 69: 'Rate', 70: 'Depth', 71: 'Manual', 72: 'Level',
        73: 'Rate', 74: 'Depth', 75: 'Trigger', 76: 'Rise',
        77: 'Sens', 78: 'Wave', 79: 'Chromatic', 80: 'Octave', 81: 'PWM Rate', 82: 'PWM Depth',
        83: 'Cutoff', 84: 'Reso', 85: 'Flt Sens', 86: 'Decay', 87: 'Flt Depth',
        88: 'Attack', 89: 'Release', 90: 'Velocity', 91: 'Hold', 92: 'Synth', 93: 'Direct',
        94: 'Mode', 95: 'Freq', 96: 'Effect', 97: 'Direct', 98: 'Pattern', 99: 'Rate', 100: 'Sens',
      },
    },
    {
      code: 7, block: 0x07, abbr: 'DD', title: 'Delay', body: '#ececec', ink: '#151515',
      on: 0, type: 1, typeUi: 'seg',
      types: {
        0: { groups: [['Time', [2, 3, 5]], ['Filter', [6]], ['Output', [7]]], main: [2, 5, 6, 7] },
        1: { groups: [['Time', [4, 5]], ['Filter', [6]], ['Output', [7]]], main: [4, 5, 6, 7] },
      },
      labels: { 2: 'Time', 3: 'Fine', 4: 'Tap', 5: 'Fdbk', 6: 'Hi Cut', 7: 'Level' },
    },
    {
      code: 8, block: 0x08, abbr: 'CE', title: 'Chorus', body: '#7cc3e6', ink: '#0f1f28',
      on: 0, seg: 1,
      types: { null: { groups: [['Chorus', [1, 2, 3, 4]], ['Filter', [5]], ['Output', [6]]], main: [2, 3, 4, 6] } },
      labels: { 1: 'Mode', 2: 'Rate', 3: 'Depth', 4: 'Pre Dly', 5: 'Hi Cut', 6: 'Level' },
    },
    {
      code: 9, block: 0x09, abbr: 'RV', title: 'Reverb', body: '#3b4966', ink: '#f2f4f8',
      on: 0, type: 1, typeUi: 'lcd',
      types: Object.fromEntries(Array.from({ length: 5 }, (_, t) => [t, {
        groups: [['Reverb', [2, 3, 6]], ['Filter', [4, 5]], ['Output', [7]]], main: [2, 3, 6, 7],
      }])),
      labels: { 2: 'Time', 3: 'Pre Dly', 4: 'Lo Cut', 5: 'Hi Cut', 6: 'Density', 7: 'Level' },
    },
    {
      code: 10, block: MASTER, abbr: 'NS', title: 'Noise Suppressor', body: '#b8bdc5', ink: '#151515',
      on: 3,
      types: { null: { groups: [['Noise Suppressor', [4, 5]]] } },
      labels: { 4: 'Thresh', 5: 'Release' },
    },
    {
      code: 11, block: MASTER, abbr: 'FV', title: 'Foot Volume', body: '#1e1f23', ink: '#e8e8ea',
      on: null, // always in the chain; the EXP pedal moves it
      types: { null: { groups: [['Foot Volume', [6]]] } },
      labels: { 6: 'Level' },
    },
  ];

  const MASTER_KNOBS = [[MASTER, 0, 'Level'], [MASTER, 1, 'BPM']];

  const BY_CODE = new Map(PEDALS.map(p => [p.code, p]));
  const DEFAULT_ORDER = PEDALS.map(p => p.code);

  // The type entry of a pedal for the patch's current type value (null key for one-type pedals).
  function typeOf(pedal, patch) {
    if (pedal.type === undefined) return { value: null, def: pedal.types.null };
    const value = GT3.getValue(patch, pedal.block, pedal.type);
    return { value, def: pedal.types[value] || null };
  }

  const isOn = (pedal, patch) => pedal.on === null || GT3.getValue(patch, pedal.block, pedal.on) === 1;

  // A parameter's caption: the pedal's own label, else its midi.xml name tidied up.
  function caption(pedal, offset) {
    return pedal.labels[offset] || GT3.param(pedal.block, offset).name.replace(/^.*:\s*/, '');
  }

  // The midi.xml value texts, made to fit under a knob: note values as 1/8, 1/8T, 1/8.
  const NOTES = { whole: '1/1', half: '1/2', quarter: '1/4', eighth: '1/8', sixteenth: '1/16' };
  function shortValue(text) {
    let t = text.replace(/\b(whole|half|quarter|eighth|sixteenth) note/, (_, n) => `${NOTES[n]} note`);
    t = t.replace(/^(doted |# )?(1\/\d+) note( triplet)?$/, (_, dot, n, tri) => n + (tri ? 'T' : '') + (dot ? '.' : ''));
    return t.replace(/mS$/, 'ms').replace(/ (%|ms|dB)$/, '$1').replace(/''$/, '').trim();
  }

  const valueText = (block, offset, value) => shortValue(GT3.label(GT3.param(block, offset), value));

  // Which control fits a parameter: knob for ranges and numeric steps, a
  // segmented switch for 2-4 named choices, a list for longer named lists.
  function widgetFor(p) {
    const table = GT3_PARAMS.tables[p.t];
    if (!table.every(Array.isArray)) return 'knob';
    if (p.max - p.min < 4) return 'seg';
    const numeric = table.filter(e => /^[-+]?\d/.test(e[1])).length;
    return numeric >= table.length * 0.8 ? 'knob' : 'select';
  }

  // Effect order of a patch; a broken chain block (garbage in old files) shows in the default order.
  function order(patch) {
    const chain = GT3.chain(patch);
    return GT3.isValidChain(chain) ? chain : DEFAULT_ORDER;
  }

  // The pedal knobs: the type's `main` list, else its first four knob-type parameters.
  function mainParams(pedal, def) {
    if (def.main) return def.main;
    const all = def.groups.flatMap(g => g[1]);
    return all.filter(o => widgetFor(GT3.param(pedal.block, o)) === 'knob').slice(0, 4);
  }

  return {
    PEDALS, BY_CODE, DEFAULT_ORDER, MASTER, MASTER_KNOBS,
    typeOf, isOn, caption, shortValue, valueText, widgetFor, order, mainParams,
  };
})();
