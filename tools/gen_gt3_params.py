"""Generate the Web MIDI editor's data files from the Qt FloorBoard's sources.

    python -I tools/gen_gt3_params.py

Reads   midi.xml, default.syx, system.syx  (in gt-3fxfloorboard_source_code_20250420/)
Writes  webmidi/gt3-params.js  - every patch and system parameter with its value table
        webmidi/factory.js     - default.syx / system.syx embedded as base64

midi.xml layout, as MidiTable.cpp reads it:

    <Structure> / <LSB block> / <LSB 00> / <DATA offset> / <PARAM ...>
    <System>    / <LSB area>  / <LSB sub> / <Msb 00>     / <Msb offset> / <PARAM ...>

A <DATA> whose children are <DATA> elements is a two-byte value: byte 1 is the
MSB, byte 2 the LSB (7 bits each), and the child <DATA value=msb> holds the
<PARAM>s for the LSB. The empty <DATA> after it is that LSB byte.
A <PARAM value="range" name="lo/hi/min/max" desc="unit"> maps lo..hi (hex)
linearly onto min..max for display; any other <PARAM> is one discrete value.
A <DATA value="range" name="00/0B/..."> repeats one value table over a span of
offsets (the FX chain slots and the patch name characters).
"""

import base64
import json
import os
import sys
import xml.etree.ElementTree as ET

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROOT = os.path.join(BASE, 'gt-3fxfloorboard_source_code_20250420')  # the Qt FloorBoard source
OUT_DIR = os.path.join(BASE, 'webmidi')

# Blocks 17-19 are the editor's own notes (patch description, creator, scratch pad).
# The GT-3 has no such memory; they live only in .syx files, behind a non-Roland header.
EDITOR_ONLY_FIRST_BLOCK = 0x17

# Typos in midi.xml, fixed here so the XML stays as the FloorBoard ships it.
# (where, value, name) -> corrected value
VALUE_FIXES = {
    # SYN:Octave Shift lists "00=0, 01=-1, 01=-2"; real patches hold 02 there.
    ('block 06 offset 50', '01', '-2'): '02',
}


def fail(msg):
    sys.exit(f'gen_gt3_params: {msg}')


def text(e, attr):
    return (e.get(attr) or '').strip()


def number(s, where):
    try:
        return float(s)
    except ValueError:
        fail(f'{where}: range bound {s!r} is not a number')


def decimals(s):
    # MidiTable::rangeToValue: precision = digits after the '.' of the max bound.
    return len(s.split('.')[-1]) if '.' in s else 0


class Tables:
    """Value tables, deduplicated: most parameters share "Off/On" or "0-100 %"."""

    def __init__(self):
        self.tables = []
        self.index = {}

    def add(self, entries):
        key = json.dumps(entries, sort_keys=True)
        if key not in self.index:
            self.index[key] = len(self.tables)
            self.tables.append(entries)
        return self.index[key]


def entries_of(params, where, base=0):
    """<PARAM> list -> table entries; values are offset by `base` (MSB * 128)."""
    entries, seen = [], set()
    for p in params:
        if p.tag != 'PARAM':
            fail(f'{where}: expected PARAM, got {p.tag}')
        value, name = p.get('value'), p.get('name') or ''
        value = VALUE_FIXES.get((where, value, name.strip()), value)
        if value == 'range':
            parts = name.split('/')
            if len(parts) != 4:
                fail(f'{where}: bad range {name!r}')
            lo, hi = int(parts[0], 16), int(parts[1], 16)
            if hi <= lo:
                fail(f'{where}: empty range {name!r}')
            entries.append({
                'lo': base + lo, 'hi': base + hi,
                'min': number(parts[2], where), 'max': number(parts[3], where),
                'dp': decimals(parts[3]), 'unit': p.get('desc') or '', 'base': base,
            })
        else:
            v = base + int(value, 16)
            # Lookups take the first match (QList::indexOf), so a duplicate value is dead.
            if v in seen:
                print(f'warning: {where}: duplicate value {value} ({name.strip()!r}) ignored', file=sys.stderr)
                continue
            seen.add(v)
            # The MIDI settings keep their value texts in desc (name="" desc="CC#01 ...").
            entries.append([v, name.strip() or (p.get('desc') or '').strip()])
    return entries


def compact(entry):
    if isinstance(entry, list):
        return entry
    e = {k: v for k, v in entry.items() if not (k == 'base' and v == 0) and not (k == 'unit' and v == '')}
    for k in ('min', 'max'):
        if e[k] == int(e[k]):
            e[k] = int(e[k])
    return e


def entry_bounds(entries):
    vals = []
    for e in entries:
        if isinstance(e, list):
            vals.append(e[0])
        else:
            vals += [e['lo'], e['hi']]
    return min(vals), max(vals)


def param_of(data, offset, where, tables):
    """One <DATA> -> parameter dict (without the offset), or None for an empty byte."""
    kids = list(data)
    p = {'name': text(data, 'desc') or text(data, 'name'), 'short': text(data, 'customdesc')}
    if p['name'] == 'MIDI SETTING':
        # Every MIDI setting has desc "MIDI SETTING"; its own name is in customdesc.
        p['name'] = p['short']
    if not kids:
        return None
    if all(k.tag == 'DATA' for k in kids):
        entries = []
        for k in kids:
            msb = int(k.get('value'), 16)
            entries += entries_of(list(k), f'{where} msb {msb:02X}', base=msb * 128)
        p['size'] = 2
    else:
        entries = entries_of(kids, where)
        p['size'] = 1
    p['min'], p['max'] = entry_bounds(entries)
    p['t'] = tables.add([compact(e) for e in entries])
    return p


def params_of(container, where, tables, editor_only=False):
    params, covered = [], set()
    for data in container:
        if data.tag not in ('DATA', 'Msb'):
            fail(f'{where}: unexpected <{data.tag}>')
        value = data.get('value')
        if value == 'range':
            # One table over offsets lo..hi; name like "00/0B/Chain1/Chain12".
            lo, hi = (int(x, 16) for x in text(data, 'name').split('/')[:2])
            label = text(data, 'name').split('/')[2].rstrip('0123456789')
            t = tables.add([compact(e) for e in entries_of(list(data), where)])
            vmin, vmax = entry_bounds(tables.tables[t])
            for off in range(lo, hi + 1):
                params.append({'offset': off, 'name': f'{label}{off - lo + 1}', 'short': f'{label}{off - lo + 1}',
                               'size': 1, 'min': vmin, 'max': vmax, 't': t})
                covered.add(off)
            continue
        offset = int(value, 16)
        p = param_of(data, offset, f'{where} offset {offset:02X}', tables)
        if p is None:
            # The LSB of the two-byte value before it, or an unused byte.
            if offset not in covered and not editor_only:
                prev = next((q for q in params if q['offset'] == offset - 1), None)
                if not prev or prev['size'] != 2:
                    print(f'warning: {where}: empty byte {offset:02X} is not the LSB of a two-byte value',
                          file=sys.stderr)
            continue
        p = {'offset': offset, **p}
        params.append(p)
        covered.update(range(offset, offset + p['size']))
    return params, (max(covered) + 1 if covered else 0)


def structure(xml_root, tables):
    blocks = []
    for blk in xml_root.find('Structure'):
        block_id = int(blk.get('value'), 16)
        subs = list(blk)
        if len(subs) != 1 or subs[0].get('value') != '00':
            fail(f'block {block_id:02X}: expected exactly one <LSB value="00">')
        editor_only = block_id >= EDITOR_ONLY_FIRST_BLOCK
        params, size = params_of(subs[0], f'block {block_id:02X}', tables, editor_only)
        # Size = data bytes in the block's DT1 message: every <DATA>, empty ones included.
        size = max(size, len(subs[0]))
        blocks.append({
            'id': block_id, 'abbr': text(blk, 'abbr') or text(blk, 'name'), 'name': text(blk, 'name'),
            'size': size, **({'editorOnly': True} if editor_only else {}), 'params': params,
        })
    if [b['id'] for b in blocks] != list(range(len(blocks))):
        fail('Structure blocks are not numbered 00, 01, 02, ...')
    return blocks


def system(xml_root, tables):
    areas = []
    for area in xml_root.find('System'):
        for sub in area:
            for msb in sub:
                address = [int(area.get('value'), 16), int(sub.get('value'), 16), int(msb.get('value'), 16)]
                where = 'system ' + ' '.join(f'{a:02X}' for a in address)
                params, size = params_of(list(msb), where, tables)
                areas.append({'address': address, 'name': text(sub, 'name'), 'size': size, 'params': params})
    return areas


def targets(blocks):
    """Assign / EXP / CTL target codes -> the parameter they drive.

    In the Target table each <PARAM> carries the target's block in `desc` and
    its offset in `customdesc` (see customControlTarget.cpp). All target
    parameters must share one list, so it is stored once.
    """
    xml_root = ET.parse(os.path.join(ROOT, 'midi.xml')).getroot()
    lists = {}
    for blk in xml_root.find('Structure'):
        for data in blk[0]:
            if text(data, 'desc') != 'Target':
                continue
            out = []
            for k in data:
                msb = int(k.get('value'), 16)
                for p in k:
                    out.append([msb * 128 + int(p.get('value'), 16), int(p.get('desc'), 16),
                                int(p.get('customdesc'), 16), (p.get('name') or '').strip()])
            lists[(int(blk.get('value'), 16), int(data.get('value'), 16))] = out
    if not lists:
        fail('no Target parameters found')
    first = next(iter(lists.values()))
    for where, lst in lists.items():
        if lst != first:
            fail(f'target list of block {where[0]:02X} differs from the first one')
    by_block = {b['id']: {p['offset'] for p in b['params']} for b in blocks}
    for code, block, offset, name in first:
        if offset not in by_block.get(block, ()):
            print(f'warning: target {code} ({name!r}) points at {block:02X}/{offset:02X}, '
                  'which has no parameter', file=sys.stderr)
    return {'params': sorted(f'{b:02X}/{o:02X}' for b, o in lists), 'list': first}


def syx_blocks(path):
    data = open(path, 'rb').read()
    msgs, start = [], 0
    while start < len(data):
        end = data.index(0xF7, start)
        msgs.append(data[start:end + 1])
        start = end + 1
    return data, msgs


def check_against_default(blocks):
    """The block sizes from midi.xml must match the data lengths in default.syx."""
    _, msgs = syx_blocks(os.path.join(ROOT, 'default.syx'))
    if len(msgs) != len(blocks):
        fail(f'default.syx has {len(msgs)} messages, midi.xml {len(blocks)} blocks')
    for m, b in zip(msgs, blocks):
        if m[8] != b['id'] or len(m) - 12 != b['size']:
            fail(f'block {b["id"]:02X}: midi.xml says {b["size"]} bytes, default.syx has {len(m) - 12}')


def js(value):
    return json.dumps(value, ensure_ascii=False, separators=(',', ':'))


def head_of(item):
    """An item's fields without `params`, as the start of a JS object literal (no closing brace)."""
    return js({k: v for k, v in item.items() if k != 'params'})[:-1]


def write_params(blocks, sys_areas, tgt, tables):
    lines = [
        "'use strict';",
        '',
        '// GENERATED by tools/gen_gt3_params.py from midi.xml - do not edit by hand.',
        '//',
        '// blocks[id].params[]: {offset, name, short, size (1|2 bytes), min, max, t}',
        '// system[]:            {address: [a, b, c], name, size, params[]} - DT1 address is [...address, offset]',
        '// tables[t]:           value table - [value, label] or {lo, hi, min, max, dp, unit, base}',
        '// targets.list[]:      [code, block, offset, name] - what an Assign / EXP / CTL target drives',
        '',
        'const GT3_PARAMS = {',
        '  blocks: [',
    ]
    for b in blocks:
        lines.append(f'    {head_of(b)}, "params": [')
        lines += [f'      {js(p)},' for p in b['params']]
        lines.append('    ] },')
    lines += ['  ],', '  system: [']
    for a in sys_areas:
        lines.append(f'    {head_of(a)}, "params": [')
        lines += [f'      {js(p)},' for p in a['params']]
        lines.append('    ] },')
    lines += ['  ],', f'  targets: {{ params: {js(tgt["params"])}, list: [']
    lines += [f'    {js(t)},' for t in tgt['list']]
    lines += ['  ] },', '  tables: [']
    lines += [f'    {js(t)},' for t in tables.tables]
    lines += ['  ],', '};', '']
    with open(os.path.join(OUT_DIR, 'gt3-params.js'), 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(lines))


def write_factory():
    def b64_lines(name):
        data, _ = syx_blocks(os.path.join(ROOT, name))
        b64 = base64.b64encode(data).decode()
        return ' +\n'.join(f"  '{b64[i:i + 100]}'" for i in range(0, len(b64), 100)) + ';'

    out = [
        "'use strict';",
        '',
        '// GENERATED by tools/gen_gt3_params.py - embedded so the editor also works opened',
        '// straight from disk, where fetch() of local files is blocked.',
        '',
        '// default.syx: the init patch (23 GT-3 blocks + 3 editor-only blocks, 878 bytes).',
        'const DEFAULT_PATCH_SYX_BASE64 =',
        b64_lines('default.syx'),
        '',
        '// system.syx: a full GT-3 system-area dump.',
        'const SYSTEM_SYX_BASE64 =',
        b64_lines('system.syx'),
        '',
    ]
    with open(os.path.join(OUT_DIR, 'factory.js'), 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(out))


def main():
    xml_root = ET.parse(os.path.join(ROOT, 'midi.xml')).getroot()
    tables = Tables()
    blocks = structure(xml_root, tables)
    check_against_default(blocks)
    sys_areas = system(xml_root, tables)
    tgt = targets(blocks)
    os.makedirs(OUT_DIR, exist_ok=True)
    write_params(blocks, sys_areas, tgt, tables)
    write_factory()
    n = sum(len(b['params']) for b in blocks)
    print(f'{len(blocks)} blocks, {n} patch parameters, {sum(len(a["params"]) for a in sys_areas)} system '
          f'parameters, {len(tgt["list"])} targets, {len(tables.tables)} value tables')


if __name__ == '__main__':
    main()
