"""Write one user patch into the GT-3's memory (the GT-3 must be in BULK LOAD mode).

    python gt3_write.py <patch 0-139> <source .syx> [--name "NEW NAME"] [--gap ms]
    python gt3_write.py 139 ../gt3-backup-2026-11-01.syx --name "WRITE TEST"

The patch's 23 blocks are taken from the source file (a backup with the same patch number,
or any file holding exactly one patch), readdressed to the target slot (07 nn / 08 nn) and
sent one block per message. --name replaces the 11-character name (block 0C).
"""
import argparse
import time

import gt3_tester as t


def messages(path):
    data = open(path, 'rb').read()
    out, i = [], 0
    while i < len(data):
        j = data.index(0xF7, i)
        out.append(bytes(data[i:j + 1]))
        i = j + 1
    return out


def dt1(address, values):
    body = [*address, *values]
    return bytes([0xF0, 0x41, 0x00, 0x00, 0x1B, 0x12, *body, t.checksum(body), 0xF7])


parser = argparse.ArgumentParser()
parser.add_argument('patch', type=int)
parser.add_argument('source')
parser.add_argument('--name')
parser.add_argument('--gap', type=float, default=20, help='ms between messages (default 20)')
args = parser.parse_args()

addr = t.patch_address(args.patch)
patch_msgs = [m for m in messages(args.source) if m[6] >= 0x07 and m[8] <= 0x16]
own = [m for m in patch_msgs if list(m[6:8]) == addr]
blocks = {m[8]: m[10:-2] for m in (own or patch_msgs)}
if len(blocks) != 23 or (not own and len(patch_msgs) != 23):
    raise SystemExit(f'{args.source}: no complete patch {args.patch} (or a single patch) in it')
if args.name is not None:
    blocks[0x0C] = args.name.encode('ascii')[:11].ljust(11)

out = t.MidiOut(t.pick(t.output_names(), 'DOREMiDi', 'output'))
try:
    for b in sorted(blocks):
        m = dt1([*addr, b, 0x00], blocks[b])
        out.send(m)
        print(time.strftime('%H:%M:%S'), '->', t.hexs(m[:12]), '…' if len(m) > 12 else '', f'({len(m)} bytes)')
        time.sleep(args.gap / 1000)
finally:
    out.close()
print(f'sent U{args.patch // 4 + 1:02}-{args.patch % 4 + 1} ({" ".join(f"{x:02X}" for x in addr)}), '
      f'name "{bytes(blocks[0x0C]).decode("ascii", "replace")}"')
