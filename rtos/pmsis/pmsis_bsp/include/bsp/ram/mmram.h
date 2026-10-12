/*
 * Copyright 2026 Fondazione Chips-IT.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BSP_RAM_MMRAM_H__
#define __BSP_RAM_MMRAM_H__

#include "bsp/ram.h"

/**
 * Memory-mapped RAM (e.g. HyperRAM behind the SoC interconnect) reachable by
 * the cluster iDMA only as AXI<->L1. Copies with the processor side (L2) are
 * staged through two L1 buffers: AXI src -> L1 stage -> AXI dst.
 * Open it after pi_cluster_open(): the staging buffers come from the L1 heap.
 */

#ifndef MMRAM_STAGE_BYTES
#define MMRAM_STAGE_BYTES 4096  // per staging buffer (two are allocated)
#endif

struct pi_mmram_conf
{
    struct pi_ram_conf ram;  /*!< Generic RAM configuration. */
    uint32_t ram_start;      /*!< Heap start; 0 = end of the linked L3 data (__l3_end). */
    uint32_t ram_size;       /*!< Heap size; 0 = up to the end of the L3 region. */
    uint32_t stage_bytes;    /*!< Bytes per L1 staging buffer, < 64 KiB. */
};

void pi_mmram_conf_init(struct pi_mmram_conf *conf);

#endif
