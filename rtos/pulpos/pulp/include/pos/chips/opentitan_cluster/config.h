// Copyright 2026 ETH Zurich, University of Bologna and Fondazione Chips-IT.
// SPDX-License-Identifier: Apache-2.0

#ifndef __POS__CHIPS__OPENTITAN_CLUSTER__CONFIG_H__
#define __POS__CHIPS__OPENTITAN_CLUSTER__CONFIG_H__

#include "archi/pulp_defs.h"

// Same IPs as pulp: keep CHIP_PULP/CONFIG_PULP, only archi/hal dirs differ
#define PULP_CHIP CHIP_PULP
#define PULP_CHIP_FAMILY CHIP_PULP
#define CONFIG_PULP 1
#define PULP_CHIP_STR opentitan_cluster
#define PULP_CHIP_FAMILY_STR opentitan_cluster

#ifdef __cv32e40p__
#define ARCHI_CORE_HAS_COREV_V2 1
#else
#define ARCHI_CORE_HAS_PULPV2 1
#endif

#define ARCHI_CORE_HAS_1_10 1

#endif
