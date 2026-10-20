# GT-3 MIDI Editor

A patch editor for the **BOSS GT-3 Guitar Effects Processor** (1999), controlled over MIDI SysEx from a computer or phone. It is a browser port of the Qt [GT-3 FxFloorBoard](http://sourceforge.net/projects/fxfloorboard/).

**Open it in the browser:** https://kirlaca.github.io/BOSSGT3-MIDI/

No install needed: the editor runs entirely in Chrome or Edge using the Web MIDI API.

> **Status:** feature-complete, but **not yet tried with a real GT-3** – only against a simulated unit and the 325 real patch files that ship with the FloorBoard. Back up your patches before writing to the unit (Library → Read from GT-3 → Save selected…). [`WEBMIDI_FELDERITES.md`](WEBMIDI_FELDERITES.md) lists what is still to be checked on the hardware.

## What it does

- **Effects page:** the patch's 12 effect blocks as BOSS-style pedals – SFX, CS, WAH, OD/DS, PRE, EQ, MOD, DD, CE, RV, NS, FV – standing in the patch's own effect chain order, which can be rearranged by dragging. Type-dependent blocks (6 SFX, 12 OD/DS, 14 preamp, 12 MOD types) swap their knobs with the type; every parameter of every type is in the detail panel.
- **EXP / CTL / Assign page:** the expression pedal, CTL pedal and the 8 assigns – 207 targets, Min / Max in the target's own units, source, mode, active range, response curve, internal and wave pedal.
- **Library page:** all 340 patches (140 user + 200 preset) in a list; read, write, copy, init and save many at once. Patches changed only in the editor are marked.
- **System page:** tuner, output select, global EQ, controls, patch change, MIDI and manual mode settings.
- Edits are sent to the unit's temporary patch live, so you hear them immediately; Write stores them in a user patch.
- Reads and writes the FloorBoard's `.syx` files (single patches, bulk dumps, system dumps), so files move freely between the two programs.

## What you need

- A BOSS GT-3.
- A USB-MIDI interface connected to both MIDI IN and MIDI OUT of the unit. Use a decent one: cheap no-name adapters often pass Program Change but drop SysEx.
- Chrome or Edge on desktop or Android. Safari and iOS do not support Web MIDI.

## Using it

1. Connect the interface, crossed over: interface OUT → GT-3 MIDI IN, interface IN ← GT-3 MIDI OUT.
2. Open the editor and allow MIDI / SysEx access when the browser asks.
3. Use auto-detect (it tries every port and all 32 device IDs), or pick the ports, device ID and patch change channel yourself.
4. After connecting, the editor goes to its current patch on the GT-3 and reads it; other patches are read when you select them. Until the current patch is read, edits are not sent and auto write is held (the status shows "not read"), so a Write can't replace your own sound with the editor's default patch. **Read GT-3** loads all 140 user patches at once.

User guides: [English](webmidi/guide.html) · [Magyar](webmidi/utmutato.html). They can also be opened from the editor's header.

## Repository layout

| Path | What it is |
|---|---|
| `webmidi/` | **The editor.** Plain HTML/CSS/JS, no build step, no server. Published on GitHub Pages. See [`webmidi/README.md`](webmidi/README.md) (Hungarian) for the files and the protocol. |
| `index.html` | GitHub Pages entry point; redirects to `webmidi/`. |
| `tools/gen_gt3_params.py` | Generates `webmidi/gt3-params.js` (every parameter, value table and assign target) and `webmidi/factory.js` from the FloorBoard's `midi.xml`, `default.syx` and `system.syx`. |
| `tools/test_gt3.mjs`, `tools/test_midi.mjs` | Tests without a GT-3: the protocol against the FloorBoard's own formulas and 325 real patch files, and `midi.js` against a simulated GT-3. |
| `gt-3fxfloorboard_source_code_20250420/` | The data files of the Qt FloorBoard that the generator and the tests read (`midi.xml`, `default.syx`, `system.syx`, the 325 init patches), with its license. |
| `WEBMIDI_FELDERITES.md` | Survey, design notes and progress log (Hungarian). |

### Running locally

Serve the editor from any local web server, for example:

```
python -m http.server -d webmidi
```

Then open the printed `http://localhost:...` address. Browsers may refuse MIDI access on `file://` pages.

### Regenerating and testing

```
python -I tools/gen_gt3_params.py   # midi.xml -> webmidi/gt3-params.js, webmidi/factory.js
node tools/test_gt3.mjs             # protocol, default.syx / system.syx, 325 real patches
node tools/test_midi.mjs            # midi.js against a simulated GT-3
```

## Visitor statistics

The GitHub Pages build counts page views with [GoatCounter](https://www.goatcounter.com/) (`webmidi/index.html`, `guide.html`, `utmutato.html`). It sets no cookies and sends only the page path, the referrer and the screen size, never any MIDI or patch data. Views from `localhost` are not counted, and nothing is sent while offline.

The editor also counts a few actions as GoatCounter events (only the event name is sent):

| Event | When |
|---|---|
| `no-webmidi` | The page was opened in a browser without Web MIDI (Safari, iOS) |
| `midi-denied` | The browser refused MIDI / SysEx access |
| `connect-auto`, `connect-manual`, `reconnect` | Connected by auto-detect, by picking ports, or restored from the last session |
| `detect-not-found` | Auto-detect found no GT-3 |
| `read-gt3`, `read-gt3-failed` | Read GT-3 (all 140 user patches) succeeded or failed |
| `write-patch`, `write-patch-offline`, `auto-write-on` | A patch was written by hand (to the GT-3 / editor only), auto write was turned on |
| `load-file`, `save-file` | File load and save on the patch page |
| `library-read`, `library-write`, `library-copy`, `library-init`, `library-save` | Library page actions |
| `system-read`, `system-send` | System settings read from or sent to the GT-3 |

Statistics: https://gt3-midi.goatcounter.com/

## Credits and license

The parameter map, value tables, patch format and SysEx protocol come from the **GT-3 FxFloorBoard** by Colin Willcocks (gumtownbassman), based on the FxFloorBoard by Uco Mesdag – licensed under the GNU GPL v2 or later. The data files the tools read are included unchanged in `gt-3fxfloorboard_source_code_20250420/`; the C++ source is not, as the editor does not use it. The look and structure of the editor follow the author's [ME-5 MIDI Editor](https://github.com/kirlaca/ME5-MIDI).

Licensed under the GNU GPL v3. See [LICENSE](LICENSE). No warranty: use at your own risk.

BOSS and GT-3 are trademarks of Roland Corporation. This project is not affiliated with Roland.
