// Copyright 2026 ETH Zurich, University of Bologna and Fondazione Chips-IT.
// SPDX-License-Identifier: Apache-2.0

#ifndef __ARCHI_CHIPS_OPENTITAN_CLUSTER_PROPERTIES_H__
#define __ARCHI_CHIPS_OPENTITAN_CLUSTER_PROPERTIES_H__

#include "archi/chips/pulp/properties.h"

#define ARCHI_HAS_MAILBOXES 1

// EU event raised by the mailbox RCV irq (cluster_event_map)
#define ARCHI_CL_EVT_MBOX   22

// No FLL: fixed frequencies reported to pi_freq_get()
#ifndef ARCHI_FPGA_SOC_FREQUENCY
#define ARCHI_FPGA_SOC_FREQUENCY 5000000
#endif
#ifndef ARCHI_FPGA_PER_FREQUENCY
#define ARCHI_FPGA_PER_FREQUENCY 5000000
#endif
#ifndef ARCHI_FPGA_CL_FREQUENCY
#define ARCHI_FPGA_CL_FREQUENCY 5000000
#endif

#endif
