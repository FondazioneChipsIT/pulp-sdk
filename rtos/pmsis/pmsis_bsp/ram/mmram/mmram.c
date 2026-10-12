/*
 * Copyright 2026 Fondazione Chips-IT.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "pmsis.h"
#include "bsp/ram/mmram.h"
#include "../extern_alloc.h"

// Region bounds from the chip link.ld
extern char __l3_base[], __l3_size[], __l3_end[];

#define MMRAM_IDMA_CLK_BIT (1u << 17)  // cluster-ctrl cfg: iDMA clock enable

typedef struct
{
    extern_alloc_t alloc;
    uint8_t *stage[2];
    uint32_t stage_bytes;
} mmram_t;

static int mmram_open(struct pi_device *device)
{
    struct pi_mmram_conf *conf = (struct pi_mmram_conf *)device->config;
    uint32_t region_end = (uint32_t)__l3_base + (uint32_t)__l3_size;
    uint32_t start = conf->ram_start ? conf->ram_start : (uint32_t)__l3_end;
    uint32_t size = conf->ram_size ? conf->ram_size : region_end - start;

    if (conf->stage_bytes == 0 || conf->stage_bytes > 0xFFFC)
        return -1;

    mmram_t *mmram = (mmram_t *)pmsis_l2_malloc(sizeof(mmram_t));
    if (mmram == NULL)
        return -1;

    mmram->stage_bytes = conf->stage_bytes & ~3u;
    mmram->stage[0] = (uint8_t *)pmsis_l1_malloc(2 * mmram->stage_bytes);
    if (mmram->stage[0] == NULL)
        goto error;
    mmram->stage[1] = mmram->stage[0] + mmram->stage_bytes;

    if (extern_alloc_init(&mmram->alloc, (void *)start, size))
        goto error_l1;

    device->data = (void *)mmram;
    return 0;

error_l1:
    pmsis_l1_malloc_free(mmram->stage[0], 2 * mmram->stage_bytes);
error:
    pmsis_l2_malloc_free(mmram, sizeof(mmram_t));
    return -2;
}

static void mmram_close(struct pi_device *device)
{
    mmram_t *mmram = (mmram_t *)device->data;
    extern_alloc_deinit(&mmram->alloc);
    pmsis_l1_malloc_free(mmram->stage[0], 2 * mmram->stage_bytes);
    pmsis_l2_malloc_free(mmram, sizeof(mmram_t));
}

// Walks the copy as pieces of <= stage_bytes: RAM side rows of `length` bytes
// every `stride`, processor side contiguous. Short rows are batched into one
// piece (2D on the RAM side), long rows are split.
typedef struct
{
    uint32_t addr, data, size, stride, length, stage_bytes;
    int ext2loc;
    uint32_t row, off;
} mmram_it_t;

typedef struct
{
    uint32_t src, dst, n;
    uint32_t len, reps;  // reps > 0: `reps` rows of `len` bytes, RAM stride it->stride
} mmram_piece_t;

static int mmram_next_piece(mmram_it_t *it, mmram_piece_t *p)
{
    uint32_t row_start = it->row * it->length;
    if (row_start >= it->size)
        return 0;

    uint32_t row_bytes = it->size - row_start;
    if (row_bytes > it->length)
        row_bytes = it->length;

    uint32_t ram = it->addr + it->row * it->stride + it->off;
    uint32_t loc = it->data + row_start + it->off;
    uint32_t full = (it->size - row_start) / it->length;
    uint32_t n;

    p->reps = 0;
    if (it->off == 0 && it->length <= it->stage_bytes && full >= 2)
    {
        uint32_t reps = it->stage_bytes / it->length;
        if (reps > full)
            reps = full;
        p->len = it->length;
        p->reps = reps;
        n = reps * it->length;
        it->row += reps;
    }
    else
    {
        n = row_bytes - it->off;
        if (n > it->stage_bytes)
            n = it->stage_bytes;
        it->off += n;
        if (it->off >= row_bytes)
        {
            it->off = 0;
            it->row++;
        }
    }

    p->src = it->ext2loc ? ram : loc;
    p->dst = it->ext2loc ? loc : ram;
    p->n = n;
    return 1;
}

// src -> stage (AXI->L1, stream 1); 2D when the source is a batch of RAM rows
static unsigned int mmram_fill(mmram_it_t *it, mmram_piece_t *p, uint32_t stage)
{
    if (it->ext2loc && p->reps)
        return pulp_cl_idma_L2ToL1_2d(p->src, stage, (unsigned short)p->len, it->stride, p->len, p->reps);
    return pulp_cl_idma_L2ToL1(p->src, stage, (unsigned short)p->n);
}

// stage -> dst (L1->AXI, stream 0); 2D when the destination is a batch of RAM rows
static unsigned int mmram_drain(mmram_it_t *it, mmram_piece_t *p, uint32_t stage)
{
    if (!it->ext2loc && p->reps)
        return pulp_cl_idma_L1ToL2_2d(stage, p->dst, (unsigned short)p->len, p->len, it->stride, p->reps);
    return pulp_cl_idma_L1ToL2(stage, p->dst, (unsigned short)p->n);
}

// Each piece goes src -> stage[k] (stream 1) -> dst (stream 0).
// The fill of piece n+1 is queued before waiting on the fill of piece n, so the
// read latency of the source is hidden too; drain n overlaps fill n+1.
static void mmram_copy_2d_sync(mmram_t *mmram, uint32_t addr, uint8_t *data, uint32_t size,
                               uint32_t stride, uint32_t length, int ext2loc)
{
    if (size == 0 || length == 0)
        return;

    // Contiguous rows: one long row
    if (stride == length)
        stride = length = size;

    uint32_t cfg = plp_ctrl_cluster_cfg_get();
    if (!(cfg & MMRAM_IDMA_CLK_BIT))
        plp_ctrl_cluster_cfg_set(cfg | MMRAM_IDMA_CLK_BIT);

    mmram_it_t it = {addr, (uint32_t)data, size, stride, length, mmram->stage_bytes, ext2loc, 0, 0};
    mmram_piece_t cur, nxt;
    uint32_t stage[2] = {(uint32_t)mmram->stage[0], (uint32_t)mmram->stage[1]};
    unsigned int fill_id[2], drain_id[2];
    int draining[2] = {0, 0};
    int k = 0;

    mmram_next_piece(&it, &cur);
    fill_id[0] = mmram_fill(&it, &cur, stage[0]);

    for (;;)
    {
        int have_nxt = mmram_next_piece(&it, &nxt);
        if (have_nxt)
        {
            if (draining[k ^ 1])
                plp_cl_dma_wait_toL2(drain_id[k ^ 1]);  // other buffer free again
            draining[k ^ 1] = 0;
            fill_id[k ^ 1] = mmram_fill(&it, &nxt, stage[k ^ 1]);
        }

        plp_cl_dma_wait_toL1(fill_id[k]);  // ids complete in order per stream
        drain_id[k] = mmram_drain(&it, &cur, stage[k]);
        draining[k] = 1;

        if (!have_nxt)
            break;
        cur = nxt;
        k ^= 1;
    }

    for (int i = 0; i < 2; i++)
        if (draining[i])
            plp_cl_dma_wait_toL2(drain_id[i]);

    // Restore the saved value: plp_idma_disable_clk() would clear the whole register
    if (!(cfg & MMRAM_IDMA_CLK_BIT))
        plp_ctrl_cluster_cfg_set(cfg);
}

static void mmram_copy_async(struct pi_device *device, uint32_t addr, void *data, uint32_t size,
                             int ext2loc, pi_task_t *task)
{
    mmram_copy_2d_sync((mmram_t *)device->data, addr, (uint8_t *)data, size, size, size, ext2loc);
    pi_task_push(task);
}

static void mmram_copy_2d_async(struct pi_device *device, uint32_t addr, void *data, uint32_t size,
                                uint32_t stride, uint32_t length, int ext2loc, pi_task_t *task)
{
    mmram_copy_2d_sync((mmram_t *)device->data, addr, (uint8_t *)data, size, stride, length, ext2loc);
    pi_task_push(task);
}

static int mmram_alloc(struct pi_device *device, uint32_t *addr, uint32_t size)
{
    void *chunk;
    mmram_t *mmram = (mmram_t *)device->data;
    int err = extern_alloc(&mmram->alloc, size, &chunk);
    *addr = (uint32_t)chunk;
    return err;
}

static int mmram_free(struct pi_device *device, uint32_t addr, uint32_t size)
{
    mmram_t *mmram = (mmram_t *)device->data;
    return extern_free(&mmram->alloc, size, (void *)addr);
}

static int mmram_ioctl(struct pi_device *device, uint32_t cmd, void *arg)
{
    return 0;
}

static pi_ram_api_t mmram_api = {
    .ioctl         = &mmram_ioctl,
    .open          = &mmram_open,
    .close         = &mmram_close,
    .copy_async    = &mmram_copy_async,
    .copy_2d_async = &mmram_copy_2d_async,
    .alloc         = &mmram_alloc,
    .free          = &mmram_free,
};

void pi_mmram_conf_init(struct pi_mmram_conf *conf)
{
    conf->ram.api = &mmram_api;
    conf->ram_start = 0;
    conf->ram_size = 0;
    conf->stage_bytes = MMRAM_STAGE_BYTES;
}
