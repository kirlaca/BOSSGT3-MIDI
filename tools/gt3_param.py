"""Send one parameter to the GT-3's temporary patch (DT1, device ID 00).

    python gt3_param.py <area 0C|0B> <block hex> <offset hex> <value>
    python gt3_param.py 0C 0A 00 30     (Master Level = 30 through 0C 00, the FloorBoard's live-edit area)

Values are decimal; several bytes of one parameter as hex, comma-separated: 01,0A.
Nothing here writes to the patch memory.
"""
import sys
import time

import gt3_tester as t


def dt1(body):
    return bytes([0xF0, 0x41, 0x00, 0x00, 0x1B, 0x12, *body, t.checksum(body), 0xF7])


area, block, offset = (int(x, 16) for x in sys.argv[1:4])
values = [int(v, 16) for v in sys.argv[4].split(',')] if ',' in sys.argv[4] else [int(sys.argv[4])]
msg = dt1([area, 0x00, block, offset, *values])
out = t.MidiOut(t.pick(t.output_names(), 'DOREMiDi', 'output'))
try:
    out.send(msg)
    print(time.strftime('%H:%M:%S'), '->', t.hexs(msg))
finally:
    out.close()
