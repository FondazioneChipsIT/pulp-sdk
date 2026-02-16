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

# echo -O3 -g    -D__riscv__ -fdata-sections -ffunction-sections -include pos/chips/pulp/config.h -I/scratch/riccardo.gandolfi/work/pulp_cluster_sdk/pulp-sdk/rtos/pulpos/pulp/include/pos/chips/pulp -I/scratch/riccardo.gandolfi/work/pulp_cluster_sdk/pulp-sdk/ext_libs/include -fno-jump-tables -fno-tree-loop-distribute-patterns -DCONFIG_PULP -DCONFIG_BOARD_VERSION_PULP -DCONFIG_PROFILE_PULP -D__CONFIG_UDMA__ -D__PULPOS2__ -D__PLATFORM__=ARCHI_PLATFORM_RTL -D__PLATFORM_RTL__ -DARCHI_CLUSTER_NB_PE=8 -DARCHI_NO_FC=1 -DPOS_CONFIG_IO_UART=1 -DPOS_CONFIG_IO_UART_BAUDRATE=115200 -DPOS_CONFIG_IO_UART_ITF=1 -D__TRACE_LEVEL__=3 -DPI_LOG_LOCAL_LEVEL=2 -DCONFIG_PULP -DCONFIG_BOARD_VERSION_PULP -DCONFIG_PROFILE_PULP
# -O3 -g -D__riscv__ -fdata-sections -ffunction-sections -include pos/chips/pulp/config.h -I/scratch/riccardo.gandolfi/work/pulp_cluster_sdk/pulp-sdk/rtos/pulpos/pulp/include/pos/chips/pulp -I/scratch/riccardo.gandolfi/work/pulp_cluster_sdk/pulp-sdk/ext_libs/include -fno-jump-tables -fno-tree-loop-distribute-patterns -DCONFIG_PULP -DCONFIG_BOARD_VERSION_PULP -DCONFIG_PROFILE_PULP -D__CONFIG_UDMA__ -D__PULPOS2__ -D__PLATFORM__=ARCHI_PLATFORM_RTL -D__PLATFORM_RTL__ -DARCHI_CLUSTER_NB_PE=8 -DARCHI_NO_FC=1 -DPOS_CONFIG_IO_UART=1 -DPOS_CONFIG_IO_UART_BAUDRATE=115200 -DPOS_CONFIG_IO_UART_ITF=1 -D__TRACE_LEVEL__=3 -DPI_LOG_LOCAL_LEVEL=2 -DCONFIG_PULP -DCONFIG_BOARD_VERSION_PULP -DCONFIG_PROFILE_PULP
