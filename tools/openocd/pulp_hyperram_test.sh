#!/bin/bash
# Test the PULP HyperRAM over JTAG.
#
#   pulp_hyperram_test.sh <openocd board cfg> [size] [l3_addr]
#       Data test: write random data to the CSN1 chip, read it back, compare.
#       size defaults to 4096 bytes and l3_addr to 0.
#
#   pulp_hyperram_test.sh <openocd board cfg> addr [chip_size]
#       Addressability test: address-line test of the CSN1 chip over
#       chip_size bytes (default 0x4000000, 512 Mbit) and a read probe of the
#       CSN0 chip, which this udma_hyper can only reach in HyperFlash mode.
#
# Set T_LATENCY to override the
# controller's HyperRAM initial latency (default 7, see pulp_hyperram.tcl).
# Keep size <= 64 KB (one chunk) so the readback does not depend on Jim file
# I/O in the OpenOCD build.

set -e

if [ $# -lt 1 ]; then
    echo "usage: $0 <openocd board cfg> [size] [l3_addr]" >&2
    echo "       $0 <openocd board cfg> addr [chip_size]" >&2
    exit 1
fi

CFG=$1
OPENOCD=${OPENOCD_BIN:-openocd}
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

run_openocd() {
    "$OPENOCD" -f "$CFG" \
        ${T_LATENCY:+-c "set PULP_HYPER_T_LATENCY $T_LATENCY"} \
        -f "$SCRIPT_DIR/pulp_hyperram.tcl" \
        "$@" \
        -c "shutdown"
}

if [ "$2" = addr ]; then
    if run_openocd -c "pulp_hyper_check_chips ${3:-0x4000000}"; then
        echo "PASS: addressability"
    else
        echo "FAIL: addressability (see pulp_hyper messages above)"
        exit 1
    fi
    exit 0
fi

SIZE=${2:-4096}
L3_ADDR=${3:-0x0}

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

head -c "$SIZE" /dev/urandom > "$WORK/src.bin"

run_openocd \
    -c "pulp_hyperram_load $WORK/src.bin $L3_ADDR" \
    -c "pulp_hyperram_dump $WORK/dst.bin $L3_ADDR $SIZE"

if cmp -s "$WORK/src.bin" "$WORK/dst.bin"; then
    echo "PASS: $SIZE bytes at L3 $L3_ADDR read back identical"
else
    echo "FAIL: readback differs (byte offset, written, read; octal):"
    cmp -l "$WORK/src.bin" "$WORK/dst.bin" | head -20
    if cmp -l "$WORK/src.bin" "$WORK/dst.bin" | awk '$3 != 245 {x=1} END {exit x}'; then
        echo "All bad bytes are 0xa5: the read never reached L2 (no HyperRAM response?)"
    fi
    python3 -c '
import sys
src, dst = (open(p, "rb").read() for p in sys.argv[1:3])
for k in range(-32, 33):
    a, b = (src[k:], dst) if k >= 0 else (src, dst[-k:])
    n = min(len(a), len(b)) - 32
    if n > 0 and a[:n] == b[:n]:
        print("Readback is the written data shifted: read[i] == written[i%+d]" % k)
        if k > 0 and k % 4 == 0:
            print("Write latency too short by %d clock(s): retry with T_LATENCY=%d"
                  % (k // 4, int(sys.argv[3]) + k // 4))
        break
' "$WORK/src.bin" "$WORK/dst.bin" "${T_LATENCY:-7}" || true
    exit 1
fi
