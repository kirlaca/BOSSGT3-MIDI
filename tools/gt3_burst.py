"""Send a run of parameter values to the GT-3's temporary patch (0C 00), like a knob being turned.

    python gt3_burst.py <block hex> <offset hex> <from> <to> <steps> <gap ms>
    python gt3_burst.py 03 02 40 100 13 25     (OD Drive 40 -> 100 in 13 steps, 25 ms apart)

Values are decimal. Nothing here writes to the patch memory.
"""
import sys
import time

import gt3_tester as t


def dt1(body):
    return bytes([0xF0, 0x41, 0x00, 0x00, 0x1B, 0x12, *body, t.checksum(body), 0xF7])


block, offset = (int(x, 16) for x in sys.argv[1:3])
start, end, steps, gap = (int(x) for x in sys.argv[3:7])
values = [round(start + (end - start) * i / (steps - 1)) for i in range(steps)]
out = t.MidiOut(t.pick(t.output_names(), 'DOREMiDi', 'output'))
try:
    for v in values:
        m = dt1([0x0C, 0x00, block, offset, v])
        out.send(m)
        print(time.strftime('%H:%M:%S'), '->', t.hexs(m))
        time.sleep(gap / 1000)
finally:
    out.close()
