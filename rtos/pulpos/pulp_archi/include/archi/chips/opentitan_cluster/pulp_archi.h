// Copyright 2026 ETH Zurich, University of Bologna and Fondazione Chips-IT.
// SPDX-License-Identifier: Apache-2.0

// Same IPs as pulp; own properties and memory map, plus mailboxes
#ifndef __ARCHI_CHIPS_OPENTITAN_CLUSTER_PULP_ARCHI_H__
#define __ARCHI_CHIPS_OPENTITAN_CLUSTER_PULP_ARCHI_H__

#include "archi/chips/opentitan_cluster/properties.h"
#include "archi/chips/pulp/apb_soc_ctrl.h"

#include "archi/gpio/gpio_v3.h"
#include "archi/riscv/priv_1_10.h"
#include "archi/riscv/pcer_v2.h"
#include "archi/itc/itc_v1.h"

#include "archi/chips/opentitan_cluster/memory_map.h"
#include "archi/chips/pulp/apb_soc_ctrl/apb_soc_ctrl.h"
#include "archi/chips/pulp/cluster_ctrl_unit/cluster_ctrl_unit.h"
#include "archi/chips/pulp/cluster_icache_ctrl/cluster_icache_ctrl.h"
#include "archi/stdout/stdout_v3.h"
#include "archi/eu/eu_v3.h"
#include "archi/dma/mchan_v7.h"
#include "archi/dma/idma_v2.h"
#include "archi/ima/ima_v1.h"
#include "archi/mailboxes/mailboxes.h"

#include "archi/udma/cpi/udma_cpi_v1.h"
#include "archi/udma/i2c/udma_i2c_v2.h"
#include "archi/udma/i2s/udma_i2s_v2.h"
#include "archi/udma/spim/udma_spim_v3.h"
#include "archi/udma/uart/udma_uart_v1.h"
#include "archi/udma/hyper/udma_hyper_v3.h"
#include "archi/udma/udma_v3.h"

#endif
