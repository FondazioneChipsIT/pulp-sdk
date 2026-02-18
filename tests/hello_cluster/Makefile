APP = test
APP_SRCS += test.c

ifdef USE_CLUSTER
APP_CFLAGS += -DCLUSTER -DNUM_CLUSTER=$(USE_CLUSTER)
ifdef NUM_CORES
APP_CFLAGS += -DNUM_CORES=$(NUM_CORES)
else
APP_CFLAGS += -DNUM_CORES=1
endif
endif

CONFIG_UART = 1

PULP_CFLAGS = -O3 -g

print_flags:
	echo ${PULP_CFLAGS}

# APP_CFLAGS += -Os -g
# APP_LDFLAGS += -Os -g

include $(RULES_DIR)/pmsis_rules.mk
