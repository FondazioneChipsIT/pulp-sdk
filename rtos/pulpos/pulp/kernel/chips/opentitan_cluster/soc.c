// Copyright 2026 ETH Zurich, University of Bologna and Fondazione Chips-IT.
// SPDX-License-Identifier: Apache-2.0

#include "pmsis.h"

// No FLL on the OT cluster: report fixed frequencies
void pos_soc_init()
{
    pos_freq_domains[PI_FREQ_DOMAIN_FC] = ARCHI_FPGA_SOC_FREQUENCY;

    pos_freq_domains[PI_FREQ_DOMAIN_PERIPH] = ARCHI_FPGA_PER_FREQUENCY;

    pos_freq_domains[PI_FREQ_DOMAIN_CL] = ARCHI_FPGA_CL_FREQUENCY;
}
