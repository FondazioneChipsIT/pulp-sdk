// Register defines for idma_reg32_3d
//
// Matches iDMA src/frontend/reg/idma_reg.rdl generated with
//   SysAddrWidth=32, NumDims=3, Log2NumDims=2   (see idma.mk)
// Cross-checked against target/rtl/idma_reg32_3d_reg_top.sv and the
// PeakRDL raw-header output (idma_reg32_3d_raw_regs.h).

// Copyright information found in source file:
// Copyright 2023 ETH Zurich and University of Bologna.

// Licensing information found in source file:
//
// SPDX-License-Identifier: SHL-0.51

#ifndef __INCLUDE_ARCHI_IDMA_V2_H__
#define __INCLUDE_ARCHI_IDMA_V2_H__

#ifndef LANGUAGE_ASSEMBLY

#include <stdint.h>
#include "archi/utils.h"

#endif

#ifdef __cplusplus
extern "C" {
#endif
// Number of dimensions available
#define IDMA_REG32_3D_PARAM_NUM_DIMS 3

// Log2 of the number of dimensions (width of conf.enable_nd - 1)
#define IDMA_REG32_3D_PARAM_LOG2_NUM_DIMS 2

// Register width
#define IDMA_REG32_3D_PARAM_REG_WIDTH 32

// Max number of stream slots in status/next_id/done_id arrays
#define IDMA_REG32_3D_PARAM_MAX_NUM_STREAMS 16

// Total size of the register file
#define IDMA_REG32_3D_SIZE 0x114

// ---------------------------------------------------------------------------
// conf: Configuration Register for DMA settings
// ---------------------------------------------------------------------------
#define IDMA_REG32_3D_CONF_REG_OFFSET 0x0
#define IDMA_REG32_3D_CONF_DECOUPLE_AW_BIT 0
#define IDMA_REG32_3D_CONF_DECOUPLE_RW_BIT 1
#define IDMA_REG32_3D_CONF_SRC_REDUCE_LEN_BIT 2
#define IDMA_REG32_3D_CONF_DST_REDUCE_LEN_BIT 3
#define IDMA_REG32_3D_CONF_SRC_MAX_LLEN_MASK 0x7
#define IDMA_REG32_3D_CONF_SRC_MAX_LLEN_OFFSET 4
#define IDMA_REG32_3D_CONF_SRC_MAX_LLEN_FIELD \
  ((bitfield_field32_t) { .mask = IDMA_REG32_3D_CONF_SRC_MAX_LLEN_MASK, .index = IDMA_REG32_3D_CONF_SRC_MAX_LLEN_OFFSET })
#define IDMA_REG32_3D_CONF_DST_MAX_LLEN_MASK 0x7
#define IDMA_REG32_3D_CONF_DST_MAX_LLEN_OFFSET 7
#define IDMA_REG32_3D_CONF_DST_MAX_LLEN_FIELD \
  ((bitfield_field32_t) { .mask = IDMA_REG32_3D_CONF_DST_MAX_LLEN_MASK, .index = IDMA_REG32_3D_CONF_DST_MAX_LLEN_OFFSET })
// enable_nd: 0 = 1D, 1 = 2D, >=2 = 3D   -> bits [12:10]
#define IDMA_REG32_3D_CONF_ENABLE_ND_MASK 0x7
#define IDMA_REG32_3D_CONF_ENABLE_ND_OFFSET 10
#define IDMA_REG32_3D_CONF_ENABLE_ND_FIELD \
  ((bitfield_field32_t) { .mask = IDMA_REG32_3D_CONF_ENABLE_ND_MASK, .index = IDMA_REG32_3D_CONF_ENABLE_ND_OFFSET })
// src_protocol (idma_pkg::protocol_e) -> bits [15:13]
#define IDMA_REG32_3D_CONF_SRC_PROTOCOL_MASK 0x7
#define IDMA_REG32_3D_CONF_SRC_PROTOCOL_OFFSET 13
#define IDMA_REG32_3D_CONF_SRC_PROTOCOL_FIELD \
  ((bitfield_field32_t) { .mask = IDMA_REG32_3D_CONF_SRC_PROTOCOL_MASK, .index = IDMA_REG32_3D_CONF_SRC_PROTOCOL_OFFSET })
// dst_protocol (idma_pkg::protocol_e) -> bits [18:16]
#define IDMA_REG32_3D_CONF_DST_PROTOCOL_MASK 0x7
#define IDMA_REG32_3D_CONF_DST_PROTOCOL_OFFSET 16
#define IDMA_REG32_3D_CONF_DST_PROTOCOL_FIELD \
  ((bitfield_field32_t) { .mask = IDMA_REG32_3D_CONF_DST_PROTOCOL_MASK, .index = IDMA_REG32_3D_CONF_DST_PROTOCOL_OFFSET })

// ---------------------------------------------------------------------------
// status[16] / next_id[16] / done_id[16]  (one slot per stream)
// In pulp_cluster: stream 0 = L1 -> L2 (OBI -> AXI), stream 1 = L2 -> L1 (AXI -> OBI)
// ---------------------------------------------------------------------------
#define IDMA_REG32_3D_STATUS_REG_OFFSET(idx)  (0x04 + ((idx) * 0x4))
#define IDMA_REG32_3D_NEXT_ID_REG_OFFSET(idx) (0x44 + ((idx) * 0x4))
#define IDMA_REG32_3D_DONE_ID_REG_OFFSET(idx) (0x84 + ((idx) * 0x4))

// DMA Status (busy = {midend_busy, backend busy bits})
#define IDMA_REG32_3D_STATUS_0_REG_OFFSET 0x4
#define IDMA_REG32_3D_STATUS_0_BUSY_0_MASK 0x3ff
#define IDMA_REG32_3D_STATUS_0_BUSY_0_OFFSET 0
#define IDMA_REG32_3D_STATUS_0_BUSY_0_FIELD \
  ((bitfield_field32_t) { .mask = IDMA_REG32_3D_STATUS_0_BUSY_0_MASK, .index = IDMA_REG32_3D_STATUS_0_BUSY_0_OFFSET })

// DMA Status
#define IDMA_REG32_3D_STATUS_1_REG_OFFSET 0x8
#define IDMA_REG32_3D_STATUS_1_BUSY_1_MASK 0x3ff
#define IDMA_REG32_3D_STATUS_1_BUSY_1_OFFSET 0
#define IDMA_REG32_3D_STATUS_1_BUSY_1_FIELD \
  ((bitfield_field32_t) { .mask = IDMA_REG32_3D_STATUS_1_BUSY_1_MASK, .index = IDMA_REG32_3D_STATUS_1_BUSY_1_OFFSET })

// Next ID, launches transfer, returns 0 if transfer not set up properly.
#define IDMA_REG32_3D_NEXT_ID_0_REG_OFFSET 0x44

// Next ID, launches transfer, returns 0 if transfer not set up properly.
#define IDMA_REG32_3D_NEXT_ID_1_REG_OFFSET 0x48

// Get ID of finished transactions.
#define IDMA_REG32_3D_DONE_ID_0_REG_OFFSET 0x84

// Get ID of finished transactions.
#define IDMA_REG32_3D_DONE_ID_1_REG_OFFSET 0x88

// ---------------------------------------------------------------------------
// Transfer descriptor (32-bit addresses: one register each, 4-byte spacing).
// The *_LOW_* names are kept for source compatibility with existing drivers;
// there are no *_HIGH_* counterparts on the 32-bit frontend.
// ---------------------------------------------------------------------------

// Destination address
#define IDMA_REG32_3D_DST_ADDR_REG_OFFSET 0xd0
#define IDMA_REG32_3D_DST_ADDR_LOW_REG_OFFSET IDMA_REG32_3D_DST_ADDR_REG_OFFSET

// Source address
#define IDMA_REG32_3D_SRC_ADDR_REG_OFFSET 0xd4
#define IDMA_REG32_3D_SRC_ADDR_LOW_REG_OFFSET IDMA_REG32_3D_SRC_ADDR_REG_OFFSET

// Transfer length in byte
#define IDMA_REG32_3D_LENGTH_REG_OFFSET 0xd8
#define IDMA_REG32_3D_LENGTH_LOW_REG_OFFSET IDMA_REG32_3D_LENGTH_REG_OFFSET

// dim[0] -> dimension 2
#define IDMA_REG32_3D_DST_STRIDE_2_REG_OFFSET 0xe0
#define IDMA_REG32_3D_SRC_STRIDE_2_REG_OFFSET 0xe4
#define IDMA_REG32_3D_REPS_2_REG_OFFSET       0xe8
#define IDMA_REG32_3D_DST_STRIDE_2_LOW_REG_OFFSET IDMA_REG32_3D_DST_STRIDE_2_REG_OFFSET
#define IDMA_REG32_3D_SRC_STRIDE_2_LOW_REG_OFFSET IDMA_REG32_3D_SRC_STRIDE_2_REG_OFFSET
#define IDMA_REG32_3D_REPS_2_LOW_REG_OFFSET       IDMA_REG32_3D_REPS_2_REG_OFFSET

// dim[1] -> dimension 3
#define IDMA_REG32_3D_DST_STRIDE_3_REG_OFFSET 0xec
#define IDMA_REG32_3D_SRC_STRIDE_3_REG_OFFSET 0xf0
#define IDMA_REG32_3D_REPS_3_REG_OFFSET       0xf4
#define IDMA_REG32_3D_DST_STRIDE_3_LOW_REG_OFFSET IDMA_REG32_3D_DST_STRIDE_3_REG_OFFSET
#define IDMA_REG32_3D_SRC_STRIDE_3_LOW_REG_OFFSET IDMA_REG32_3D_SRC_STRIDE_3_REG_OFFSET
#define IDMA_REG32_3D_REPS_3_LOW_REG_OFFSET       IDMA_REG32_3D_REPS_3_REG_OFFSET

// ---------------------------------------------------------------------------
// compute_cfg: per-transfer on-the-fly compute, sampled when next_id is read
// ---------------------------------------------------------------------------
#define IDMA_REG32_3D_COMPUTE_CFG_REG_OFFSET 0xf8
#define IDMA_REG32_3D_COMPUTE_CFG_COMPUTE_ENABLE_BIT 0
#define IDMA_REG32_3D_COMPUTE_CFG_COMPUTE_OP_MASK 0xf
#define IDMA_REG32_3D_COMPUTE_CFG_COMPUTE_OP_OFFSET 1
#define IDMA_REG32_3D_COMPUTE_CFG_COMPUTE_OP_FIELD \
  ((bitfield_field32_t) { .mask = IDMA_REG32_3D_COMPUTE_CFG_COMPUTE_OP_MASK, .index = IDMA_REG32_3D_COMPUTE_CFG_COMPUTE_OP_OFFSET })
#define IDMA_REG32_3D_COMPUTE_CFG_TRANSPOSE_MODE_MASK 0x3
#define IDMA_REG32_3D_COMPUTE_CFG_TRANSPOSE_MODE_OFFSET 5
#define IDMA_REG32_3D_COMPUTE_CFG_TRANSPOSE_MODE_FIELD \
  ((bitfield_field32_t) { .mask = IDMA_REG32_3D_COMPUTE_CFG_TRANSPOSE_MODE_MASK, .index = IDMA_REG32_3D_COMPUTE_CFG_TRANSPOSE_MODE_OFFSET })
#define IDMA_REG32_3D_COMPUTE_CFG_TRANSPOSE_TENSOR_M_MASK 0xfff
#define IDMA_REG32_3D_COMPUTE_CFG_TRANSPOSE_TENSOR_M_OFFSET 7
#define IDMA_REG32_3D_COMPUTE_CFG_TRANSPOSE_TENSOR_M_FIELD \
  ((bitfield_field32_t) { .mask = IDMA_REG32_3D_COMPUTE_CFG_TRANSPOSE_TENSOR_M_MASK, .index = IDMA_REG32_3D_COMPUTE_CFG_TRANSPOSE_TENSOR_M_OFFSET })
#define IDMA_REG32_3D_COMPUTE_CFG_TRANSPOSE_TENSOR_N_MASK 0xfff
#define IDMA_REG32_3D_COMPUTE_CFG_TRANSPOSE_TENSOR_N_OFFSET 19
#define IDMA_REG32_3D_COMPUTE_CFG_TRANSPOSE_TENSOR_N_FIELD \
  ((bitfield_field32_t) { .mask = IDMA_REG32_3D_COMPUTE_CFG_TRANSPOSE_TENSOR_N_MASK, .index = IDMA_REG32_3D_COMPUTE_CFG_TRANSPOSE_TENSOR_N_OFFSET })
#define IDMA_REG32_3D_COMPUTE_CFG_TRANSPOSE_COMPACT_BIT 31

// compute_op encoding (idma_pkg::compute_op_e)
#define IDMA_REG32_3D_COMPUTE_OP_NONE           0
#define IDMA_REG32_3D_COMPUTE_OP_TRANSPOSE      1
#define IDMA_REG32_3D_COMPUTE_OP_MXQUANT        2
#define IDMA_REG32_3D_COMPUTE_OP_MXQUANT_FP16   3
#define IDMA_REG32_3D_COMPUTE_OP_MXDEQUANT      4
#define IDMA_REG32_3D_COMPUTE_OP_MXDEQUANT_FP16 5

// ---------------------------------------------------------------------------
// mx_cfg: MX options, sampled with compute_cfg
// ---------------------------------------------------------------------------
#define IDMA_REG32_3D_MX_CFG_REG_OFFSET 0xfc
#define IDMA_REG32_3D_MX_CFG_MX_POISON_DIS_BIT 0
#define IDMA_REG32_3D_MX_CFG_MX_RCEIL_BIT 1
#define IDMA_REG32_3D_MX_CFG_MX_ELEM_FMT_MASK 0x3
#define IDMA_REG32_3D_MX_CFG_MX_ELEM_FMT_OFFSET 2
#define IDMA_REG32_3D_MX_CFG_MX_ELEM_FMT_FIELD \
  ((bitfield_field32_t) { .mask = IDMA_REG32_3D_MX_CFG_MX_ELEM_FMT_MASK, .index = IDMA_REG32_3D_MX_CFG_MX_ELEM_FMT_OFFSET })
#define IDMA_REG32_3D_MX_CFG_MX_GROUP_BIT 4

// mx_elem_fmt encoding
#define IDMA_REG32_3D_MX_ELEM_FMT_E5M2 0
#define IDMA_REG32_3D_MX_ELEM_FMT_E4M3 1

// mx_group encoding
#define IDMA_REG32_3D_MX_GROUP_G64 0
#define IDMA_REG32_3D_MX_GROUP_G32 1

// MX scale plane address (64 B aligned)
#define IDMA_REG32_3D_SCALE_ADDR_REG_OFFSET 0x100

// MX scale plane stride (multiple of 64 B): mx_dim[0] -> dim 2, mx_dim[1] -> dim 3
#define IDMA_REG32_3D_SCALE_STRIDE_2_REG_OFFSET 0x104
#define IDMA_REG32_3D_SCALE_STRIDE_3_REG_OFFSET 0x108

// ---------------------------------------------------------------------------
// user field
// ---------------------------------------------------------------------------
#define IDMA_REG32_3D_USER_HIGH_REG_OFFSET 0x10c
#define IDMA_REG32_3D_USER_LOW_REG_OFFSET  0x110

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // __INCLUDE_ARCHI_IDMA_V2_H__
// End register defines for idma_reg32_3d
