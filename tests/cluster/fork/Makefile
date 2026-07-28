APP = test
APP_SRCS = test.c
APP_CFLAGS += -O3 -g

# corev-gcc ICEs on cluster_entry with hardware loops (dangling loop-end
# LABEL_REF in hwloop_setupi_p). Disable doloop conversion for this file only.
ifdef USE_CV32E40P
APP_CFLAGS += -fno-branch-count-reg
endif

include $(RULES_DIR)/pmsis_rules.mk