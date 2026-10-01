/*
 * Copyright (C) 2018 ETH Zurich and University of Bologna
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * xpulp instructions used by the kernel .S files. Each macro has a native form,
 * spelled "p.<op>" by pulp-gcc and "cv.<op>" by the CoreV assembler, and a plain
 * RV32I one for when a reduced -march leaves the extension out. The generic
 * forms are free except where noted at the macro.
 */

#ifndef __ARCHI_RISCV_XPULP_ASM_H__
#define __ARCHI_RISCV_XPULP_ASM_H__

/* ---- which extensions may we use -------------------------------------- */

#ifdef __cv32e40p__

#ifdef __riscv_xcvmem
#define XPULP_HAS_MEM 1
#endif
#ifdef __riscv_xcvbitmanip
#define XPULP_HAS_BITMANIP 1
#endif
#ifdef __riscv_xcvbi
#define XPULP_HAS_BI 1
#endif

#else   /* pulp-gcc: the xgap9 ISA carries all of them */

#define XPULP_HAS_MEM 1
#define XPULP_HAS_BITMANIP 1
#define XPULP_HAS_BI 1

#endif

/* ---- event load: rd = [base + imm], core idle until the event ---------- */

/* No generic form: it sits on the barrier path. */
#ifdef __cv32e40p__
#define XPULP_ELW cv.elw
#else
#define XPULP_ELW p.elw
#endif

/* ---- register-offset load and store: [base + off] ---------------------- */

/* The generic form CLOBBERS off; the callers reload it before every use. */
#ifdef XPULP_HAS_MEM

#ifdef __cv32e40p__
#define XPULP_LW(rd, off, base) cv.lw rd, off(base)
#define XPULP_SW(rd, off, base) cv.sw rd, off(base)
#else
#define XPULP_LW(rd, off, base) p.lw rd, off(base)
#define XPULP_SW(rd, off, base) p.sw rd, off(base)
#endif

#else

#define XPULP_LW(rd, off, base) add off, off, base ; lw rd, 0(off)
#define XPULP_SW(rd, off, base) add off, off, base ; sw rd, 0(off)

#endif

/* ---- set one bit: rd = rs1 | (1 << bit) -------------------------------- */

/* The callers mask bit to five bits, so the width is one. The generic form
 * needs a scratch register, which the caller names. */
#ifdef XPULP_HAS_BITMANIP

#ifdef __cv32e40p__
#define XPULP_BSETR(rd, rs1, bit, tmp) cv.bsetr rd, rs1, bit
#else
#define XPULP_BSETR(rd, rs1, bit, tmp) p.bsetr rd, rs1, bit
#endif

#else

#define XPULP_BSETR(rd, rs1, bit, tmp) \
    li tmp, 1 ; sll tmp, tmp, bit ; or rd, rs1, tmp

#endif

/* ---- extract an unsigned bit field ------------------------------------- */

/* size is passed as size-1. The generic form needs size <= 11, to fit andi. */
#ifdef XPULP_HAS_BITMANIP

#ifdef __cv32e40p__
#define XPULP_EXTRACTU(rd, rs1, size_m1, off) cv.extractu rd, rs1, size_m1, off
#else
#define XPULP_EXTRACTU(rd, rs1, size_m1, off) p.extractu rd, rs1, size_m1, off
#endif

#else

#define XPULP_EXTRACTU(rd, rs1, size_m1, off) \
    srli rd, rs1, off ; andi rd, rd, ((1 << ((size_m1) + 1)) - 1)

#endif

/* ---- branch if equal to zero ------------------------------------------- */

/* Against zero only: the one case used, and free in RV32I. */
#ifdef XPULP_HAS_BI

#ifdef __cv32e40p__
#define XPULP_BEQZ(rs1, label) cv.beqimm rs1, 0, label
#else
#define XPULP_BEQZ(rs1, label) p.beqimm rs1, 0, label
#endif

#else

#define XPULP_BEQZ(rs1, label) beqz rs1, label

#endif

#endif
