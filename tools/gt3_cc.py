"""Send MIDI CC to the GT-3 on channel 1, like a knob being turned (works in normal mode).

    python gt3_cc.py <cc> <value>                    (one value)
    python gt3_cc.py <cc> <from> <to> <steps> <ms>   (a sweep: 0 127 20 100 = 2 s up)
"""
import sys
import time

import gt3_tester as t

cc = int(sys.argv[1])
if len(sys.argv) == 3:
    values, gap = [int(sys.argv[2])], 0
else:
    a, b, steps, gap = (int(x) for x in sys.argv[2:6])
    values = [round(a + (b - a) * i / (steps - 1)) for i in range(steps)]
out = t.MidiOut(t.pick(t.output_names(), 'DOREMiDi', 'output'))
try:
    for v in values:
        out.send(bytes([0xB0, cc, v]))
        print(time.strftime('%H:%M:%S'), f'-> CC#{cc} = {v}', flush=True)
        time.sleep(gap / 1000)
finally:
    out.close()
