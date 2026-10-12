# PULP cluster inside the OpenTitan secure domain: pulp target with OT memory map
POS_CHIP := opentitan_cluster

# L3 window in HyperRAM (defaults in link.ld): make L3_ADDR=0x80400000 L3_SIZE=0x400000
ifdef L3_ADDR
PULP_LDFLAGS += -Wl,--defsym=__l3_base=$(L3_ADDR)
endif
ifdef L3_SIZE
PULP_LDFLAGS += -Wl,--defsym=__l3_size=$(L3_SIZE)
endif

# L3 as a pi_ram device (bsp/ram/mmram.h)
CONFIG_MMRAM ?= 1

include $(PULPOS_PULP_HOME)/rules/pulpos/targets/pulp.mk
