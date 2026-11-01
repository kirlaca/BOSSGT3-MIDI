# GT-3 FxFloorBoard → Web MIDI port – felderítés

Dátum: 2026-10-06 (frissítve 2026-11-01) · Állapot: **minden lépés kész**, a 3. (próba a pedállal) 2026-11-01-én –
lásd a „2026-11-01” szakaszt. Nyitott: az EXP / CTL / Assign és a System oldal írása a pedálon, a teljes patch temp-be küldése. Részletek: [`webmidi/README.md`](webmidi/README.md).

## Döntés

A Qt/C++ programot **újraírjuk JS-ben** (nem Qt for WebAssembly). Minta és design:
`D:\BOSSME5\webmidi\` (a ME-5 Web MIDI editor) – ugyanaz a felépítés és stílus.

## A mostani program (Qt/C++)

- ~150 fájl, ~30 ezer sor C++, Qt `widgets` + `xml` + `printsupport`.
- MIDI: `midiIO.cpp` (RtMidi, QThread, blokkoló `msleep` várakozások) – ezt váltja ki a Web MIDI.
- SysEx logika: `SysxIO.cpp` (patch adat tárolása/olvasása), `MidiTable.cpp` (üzenetek összerakása).
- Paramétertábla: `midi.xml` (~1,4 MB, ~1850 paraméter-definíció) – **újrahasznosítható**.
- Effekt blokkok: `stompbox_*.cpp` (CS, WAH, OD, PRE, EQ, FX1, FX2, DD, CE, RV, NS, FV).
- Egyéb oldalak: `menuPage_assign/midi/system.cpp`, bulk load/save, bank lista.
- Újrahasznosítható még: `default.syx`, `system.syx`, `images/`.

## A ME-5 webmidi minta felépítése → GT-3 megfelelő

| ME-5 webmidi | GT-3 |
|---|---|
| `midi.js` – requestMIDIAccess(sysex), sendAndCollect, auto-detect, port figyelés | szinte változatlanul átvehető; GT-3 identity/dump kérés kell bele |
| `me5.js` – paramétertábla + DT1/RQ1 kódolás | `gt3.js` – a `SysxIO`/`MidiTable` logika; a paramétertábla **generált** fájl a `midi.xml`-ből |
| `app.js` – pedálok (SVG knob, fader, kapcsoló), patch térkép, LED kijelző | ugyanez, a GT-3 blokkjaival; FX1/FX2-nél típusonként más knob-készlet |
| `factory.js` – beágyazott gyári dump | `default.syx` / `system.syx` beágyazva |
| `ask.js`, `style.css` | egy az egyben |

## SysEx protokoll (a kódból kiolvasva)

Fejléc: `F0 41 <dev> 00 1B <cmd>` – Roland, **kétbájtos model ID (`00 1B`)**, `11` = RQ1, `12` = DT1.
Cím: **4 bájt**. Checksum: szabványos Roland (cím + adat), `checksumOffset = 6`.

Élő szerkesztés (`MidiTable::dataChange`, `MidiTable.cpp:540`):

```
F0 41 <dev> 00 1B 12 | 0C 00 <blokk> <param> | <érték> | <checksum> F7
```

- Identity request: `F04100001B1100000000000000017FF7`, várt válasz eleje: `F04100001B12`
- System request: `F04100001B1100000000020000017DF7`
- Patch: **23 SysEx csomag, 650 bájt** (`patchPackets`, `patchSize`); név offset 374, hossz 11.
- Patch kérés (`MidiTable::patchRequest`): user patchek a `07 xx 00 00` címtől
  (35 user bank × 4 patch, 85 bank összesen, a preset patchek új memóriatartományban kezdődnek).
  Temp buffer olvasás: `0B 00 00 00`.

### Blokkok (`midi.xml` → `<Structure>`)

| Blokk | Tartalom | Blokk | Tartalom |
|---|---|---|---|
| 00 | SFX (Acoustic sim, Slow Gear, AFB, FB, Picking, Tremolo) | 0A | Master |
| 01 | Compressor | 0B | FX Chain (sorrend) |
| 02 | Wah | 0C | Name |
| 03 | OD/DS | 0D | EXP pedál |
| 04 | PreAmp + Spkr Cab | 0E | CTL pedál |
| 05 | EQ | 0F–16 | Assign 1–8 |
| 06 | MOD (Harmonist, Flanger, Phaser, Auto Riff, Sub EQ, …) | 17–19 | leírás / szerző / jegyzet (csak az editoré) |
| 07 / 08 / 09 | Delay / Chorus / Reverb | | |

System terület: Tuner, Global, System, Midi, Manual (`midi.xml` → `<System>`).

## Assign vs. ME-5 fix címzés

- **A GT-3 címzése is fix** (blokk + param), mint a ME-5-é – csak hosszabb (4 bájt) és több blokk.
- Az **assign egy patch-funkció**, nem címzési mód: 8 assign blokk (`0F`–`16`), mindegyikben
  Target (kétbájtos kód → melyik paramétert vezérli), Target min/max, Source, Source mode, range,
  internal pedal (trigger/time/curve), wave form/rate. Ugyanez a logika az EXP és CTL pedálnál (`0D`, `0E`).
- Nincs előre letárolva a `SysxIO`-ban: ugyanolyan patch-adat, mint egy knob, benne van a 650 bájtban.
- UI-oldali teendő: a Target lista a `midi.xml` target táblájából; cél váltásakor a min/max knobok
  tartományát és feliratait a cél paraméter adataiból kell újraszámolni (lásd `customControlTarget.cpp`).
  A generált JS táblába a „target kód → blokk/param” leképezés is kell.

## Hardver (szervizkézikönyvből)

Forrás: [`docs/GT-3_Service_Notes_1999.pdf`](docs/GT-3_Service_Notes_1999.pdf) (BOSS GT-3 Service Notes, First Edition, 1999. feb.)
– specifikáció, önteszt menü, blokkdiagram, kapcsolási rajzok.

- **Az OD/DS szekció analóg áramkör.** A blokkdiagramon két AK4522 codec van: az egyik a bemenet/kimenet, a
  másik csak az „Analog Distortion” ki- és visszavezetése. Jelút: A/D → DSP → D/A → **analóg OD/DS** → A/D → DSP → D/A.
  Az önteszt „DS A/D/A” pontjai (GAIN LOW/HIGH, OVERDRIVE, CV CHECK) ezt mérik; a torzító paramétereit a CPU
  **vezérlőfeszültséggel (CV, 0–100)** állítja. Ezért tehető az OD/DS az FX Chainben bárhová.
- **„External” OD típus** (az OD típuslista 12. eleme) – valószínűleg az **EXT OD/DS SEND/RETURN** hurok
  (hátlapi jackek, SEND 2 kΩ, RETURN −10 dBm / 220 kΩ): a lánc OD/DS helyén külső pedál szól.
  UI teendő: External típusnál jelezni, hogy a Drive/Bass/Treble nem a belső torzítót állítja (pedálon ellenőrizni,
  mit csinálnak ilyenkor ezek a paraméterek).
- Specifikáció: A/D 24 bit AF, 64× túlmintavételezés · D/A 20 bit, 128× · 44,1 kHz · 340 patch (140 user + 200 preset)
  · INPUT −10 dBm / 1 MΩ · OUTPUT 2 kΩ · dinamika ≥ 100 dB (IHF-A).
- Belül: H8/3006 CPU, külön DSP, 4 Mbit ROM, 4 Mbit DRAM, 1 Mbit SRAM; vezérlők: EXP, SUB EXP, SUB CTL 1/2, 4 + CTL lábkapcsoló.

## A ME-5-höz képest új nehézségek

1. **Több csomagos patch** (23 csomag): kiolvasáskor összerakni, íráskor darabolni, csomagonkénti checksum.
2. **Típusfüggő blokkok**: SFX és MOD alatt sok effekt típus, mindnek saját paraméterkészlet.
3. **Assign / EXP / CTL** target-rendszer (fent).
4. **FX Chain** – változtatható effekt-sorrend (ME-5-nél fix lánc volt).
5. **Bank/patch kezelés**: 35 user bank × 4 + preset bankok, bulk load/save.

## Pedálon ellenőrizendő a munka elején

- [ ] Temp buffer írási cím: a kód `0C 00`-t használ (a `default.syx` is ezzel van mentve), a temp visszaolvasás
      `0B 00`-ról megy – a `midi.xml` fejlécében lévő `0A`/`0B` nem használt. Pedálon megerősíteni.
- [ ] Milyen címmel válaszol a GT-3 a `0B 00` temp-kérésre (a `gt3.js` a `0B 00`-t és a `0C 00`-t is temp-nek veszi).
- [ ] Device ID ↔ MIDI csatorna megfelelés (ME-5-nél dev = csatorna − 1 volt).
- [ ] Identity request valóban válaszol-e, és milyen hosszan.
- [ ] Patch dump időzítése: kell-e szünet a 23 csomag között (régi pedál, lassú MIDI puffer).
- [ ] Program Change / bank select működése a patchváltáshoz.
- [ ] OD típus „External”: valóban az EXT OD/DS send/return hurkot kapcsolja-e be, és mit csinál ilyenkor a Drive/Bass/Treble.
- [ ] Élő szerkesztés: a knob-tekerés (25 ms-onként összevont DT1-ek) hallatszik-e azonnal, nem fullad-e be a MIDI puffer.
- [ ] Write (user patchbe írás) és a Library „Write to GT-3” több patchre (100 ms szünet patchenként elég-e).
- [ ] Assign: a Toggle mód és az aktív tartomány valódi viselkedése; Min > Max fordított irányt ad-e.
- [ ] System: a rendszerdump olvasási ideje (kb. 4,5 kB, ~1,5 s), és a „Send all” egyben elfogadja-e a teljes dumpot.
- [ ] A próba után az útmutatók (`utmutato.html`, `guide.html`) ellenőrizetlennek jelölt részeit és az „Állapot”
      figyelmeztetést frissíteni.

**Folytatás a pedállal:** `webmidi/index.html` megnyitása → Offline → Auto-detect → patchváltás ◀ ▶ → egy ismeretlen
patch kiválasztása (kiolvasás) → Library: User → Select shown → Read from GT-3 → Save selected… (biztonsági mentés),
System → Save file… – és csak ezután bármilyen írás. Közben a fenti lista pontjai sorban.

## Tervezett lépések és becsült munka

| # | Lépés | Becslés (Claude-dal közös munkamenet) |
|---|---|---|
| 1 | `midi.xml` → `gt3-params.js` generátor szkript (paraméterek, típusok, target tábla) | 1 |
| 2 | `gt3.js` protokoll: fejléc, checksum, patch összerakás/darabolás, temp írás + `midi.js` átvétele | 1–2 |
| 3 | Pedálon tesztelés: kapcsolódás, temp írás, patch olvasás (a fenti ellenőrzőlista) | 1 |
| 4 | Alap UI a ME-5 designjával: kijelző, patch nav, Write/Revert, egyszerű blokkok (CS, WAH, OD, EQ, DD, CE, RV, NS, Master) | 2 |
| 5 | Típusfüggő blokkok: PreAmp, SFX, MOD (sok típus) | 2–3 |
| 6 | FX Chain szerkesztő, EXP/CTL/Assign oldalak a target-rendszerrel | 2 |
| 7 | Library: patch lista, bankok, fájl Load/Save, bulk, gyári dump, System/MIDI beállítások | 1–2 |
| 8 | Útmutatók (HU/EN), finomítás, hibajavítás a pedállal | 1–2 |

**Összesen kb. 11–15 munkamenet**, nagyjából 3–4-szerese a ME-5 webmidi változatnak.
A legnagyobb tétel a típusfüggő effektek (5.) és az assign rendszer (6.) felülete.
Az 1–2. lépés a pedál nélkül is elkezdhető; a 3. lépéstől kell a pedál.

## Haladás

### 2026-10-06 – 1–2. lépés (pedál nélkül)

- `tools/gen_gt3_params.py` → `webmidi/gt3-params.js` (26 blokk, 503 patch-paraméter, 29 system-paraméter,
  207 assign cél, 93 értéktábla) és `webmidi/factory.js`. A blokkméreteket a `default.syx`-szel ellenőrzi.
  A `midi.xml` egy elírását javítja (`SYN:Octave Shift` „01=-2” → 02).
- `webmidi/gt3.js` – protokoll és patch modell; `webmidi/midi.js` – Web MIDI (a ME-5 `midi.js`-éből).
- `tools/test_gt3.mjs` – 24 ellenőrzés: a Qt képletei (patchRequest mind a 340 patchre, programváltás,
  dataChange, checksum), a `default.syx` bájtra pontos visszaírása, és mind a 325 valódi patch
  (`packager/init patches`) bájtra pontos visszakódolása.
- `tools/test_midi.mjs` – szimulált GT-3 elleni teszt: felismerés, temp és user patch olvasás, írás, programváltás.
- Észrevételek a valódi patchekből: 17 fájlban a Master BPM `50 50` (érvénytelen, valószínűleg régi hibás mentés),
  5 fájlban a Phaser Step Rate 0 (a `midi.xml` 1-től engedi), 1 fájlban (`mtllica1.syx`) szemét az Assign 1-ben.
  A felületnek ezeket meg kell tudnia jeleníteni, de érvényes értéket csak tartományon belül írunk.

### 2026-10-15 – látványterv, szervizkézikönyv

- Látványterv (claude.ai Design canvas): https://claude.ai/artifact/PvMfufEAikA5xNHcUsTqRB – két tábla:
  *Effektek* (LED + zöld LCD kijelző, patch térkép, FX Chain sáv, pedáltábla, SFX/MOD típusváltáskor cserélődő
  potméterek) és *EXP / CTL / Assign* (vezérlőlista, Target → Min/Max a cél mértékegységében, Source, válaszgörbe,
  Internal/Wave pedal). Színek: sötétkék alap + arany „GT-3”, mint a gépen.
- `docs/GT-3_Service_Notes_1999.pdf` a projektbe mentve; belőle a fenti „Hardver” rész (analóg OD/DS, External típus).

### 2026-10-17 – 4–5. lépés: effektek oldal (pedál nélkül)

- `webmidi/index.html`, `app.js`, `effects.js`, `style.css`, `ask.js` – a látványterv *Effektek* táblája működő
  felületként: LED + LCD kijelző (átnevezés), User/Preset patch térkép, FX Chain húzással átrendezve
  (`GT3.setChain`), mind a 12 blokk pedálként a patch saját sorrendjében, Master (Level, BPM).
- Típusfüggő blokkok is kész: SFX (6 típus), MOD (12 típus), PRE (14), OD/DS (12, External megjegyzéssel), RV (5),
  CS / WAH / DD kapcsolóval. A pedálon a fő knobok, a névtáblára kattintva alul a részletes panel a típus összes
  paraméterével (a Qt szerkesztőoldalainak csoportjai szerint) – minden blokk minden paramétere elérhető.
- Write párbeszédablak (célpatch választható, preset → user), Auto write, Revert, Init patch, fájl Load/Save
  (egy patch vagy a 140 user patch), Read GT-3 (140 user patch), preset patch kiolvasása kiválasztáskor.
- Kapcsolódás: auto-detect a 32 device ID-n; a patchváltás csatornája külön állítható (a device ID ↔ csatorna
  megfelelés még pedálon ellenőrizendő, addig `dev % 16`).
- Hátravan: EXP / CTL / Assign oldal (6.), Library / System oldal (7.), útmutatók (8.) – és minden a pedállal.

### 2026-10-17 – 6. lépés: EXP / CTL / Assign oldal (pedál nélkül)

- `webmidi/assign.js` + fülek az `index.html`-ben (`#effects` / `#assign`): a látványterv második táblája
  működő felületként. Vezérlőlista (EXP, CTL, Assign 1–8), Target lista blokkonként csoportosítva, Min / Max
  a cél paraméter saját értéktáblájával kiírva, célváltáskor a cél teljes tartományára állítva
  (`customControlTarget::knobSignal` szerint), Source / mód / aktív tartomány, válaszgörbe, Internal és Wave pedal.
- Ellenőrizve (szimulált MIDI kimenettel): célváltás → 3 DT1 (`0C 00 0F 01 <msb lsb>`, Min, Max), a kétbájtos
  értékek 7-7 bitesek, checksum rendben.
- Pedálon ellenőrizendő: a Toggle mód és az aktív tartomány valódi viselkedése (a válaszgörbe a kézikönyv
  szerinti feltételezés), és hogy a Min > Max (fordított irány) működik-e.

### 2026-10-17 – 7. lépés: Library és System oldal (pedál nélkül)

- `webmidi/library.js`: mind a 340 patch listában (típusok, bekapcsolt effektek, állapot), kijelölt patchek
  olvasása / írása a GT-3-ra (folyamatjelzővel), másolása user helyekre, init, mentése egy fájlba. Az app nyilvántartja,
  melyik user patch van csak a szerkesztőben (fájlbetöltés, másolás, offline Write) – ezek a patch-térképen is jelölve.
- `webmidi/system.js` + `GT3.decodeSystem / getSystemValue / setSystemValue / encodeSystem`: a rendszerterület
  cím szerinti darabokban, így a GT-3-ból vagy fájlból jött dump bájtra azonosan megy vissza. Szerkesztés = azonnali
  DT1 a `<terület> <offset>` címre (a GT-3 rögtön tárolja). Kapcsolódva az oldal egyszer kiolvassa a beállításokat.
- Az app vezérlői (knob, kapcsoló, lista, csúszka) mostantól bármilyen értékforráshoz köthetők, nem csak patch-adathoz.
- Generátor-javítás: a MIDI beállítások neve (`RX Channel`, `EXP Pedal Out` …) és értékfeliratai (`1`–`16`, `CC#01` …)
  a midi.xml-ben a `customdesc` / `desc` mezőben vannak – eddig üresen jöttek át. Más tábla nem változott.
- Tesztek: `test_gt3.mjs` 27/27 (új: system dump értékei, szerkesztés címe, encode→decode), `test_midi.mjs` 7/7
  (új: rendszerterület olvasása a szimulált GT-3-ból).
- Pedálon ellenőrizendő: a rendszerdump olvasási ideje (kb. 4,5 kB, ~1,5 s a vezetéken), és hogy a „Send all”
  egyben elfogadja-e a teljes dumpot, vagy kell szünet az üzenetek között.

### 2026-10-17 – 8. lépés: útmutatók

- `webmidi/utmutato.html` (magyar) és `webmidi/guide.html` (angol), a ME-5 útmutató felépítésében, `pages.css`
  GT-3 színekkel; az editor fejlécéből elérhetők. Tartalom: bekötés, indítás, csatlakozás, a fejléc, mind a négy oldal,
  mi jut el a GT-3-ra és mikor, fájlok és biztonsági mentés, gyakori hibák (a kód valódi hibaüzeneteivel).
- Az elején figyelmeztetés: valódi GT-3-mal még nem próbáltuk ki. A pedálon még nem ellenőrzött részeket
  (device ID ↔ csatorna, Min > Max) az útmutató is így jelzi – a pedálos próba (3. lépés) után frissíteni kell.

### 2026-11-01 – első próba a pedállal (DOREMiDi, Python tesztelő)

- Új eszközök: `tools/gt3_tester.py` (a GT-5-ös tesztelőből: `ports`, `connect --ask id|temp|temp0c|patch`, `scan`,
  `listen`, `receive`, `handshake`) és `tools/gt3_param.py <0B|0C> <blokk> <offset> <érték>` (egy DT1 a temp patchbe).
- A pedál: Device ID 1 (SysEx-ben `00`). Patchváltáskor küld: `B0 00 00`, `B0 20 00`, `C0 nn` (1-es csatorna).
  A gépről küldött programváltást is elfogadja.
- ~~**Normál (play) módban a GT-3 semmilyen SysEx-re nem reagál:**~~ (téves, lásd a 19:00-s helyesbítést – a DT1 működik, csak az RQ1 nem) az identity kérésre (00–1F és 7F) nincs válasz;
  Master Level DT1 a `0C 00 0A 00` és a `0B 00 0A 00` címre: nincs hatás, akkor sem, ha a kijelzőn a Master Level áll.
  → A böngészős szerkesztő ezért nem tudta kiolvasni a patchet, így a patchváltáson kívül minden tiltva maradt.
- **BULK LOAD módban** (UTILITY 4× → PARAMETER 10×, kijelzőn „waiting”):
  - identity kérés: válasz dev `00`-n és `7F`-en: `F0 41 00 00 1B 12 00 00 00 00 05 01 7A F7` (1 bájt kérésre 2 jön);
  - patch kérés (`07 00 03 00`, 6 bájt): **a kért mérettől függetlenül az egész patch jön**, 23 blokk, mind jó checksum, 0,85 s;
  - temp kérés (`0B 00 03 00`): az egész temp patch, **`0B 00` címmel** (a `0C 00`-s kérés még nem próbált);
  - DT1 a `0B 00 0A 00` és a `0C 00 0A 00` címre is beíródik a tempbe (visszaolvasva `1E`, majd `3C`);
  - **EXIT után szól** (Master Level 30 → halk, 60 → hangosabb, a kijelző is mutatja). SCR nem kell.
- A Qt FloorBoard is így működik: a kapcsoló gomb neve „Bulk Mode”, válasz nélkül „BULK MODE connection Required”;
  paraméterenként egy DT1 a `0C 00` címre, utána semmi; Write előtt „Ensure Bulk Load Mode is selected”.
- GT-3 MIDI Implementation dokumentum a neten nem található; a kézikönyv csak a Bulk Dump / Load menetét írja le.
- BULK módban **szól a gitár** (a módosítás előtti hanggal); változás nélkül az EXIT azonnali, a hang nem változik.
- **Effekt be/ki** (OD/DS On `0C 00 03 00 = 01`, BULK + EXIT): LED ég, torzítás szól – a GT-5-tel ellentétben jó.
- **Típusváltás** (OD Type Turbo OD → Metal 2, `0C 00 03 01 = 09`, BULK + EXIT): kijelzőn Metal 2, metálos torzítás szól.
- **Sorozat** (OD Drive 40 → 100, 13 DT1 25 ms-onként, BULK): nincs hiba, a temp `64`, EXIT után szól.
- **Mentés:** a GT-3 BULK DUMP „System” pontja a rendszerterülettel együtt **minden user patchet** is küld
  (`00`–`02` rendszer, `03` és `05` ismeretlen területek, `07 00`–`08 0B` patchek), 140 patch ≈ 2 perc.
  `gt3-dump-system.syx`: rendszer + U01-1…U31-4 (120 s-nál levágva); `gt3-bulk-dump.syx`: az eleje hiányzik
  (a WRITE a fogadás előtt indult), a vége a `08 05` patch 4. blokkjánál megszakadt – a GT-3 szerint rendben végzett.
- `gt3_tester.py read` (patchenként RQ1, BULK módban, 23 jó blokk = kész): `gt3-user-patches.syx` = U01-1…U17-2
  (66 patch) hibátlan, utána **a bemenet elhallgatott**.
- Azóta: a GT-3 semmire nem válaszol; figyeléskor a patchváltás sem jön be (újradugás, BULK újra, GT-3 + interfész
  újraindítás után egyszer még egy PC átjött, utána semmi; másik USB-port: semmi). A gép → GT-3 irány él
  (`C0 01` → U01-2). Ellenpróba: a GT-5 MIDI OUT-járól a DOREMiDi mindent átvett; a dugót a GT-3 MIDI OUT-jába **határozottan
  visszadugva** a GT-3 patchváltásai ismét hibátlanul jönnek. → **Laza / kontakthibás csatlakozás** volt a GT-3 MIDI OUT-on.
- Utána a `read` 35 patch után megint elakadt (a kábelhez nem nyúltak); csak az **USB újradugása** segített →
  a DOREMiDi / Windows bemenet hosszabb SysEx-folyam után beragad (mint a GT-5-nél). Patchenként 0,5 s szünettel
  (`read --gap 0.5`) a maradék 39 patch hibátlanul jött.
- **Teljes mentés:** `gt3-backup-2026-11-01.syx` (97 798 bájt) = a rendszerterület (`00`–`05`, 164 üzenet) + mind a
  140 user patch (23 blokk, 0 hibás checksum). A GT-3 saját dumpjaival összevetve 5924 átfedő blokk bájtra azonos.
- **Write a user memóriába** (`tools/gt3_write.py`, U35-4 = `08 0B`, 23 DT1 20 ms-onként, BULK módban): EXIT-kor
  kb. 5 s „Checking…”, utána tárolva. Visszaolvasva csak a név blokkja (`0C`) tér el a mentéstől („WRITE TEST”),
  a többi 22 blokk bájtra azonos. Visszaállítás a mentésből: rendben – de **ha az írt patch az épp betöltött, a
  pedál a régit szólaltatja / mutatja tovább, amíg patchet nem váltanak** (az írás a memóriát frissíti, a tempet nem).
  → A szerkesztő Write után küldjön programváltást az írt patchre (vagy írja a tempet is).
- **Élő vezérlés MIDI CC + Assign-nal (normál módban):** U35-4 Assign 2 = On, Target Master Level (`01 46`), Min 0,
  Max 100, Source CC#20 (`1A`), Normal (BULK-írás, EXIT, patchváltás oda-vissza). CC#20 0 → 127, 20 lépés 2 s alatt
  (`tools/gt3_cc.py 20 0 127 20 100`): **a hangerő tekerés közben fokozatosan nőtt** – azonnal hallható.
  CC#20 = 0 → elhalkul; patchváltás oda-vissza után **néma marad**: a pedál rendszerében **Assign Hold = On**
  (`01 02 06` = 01, a mentésből), így betöltéskor az utolsó CC-érték érvényes, nem a tárolt (mint a GT-5-nél).
  → A szerkesztőnek patchbetöltés után ki kell küldenie az élő knobok értékét.
- **Típusváltás CC-vel élőben:** U35-4 OD On, Assign 2 = OD Type (`00 31`), 0–11, CC#20. Betöltéskor (Hold, CC 127)
  EXT OD/DS; CC 0 → kijelzőn Natural OD, torzít; CC 104 → Metal 2 (kijelzőn is), sokkal erősebb torzítás. A típus
  CC-vel azonnal és helyesen vált – sima patch kijelzésből
  megismételve (CC 0 → Natural OD, CC 104 → Metal 2) is (a GT-5 SysEx-es típusváltásának gondjai nélkül). Leképezés: érték × 11 / 127, lefelé.
  → A szerkesztő élő knobjai: szabad Assign-ok CC-forrással (patchenként ≤ 8), egyszeri BULK-írással beállítva.
- **Böngészős próba – élő knobok** (`7bbc53c`): Auto-detect BULK LOAD-ban csatlakozott és kiolvasott; két knob
  kijelölve, Live (BULK → Send → EXIT → Done) után a Master és a PRE Master / Middle tekerése **hallhatóan, azonnal
  változtatta a hangot**. A kezelés (UX) még megbeszélendő.
- **HELYESBÍTÉS (19:00): normál módban a DT1 élőben működik.** Böngészőből (BULK-os csatlakozás + EXIT után)
  pötty nélkül is szólt minden szerkesztés. Pythonból, normál módban: OD/DS On (`0C 00 03 00 = 01`) → LED + torzítás;
  **friss bekapcsolás után, BULK nélkül is**; Master Level = 0 → néma, 100 → vissza. A reggeli „nincs hatás” mérési
  hiba volt: csak a Master Level 100 → 30-at néztük (nehezen hallható), és **a GT-3 kijelzője nem követi a SysEx-es
  értékváltozást, módosítás-jelet sem mutat**. RQ1-re normál módban továbbra sincs válasz (19:01-kor újramérve).
  → Végleges kép = a Qt FloorBoard felépítése: **olvasás (és Write) BULK LOAD-ban, szerkesztés normál módban, élőben.**
  A reggeli tünet oka: a szerkesztő normál módban nem tudta kiolvasni a patchet, ezért minden küldést letiltott (`needsRead`).
  A pöttyös Assign + CC élő knobok (`7bbc53c`) feleslegesek, visszavonva (`9a49280`); a CC + Assign mérések ettől még
  érvényesek (élő kapcsolókhoz később jól jöhetnek). Új menet (`e00077f`): Auto-detect BULK-ban → felajánlja mind a 140
  patch kiolvasását (0,5 s szünettel) → „nyomd meg az EXIT-et” → élő szerkesztés.
- **Böngészős Write** (`71b005b`, BULK LOAD → Write → EXIT → programváltás): működik. A felhasználó szerint
  **a pedál saját WRITE gombja** normál kijelzésnél a szerkesztőből élőben küldött változásokkal együtt menti a patchet
  (BULK nélkül) – a Write ablak ezt gyorsabb útként ajánlja. A szerkesztő erről nem értesül (a patch „szerkesztett” marad).
- **Lánc-sorrend** (`0C 00 0B 00`, 12 bájt, normál módban): a GT-3 az új sorrendet **csak mentés (pedál WRITE vagy
  BULK-os Write) és újratöltés után** szólaltatja meg. A szerkesztő sorrendváltás után figyelmeztet, és a pedál WRITE-ja
  után programváltással újratölti a patchet.
- **BULK LOAD-ba küldés MIDI-ből: nincs rá mód.** A GT-5 MIDI Implementation csak „One way communication”-t (RQ1, DT1)
  ír, handshake-et (WSD/RQD/DAT/ACK) nem, és a címtérképben nincs mód-/parancscím; Bulk Load Ready csak a Utility
  menüből. GT-3-on mérve: Roland RQD (`F0 41 00 00 1B 41 0B 00 03 00 00 00 00 06 6C F7`) normál módban – nincs válasz.
- Nem mért még: a teljes patch temp-be küldése normál módban (Revert / Init / fájlbetöltés: 23 blokk-üzenet) – szól-e.
- Az U35-4 a próbák után a mentésből visszaállítva („PrePH DRIVE”, eredeti hang).
- Nyitott: system terület írása; a `03` és `05` területek jelentése (a `05`: 4 × 13 db 11 bájtos név?);
  a szerkesztő átalakítása (BULK LOAD-os kapcsolódás, „EXIT után szól” jelzés, patchenkénti Read szünettel,
  Write után programváltás).
