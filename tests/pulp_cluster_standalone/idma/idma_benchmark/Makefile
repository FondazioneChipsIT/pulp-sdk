PULP_APP = test
TEST_SRCS ?= test.c
PULP_APP_SRCS = $(TEST_SRCS)

ifdef DEBUG_TEST
PULP_CFLAGS += -DDEBUG_TEST
endif

ifdef TX
PULP_CFLAGS += -DTX
endif

ifdef RX
PULP_CFLAGS += -DRX
endif

ifdef TX_RX
PULP_CFLAGS += -DTX_RX
endif

ifdef IDLE
PULP_CFLAGS += -DIDLE
endif

ifdef DISABLE_ERROR_CHECK
PULP_CFLAGS += -DDISABLE_ERROR_CHECK
endif

ifdef CYCLE_COUNT
PULP_CFLAGS += -DCYCLE_COUNT
PULP_CFLAGS += -DDEBUG_TEST
endif

PULP_CFLAGS += -O3

include $(RULES_DIR)/pmsis_rules.mk

printf_flags:
	echo ${PULP_CFLAGS}