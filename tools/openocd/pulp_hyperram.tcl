# OpenOCD helpers to read/write the PULP HyperRAM (L3) from the host, with
# no firmware running on the chip.
#
# The host drives the udma_hyper registers directly through the debug module
# (system bus access): data is staged in an L2 buffer with load_image, then a
# uDMA transfer moves it between L2 and L3. This is the same register sequence
# as pi_hyper_write_async / pi_hyper_read_async in
# rtos/pulpos/pulp/drivers/hyperbus/hyperbus-v3.c.
#
# Works for both pulp_fpga ports (ZCU102 and VCU118): they share the same
# pulp_soc, udma_hyper on the FMC HyperBus pins and riscv-dbg JTAG.
#
# Usage, after any OpenOCD config that ends with "init" (the core is halted here):
#
#   openocd -f openocd-zcu102-digilent-jtag-hs2.cfg \
#           -f $PULP_SDK_HOME/tools/openocd/pulp_hyperram.tcl \
#           -c "pulp_hyperram_load data.bin 0x0; shutdown"
#
#   pulp_hyperram_load <file> <l3_addr> [verify]   write file to L3 (verify=1 reads it back)
#   pulp_hyperram_dump <file> <l3_addr> <size>     read L3 into a host file
#   pulp_hyperram_addr_test [chip_size]            address-line test of the CSN1 chip
#   pulp_hyper_probe_csn0                          check that the CSN0 chip answers reads
#
# Chip select is not programmable in this udma_hyper: it follows MEM_SEL
# (udma_hyper_ctrl.sv). MEM_SEL=0 (HyperRAM) drives CSN1, MEM_SEL=1
# (HyperFlash) drives CSN0. In HyperFlash mode writes carry no data phase and
# no latency, so a HyperRAM on CSN0 can be read but not written.
#
# Register map for pulp-open (pulp_archi chips/pulp, udma v3, hyper v3).
# Override any of these with "set" before sourcing this file.

if {![info exists PULP_UDMA_ADDR]}     { set PULP_UDMA_ADDR     0x1A102000 }
if {![info exists PULP_HYPER_PERIPH]}  { set PULP_HYPER_PERIPH  8 }
# 0: HyperRAM on CSN1. 1: HyperFlash mode, CSN0 (read only for a HyperRAM).
if {![info exists PULP_HYPER_MEM_SEL]} { set PULP_HYPER_MEM_SEL 0 }
# Initial latency in clocks, must match the HyperRAM CR0 setting. The FPGA FMC
# HyperRAM needs 7 (verified on board); the SDK and the S27KS0641 sim model
# use 6. A mismatch only breaks writes (reads are RWDS-strobed): data lands
# shifted by 4 bytes per clock with the fixed 2x latency.
if {![info exists PULP_HYPER_T_LATENCY]} { set PULP_HYPER_T_LATENCY 7 }
# L2 staging buffer: interleaved L2, clear of the private banks where code lives.
if {![info exists PULP_HYPER_L2_BUF]}  { set PULP_HYPER_L2_BUF  0x1C010000 }
if {![info exists PULP_HYPER_CHUNK]}   { set PULP_HYPER_CHUNK   0x10000 }
if {![info exists PULP_HYPER_TIMEOUT]} { set PULP_HYPER_TIMEOUT 1000 }

# udma_v3.h: UDMA_PERIPH_OFFSET(id) = (id << 7) + 0x80, CG register at offset 0
set PULP_UDMA_CG    $PULP_UDMA_ADDR
set PULP_HYPER_BASE [expr {$PULP_UDMA_ADDR + ($PULP_HYPER_PERIPH << 7) + 0x80}]

set PULP_HYPER_NB_CHANNELS 8
set PULP_UDMA_CFG_EN       0x10
set PULP_HYPER_CA_WRITE    0x1
set PULP_HYPER_CA_READ     0x5

# Common registers (HYPER_COMMON_REGS_OFFSET = 8 * 0x80)
proc pulp_hyper_common {off} {
    global PULP_HYPER_BASE
    return [expr {$PULP_HYPER_BASE + 0x400 + $off}]
}

# Per transaction-id registers (HYPER_CHANNELS_OFFSET = 0x80)
proc pulp_hyper_chan {id off} {
    global PULP_HYPER_BASE
    return [expr {$PULP_HYPER_BASE + $id * 0x80 + $off}]
}

proc pulp_hyper_rd32 {addr} {
    if {[llength [info commands read_memory]]} {
        return [lindex [read_memory $addr 32 1] 0]
    }
    mem2array val 32 $addr 1
    return $val(0)
}

proc pulp_hyper_wr32 {addr val} {
    mww [format 0x%08x $addr] [format 0x%08x $val]
}

proc pulp_hyper_init {} {
    global PULP_UDMA_CG PULP_HYPER_PERIPH PULP_HYPER_MEM_SEL PULP_HYPER_T_LATENCY

    # Keep the FC from touching L2 or the uDMA while we drive it. Also needed
    # for memory accesses when the debug module has no system bus access.
    halt

    pulp_hyper_wr32 $PULP_UDMA_CG \
        [expr {[pulp_hyper_rd32 $PULP_UDMA_CG] | (1 << $PULP_HYPER_PERIPH)}]

    # pos_hyper_setup() for PI_HYPER_TYPE_RAM
    pulp_hyper_wr32 [pulp_hyper_common 0x08] 1            ;# REG_EN_LATENCY_ADD
    pulp_hyper_wr32 [pulp_hyper_common 0x20] $PULP_HYPER_MEM_SEL ;# MEM_SEL
    pulp_hyper_wr32 [pulp_hyper_common 0x0C] 0xffffffff   ;# REG_T_CS_MAX
    pulp_hyper_wr32 [pulp_hyper_common 0x04] $PULP_HYPER_T_LATENCY ;# REG_T_LATENCY_ACCESS
    pulp_hyper_wr32 [pulp_hyper_common 0x00] 0x03         ;# REG_PAGE_BOUND (1 KB)
}

proc pulp_hyper_alloc_id {} {
    global PULP_HYPER_NB_CHANNELS PULP_HYPER_TIMEOUT
    for {set i 0} {$i < $PULP_HYPER_TIMEOUT} {incr i} {
        set id [pulp_hyper_rd32 [pulp_hyper_common 0x24]]   ;# TRANS_ID_ALLOC
        if {$id < $PULP_HYPER_NB_CHANNELS} { return $id }
        sleep 1
    }
    error "pulp_hyper: no free transaction id"
}

proc pulp_hyper_wait {id cfg_reg} {
    global PULP_UDMA_CFG_EN PULP_HYPER_TIMEOUT
    for {set i 0} {$i < $PULP_HYPER_TIMEOUT} {incr i} {
        set pending [expr {[pulp_hyper_rd32 $cfg_reg] & $PULP_UDMA_CFG_EN}]
        set busy    [expr {[pulp_hyper_rd32 [pulp_hyper_chan $id 0x24]] & 1}]   ;# STATUS
        if {!$pending && !$busy} { return }
        sleep 1
    }
    error "pulp_hyper: transfer timeout (id $id)"
}

# One uDMA transfer between L2 and L3, blocking until it completes.
proc pulp_hyper_xfer {is_write l3_addr l2_addr size} {
    global PULP_HYPER_CA_WRITE PULP_HYPER_CA_READ PULP_UDMA_CFG_EN

    set id [pulp_hyper_alloc_id]

    for {set off 0x28} {$off <= 0x3C} {incr off 4} {
        pulp_hyper_wr32 [pulp_hyper_chan $id $off] 0          ;# TWD_*: linear transfer
    }
    pulp_hyper_wr32 [pulp_hyper_chan $id 0x18] \
        [expr {$is_write ? $PULP_HYPER_CA_WRITE : $PULP_HYPER_CA_READ}]   ;# HYPER_CA_SETUP
    pulp_hyper_wr32 [pulp_hyper_chan $id 0x1C] $l3_addr       ;# REG_HYPER_ADDR

    # Fill the buffer before a read, so a transfer that never lands can't pass
    # for the data left over from the previous write.
    if {!$is_write} {
        mww [format 0x%08x $l2_addr] 0xa5a5a5a5 [expr {($size + 3) / 4}]
    }

    # UDMA_HYPER_CHANNEL_TX = 0x0C, UDMA_HYPER_CHANNEL_RX = 0x00
    set chan [pulp_hyper_chan $id [expr {$is_write ? 0x0C : 0x00}]]
    pulp_hyper_wr32 [expr {$chan + 0x0}] $l2_addr             ;# SADDR
    pulp_hyper_wr32 [expr {$chan + 0x4}] $size                ;# SIZE
    pulp_hyper_wr32 [expr {$chan + 0x8}] $PULP_UDMA_CFG_EN    ;# CFG: EN | SIZE_8

    pulp_hyper_wait $id [expr {$chan + 0x8}]
}

# Jim's expr has no min()
proc pulp_hyper_min {a b} {
    return [expr {$a < $b ? $a : $b}]
}

proc pulp_hyper_read_file {path offset size} {
    set f [open $path rb]
    $f seek $offset
    set data [$f read $size]
    $f close
    return $data
}

proc pulp_hyper_tmpfile {} {
    return [file join [expr {[info exists ::env(TMPDIR)] ? $::env(TMPDIR) : "/tmp"}] \
        "pulp_hyper_chunk.bin"]
}

proc pulp_hyperram_load {path l3_addr {verify 0}} {
    global PULP_HYPER_L2_BUF PULP_HYPER_CHUNK

    set total [file size $path]
    set tmp [pulp_hyper_tmpfile]
    pulp_hyper_init

    echo [format "pulp_hyper: writing %s (%d bytes) to L3 0x%08x" $path $total $l3_addr]

    for {set off 0} {$off < $total} {incr off $n} {
        set n [pulp_hyper_min $PULP_HYPER_CHUNK [expr {$total - $off}]]

        # Shift the image base so that bytes [off, off+n) land at the L2 buffer,
        # and clip everything else away with min_address/max_length.
        load_image $path [expr {$PULP_HYPER_L2_BUF - $off}] bin $PULP_HYPER_L2_BUF $n
        pulp_hyper_xfer 1 [expr {$l3_addr + $off}] $PULP_HYPER_L2_BUF $n

        if {$verify} {
            pulp_hyper_xfer 0 [expr {$l3_addr + $off}] $PULP_HYPER_L2_BUF $n
            dump_image $tmp $PULP_HYPER_L2_BUF $n
            set got [pulp_hyper_read_file $tmp 0 $n]
            file delete $tmp
            if {$got ne [pulp_hyper_read_file $path $off $n]} {
                error [format "pulp_hyper: verify failed in chunk at L3 0x%08x" \
                    [expr {$l3_addr + $off}]]
            }
        }
    }

    echo [format "pulp_hyper: done%s" [expr {$verify ? ", verified" : ""}]]
}

proc pulp_hyperram_dump {path l3_addr size} {
    global PULP_HYPER_L2_BUF PULP_HYPER_CHUNK

    pulp_hyper_init

    # A single chunk goes straight to the file, without Jim file I/O.
    if {$size <= $PULP_HYPER_CHUNK} {
        pulp_hyper_xfer 0 $l3_addr $PULP_HYPER_L2_BUF $size
        dump_image $path $PULP_HYPER_L2_BUF $size
        echo [format "pulp_hyper: dumped %d bytes from L3 0x%08x to %s" $size $l3_addr $path]
        return
    }

    set tmp [pulp_hyper_tmpfile]
    set out [open $path wb]

    for {set off 0} {$off < $size} {incr off $n} {
        set n [pulp_hyper_min $PULP_HYPER_CHUNK [expr {$size - $off}]]
        pulp_hyper_xfer 0 [expr {$l3_addr + $off}] $PULP_HYPER_L2_BUF $n
        dump_image $tmp $PULP_HYPER_L2_BUF $n
        $out puts -nonewline [pulp_hyper_read_file $tmp 0 $n]
    }

    $out close
    file delete $tmp
    echo [format "pulp_hyper: dumped %d bytes from L3 0x%08x to %s" $size $l3_addr $path]
}

proc pulp_hyper_rd32_list {addr n} {
    if {[llength [info commands read_memory]]} {
        return [read_memory $addr 32 $n]
    }
    mem2array val 32 $addr $n
    set out {}
    for {set i 0} {$i < $n} {incr i} { lappend out $val($i) }
    return $out
}

proc pulp_hyper_log2 {x} {
    set n 0
    while {(1 << ($n + 1)) <= $x} { incr n }
    return $n
}

# Address-line test of the CSN1 HyperRAM. A unique 32-bit marker goes to
# offset 0 and to every power of two from 4 up to chip_size/2, then all are
# read back. A stuck or shorted address line makes two offsets hit the same
# cell, so the later marker overwrites the earlier one; a chip smaller than
# chip_size wraps its top offsets back onto low ones. Address bits below 2
# are covered by the data test (pulp_hyperram_test.sh).
proc pulp_hyperram_addr_test {{chip_size 0x4000000}} {
    global PULP_HYPER_L2_BUF PULP_HYPER_MEM_SEL

    set PULP_HYPER_MEM_SEL 0
    pulp_hyper_init

    set offsets [list 0]
    for {set b 2} {(1 << $b) < $chip_size} {incr b} { lappend offsets [expr {1 << $b}] }
    set n [llength $offsets]
    set wbuf $PULP_HYPER_L2_BUF
    set rbuf [expr {$PULP_HYPER_L2_BUF + 4 * $n}]

    echo [format "pulp_hyper: address test, CSN1, %d MB, %d offsets" \
        [expr {$chip_size >> 20}] $n]

    for {set i 0} {$i < $n} {incr i} {
        pulp_hyper_wr32 [expr {$wbuf + 4 * $i}] [expr {0xC0DE0000 + $i}]
    }
    for {set i 0} {$i < $n} {incr i} {
        pulp_hyper_xfer 1 [lindex $offsets $i] [expr {$wbuf + 4 * $i}] 4
    }
    for {set i 0} {$i < $n} {incr i} {
        pulp_hyper_xfer 0 [lindex $offsets $i] [expr {$rbuf + 4 * $i}] 4
    }

    set errors 0
    set i 0
    foreach got [pulp_hyper_rd32_list $rbuf $n] {
        set off [lindex $offsets $i]
        set exp [expr {0xC0DE0000 + $i}]
        if {$got != $exp} {
            incr errors
            set j [expr {$got - 0xC0DE0000}]
            if {$j >= 0 && $j < $n} {
                set other [lindex $offsets $j]
                echo [format "  0x%08x: overwritten by the write to 0x%08x (%s)" \
                    $off $other [pulp_hyper_alias_reason $off $other]]
            } elseif {$got == 0xa5a5a5a5} {
                echo [format "  0x%08x: no data returned" $off]
            } else {
                echo [format "  0x%08x: read 0x%08x, expected 0x%08x" $off $got $exp]
            }
        }
        incr i
    }

    if {$errors} {
        error [format "pulp_hyper: address test FAILED (%d of %d offsets)" $errors $n]
    }
    echo [format "pulp_hyper: address test passed, address bits 2..%d independent" \
        [pulp_hyper_log2 [lindex $offsets end]]]
}

# What it means that two single-bit offsets reached the same cell. The test
# sees which bit has no independent effect, not why (stuck, open or shorted).
proc pulp_hyper_alias_reason {a b} {
    if {$a == 0 || $b == 0} {
        set bit [pulp_hyper_log2 [expr {$a | $b}]]
        return "address bit $bit has no effect: stuck, open, shorted, or above the chip size"
    }
    return "address bits [pulp_hyper_log2 $a] and [pulp_hyper_log2 $b] both have no effect, or are shorted together"
}

# Read 16 bytes from the CSN0 chip in HyperFlash mode. Reads are captured on
# the chip's RWDS strobe, so getting anything but the 0xa5 fill back means a
# device answered. Contents are not checked: this mode cannot write them.
proc pulp_hyper_probe_csn0 {} {
    global PULP_HYPER_L2_BUF PULP_HYPER_MEM_SEL

    set PULP_HYPER_MEM_SEL 1
    pulp_hyper_init
    set ok [expr {![catch {pulp_hyper_xfer 0 0 $PULP_HYPER_L2_BUF 16} msg]}]
    set PULP_HYPER_MEM_SEL 0
    pulp_hyper_init

    if {!$ok} {
        error "pulp_hyper: CSN0 probe: $msg (no device, or it never drove RWDS)"
    }
    set data [pulp_hyper_rd32_list $PULP_HYPER_L2_BUF 4]
    set words {}
    foreach w $data { lappend words [format 0x%08x $w] }
    set fill 1
    foreach w $data { if {$w != 0xa5a5a5a5} { set fill 0 } }
    if {$fill} {
        error "pulp_hyper: CSN0 probe: transfer ended but no data arrived ($words)"
    }
    echo "pulp_hyper: CSN0 answered: $words"
}

# Addressability check of both chips: full address-line test on CSN1, read
# probe on CSN0. Runs both even if the first fails, then reports.
proc pulp_hyper_check_chips {{chip_size 0x4000000}} {
    set failed {}
    if {[catch {pulp_hyperram_addr_test $chip_size} msg]} {
        echo $msg
        lappend failed CSN1
    }
    if {[catch {pulp_hyper_probe_csn0} msg]} {
        echo $msg
        lappend failed CSN0
    }
    if {[llength $failed]} {
        error "pulp_hyper: chip check FAILED on [join $failed {, }]"
    }
    echo "pulp_hyper: chip check passed (CSN1 addressable, CSN0 answering)"
}
