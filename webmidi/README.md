# GT-3 Editor – Web MIDI változat

A Qt-s GT-3 FxFloorBoard böngészős átirata, **backend nélkül**: a böngésző a
[Web MIDI API](https://developer.mozilla.org/docs/Web/API/Web_MIDI_API)-n keresztül közvetlenül
beszél a MIDI interfésszel. Felépítés és design: a ME-5 editor (`D:\BOSSME5\webmidi\`).
A teljes terv: [`../WEBMIDI_FELDERITES.md`](../WEBMIDI_FELDERITES.md).

## Állapot

- [x] 1. `midi.xml` → `gt3-params.js` generátor
- [x] 2. Protokoll réteg (`gt3.js`) + Web MIDI réteg (`midi.js`)
- [x] 3. Próba a pedállal (2026-11-01, lásd `../WEBMIDI_FELDERITES.md`): csatlakozás, kiolvasás, élő szerkesztés, Write
- [x] 4–5. Effektek oldal (a pedállal kipróbálva)
- [x] 6. EXP / CTL / Assign oldal, FX Chain átrendezés (a lánc kipróbálva; az Assign-oldal változtatásai a pedálon még nem)
- [x] 7. Library és System oldal (a System oldal írása a pedálon még nem kipróbált)
- [x] 8. Útmutatók: `utmutato.html` (magyar), `guide.html` (angol) – a mért működéssel frissítve

## Fájlok

| Fájl | Mi ez | Qt megfelelő |
|---|---|---|
| `gt3-params.js` | **generált** – minden patch- és system-paraméter, értéktáblák, assign célok | `midi.xml` + `MidiTable.cpp` |
| `factory.js` | **generált** – `default.syx` (init patch) és `system.syx` base64-ben | `:default.syx`, `:system.syx` |
| `gt3.js` | SysEx üzenetek, patch modell, értékek és feliratok, fájl be/ki | `SysxIO.cpp`, `MidiTable.cpp`, `sysxWriter.cpp` |
| `midi.js` | Web MIDI: portok, GT-3 felismerés, patch olvasás/írás, programváltás | `midiIO.cpp` (RtMidi) |
| `effects.js` | effekt blokkonként: pedál szín, be/ki és típus paraméter, típusonként a paramétercsoportok | `stompbox_*.cpp` szerkesztőoldalai |
| `assign.js` | az EXP / CTL / Assign oldal: vezérlőlista, Target → Min/Max, Source, válaszgörbe, Internal / Wave pedal | `menuPage_assign.cpp`, `customControlTarget.cpp` |
| `library.js` | Library oldal: mind a 340 patch listában, kijelölés, olvasás / írás / másolás / init / mentés | `bankTreeList.cpp`, `bulkLoadDialog.cpp`, `bulkSaveDialog.cpp` |
| `system.js` | System oldal: tuner, kimenet, globális EQ, vezérlők, patchváltás, MIDI, manual mód | `menuPage_system.cpp` |
| `app.js` | a felület: kijelző, patch térkép, FX Chain, pedálok, részletes panel, Write / fájlok / kapcsolat | `floorBoard.cpp`, `editWindow.cpp` … |
| `utmutato.html`, `guide.html`, `pages.css` | használati útmutató magyarul és angolul (az editor fejlécéből is elérhető) | `GT-3EditorManual_E.htm` |
| `ask.js`, `style.css`, `index.html` | párbeszédablakok, stílus (a ME-5 editoréból, GT-3 színekkel) | |

A generált fájlokat kézzel nem szerkesztjük; újragenerálás (a projekt gyökeréből):

```
python -I tools/gen_gt3_params.py
```

## Indítás

Nyisd meg az `index.html`-t Chrome-ban vagy Edge-ben. Ha `file://`-ról nem kér MIDI engedélyt:
`python -m http.server -d webmidi`, és a kiírt `http://localhost:...` cím.

## Az effektek oldal

- **Kijelző**: piros LED a patch számmal (`U01-1`, `P36-1`), zöld LCD a névvel (kattintásra átnevezhető,
  11 karakter) és a PreAmp / OD típussal. ◀ ▶ vagy a billentyűzet nyilai lapoznak a 340 patch között.
- **Patch térkép**: User / Preset, 8 bank oldalanként. Amit még nem olvastunk ki, `- - -`.
  A még ismeretlen patcheket a Read (BULK) olvassa ki (a GT-3 csak BULK LOAD-ban válaszol).
- **FX Chain**: a patch saját effekt-sorrendje; húzással (vagy Alt + ← / →) átrendezhető, a pedálok is ebben
  a sorrendben állnak.
- **Pedálok**: be/ki lábkapcsoló, típusválasztó (SFX, OD/DS, PRE, MOD, RV: ◀ név ▶ LCD-ablak; CS, WAH, DD: kapcsoló),
  a fő knobok. A névtáblára kattintva alul megnyílik a **részletes panel** az adott típus összes paraméterével.
- **Write**: párbeszédablak, melyik user patchbe (presetbe nem lehet írni); vezetett BULK LOAD → küldés → EXIT,
  utána programváltás az írt patchre. Auto write nincs (minden íráshoz BULK LOAD kell). Gyors út: a pedál WRITE gombja.
  **Init patch**: a `default.syx` betöltése. **Load file**: egy patch → a szerkesztőbe, bulk → a user patchekbe.
  **Save file**: az aktuális patch, vagy mind a 140 user patch egy fájlban.
- Élő szerkesztés: paraméterenként egy DT1 a temp patchbe (`0C 00 blokk offset`), a gyors knob-tekerést
  25 ms-onként összevonva küldi; normál módban azonnal szól. A lánc-sorrend csak mentés + újratöltés után szól.
- Kapcsolódás (jobb felül): csak portválasztás, nem küld semmit. A szerkesztő követi a pedál programváltásait
  (`Midi.watchPatchChanges`). **Read (BULK)**: az aktuális patch (temp + tárolt) vagy mind a 140, 0,5 s szünettel.

## Az EXP / CTL / Assign oldal

A fejléc alatti fülről vagy a `#assign` címmel érhető el (újratöltés után is ott marad).

- Bal oldalt a patch 10 vezérlője: EXP pedál (`0D`), CTL pedál (`0E`), Assign 1–8 (`0F`–`16`), mindegyiknél
  a be/ki állapot, a cél és a forrás.
- **Target**: a 207 cél blokkonként csoportosítva. A **Min / Max** knob a cél paraméter saját tartományában
  és mértékegységében mutat (pl. DD Delay Time: ms és `1/4*bpm`); célváltáskor a Qt-hez hasonlóan Min = a cél
  minimuma, Max = a maximuma. Ismeretlen célkód (régi hibás fájl) nyers számként jelenik meg.
- **Source** (csak Assign): forrás (EXP, CTL, SUB EXP / CTL, Internal, Wave, CC#…), Normal / Toggle mód
  (CTL-nél is), aktív tartomány alsó/felső határa.
- **Response**: a forrás 0–127 állása → célérték görbe (Toggle-nál lépcső), élőben követi a beállításokat.
- **Internal pedal** (trigger, idő, görbe) és **Wave pedal** (hullámforma, sebesség): mindig szerkeszthetők,
  de halványak, ha a forrás nem az.

## A Library oldal (`#library`)

- User / Preset / All, keresés névre. Oszlopok: patch, név, PreAmp, OD/DS, SFX · MOD típus (ha be van kapcsolva),
  a bekapcsolt effektek színes pöttyökkel (lánc sorrendben), állapot: *Not read*, *Editor only* (a szerkesztőben
  változott, a GT-3-ra még nem került), *Editing*. A patch számára kattintva megnyílik az Effektek oldalon.
- A kijelölt patcheken: **Read from GT-3**, **Write to GT-3** (csak user, megerősítéssel, folyamatjelzővel),
  **Copy to…** (egymás utáni user helyekre, csak a szerkesztőben), **Init**, **Save selected…** (egy fájlba).
- Az *Editor only* patchek a fejléc patch-térképén szaggatott sárga kerettel látszanak.

## A System oldal (`#system`)

- A Qt oldal csoportjai: Output, Global EQ / NS / Reverb, Tuner, Controls, Patch change, MIDI, Manual mode.
- Nem patch-adat: a GT-3 minden változást azonnal eltárol, nincs Write. Kapcsolódva az oldal megnyitásakor
  egyszer kiolvassa a GT-3 beállításait; offline a `system.syx` alapértékei látszanak.
- **Send all to GT-3**, **Load file…** / **Save file…** (a teljes rendszerdump, a midi.xml-ben le nem írt részekkel,
  pl. a programváltás-táblával együtt, bájtra azonosan). A fejléc Load file… gombja a system fájlt is felismeri.

## Tesztek (pedál nélkül)

```
node tools/test_gt3.mjs    # protokoll: a Qt képletei, default.syx, system.syx, 325 valódi patch
node tools/test_midi.mjs   # midi.js egy szimulált GT-3 ellen (felismerés, olvasás, írás)
```

## Protokoll röviden

- Fejléc `F0 41 <dev> 00 1B <cmd>`, 4 bájtos cím, Roland checksum (cím + adat).
- Patch = blokkonként egy DT1 üzenet, cím `<terület> <terület> <blokk> 00`, 23 üzenet / 650 bájt.
  - `0C 00` – temp patch (amit a pedál éppen szól); élő szerkesztés: `0C 00 <blokk> <offset> <érték>`
  - `0B 00` – temp patch visszaolvasása (RQ1)
  - `07 nn` / `08 nn` – user patchek U01-1 … U35-4; a presetek a következő 128-as lapon kezdődnek
- Kétbájtos értékek (pl. Master BPM, assign Target/Min/Max): MSB, LSB, 7-7 bit.
- A 17–19. blokk (patch leírás, szerző, jegyzet) csak a FloorBoard fájljaiban létezik,
  `F0 00 …` fejléccel; a pedálra soha nem megy ki.
- Device ID 00–1F (a pedálon látható 1 = `00`). A GT-3 **RQ1-re csak BULK LOAD-ban válaszol**, és a kért mérettől
  függetlenül mindig az egész patchet küldi; a DT1-et normál módban élőben, BULK LOAD-ban EXIT után veszi át,
  a memóriába (`07`/`08 nn`) írást EXIT-kor tárolja. (`Midi.detectGt3` megmaradt, a felület már nem használja.)
- Patchváltás: `B0 00 <n/100>`, `B0 20 00`, `C0 <n%100>`.
