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
 * CV32E40P: route the xpulpv2 builtin macros onto the native CoreV intrinsics
 * instead of the pure-C emulation in builtins_v2_emu.h. Include AFTER the
 * emulation headers; the emulation stays the fallback for anything with no
 * intrinsic (see the bottom of this file).
 *
 * Intrinsic names come from corev-gcc/gcc/config/riscv/corev.def and are always
 * __builtin_riscv_cv_<ext>_<op>. All cv_simd_* take and return plain unsigned
 * int, not vector types, hence the punning helpers.
 *
 * Measured at -O3, instructions per op (emulated -> intrinsic):
 *   sumdotp4 23->3   abs2 14->2   dotp2 10->2   max2 7->2   clip 3->2
 */

#ifndef __ARCHI_RISCV_BUILTINS_CV32E40P_H__
#define __ARCHI_RISCV_BUILTINS_CV32E40P_H__

#ifdef __cv32e40p__

/* v2s/v2u/v4s/v4u are all 4 bytes, so this is a reinterpret, not a conversion.
 * Compiles to nothing: the ops below emit the bare instruction at -O2. */
#define __CV_PUN(from_t, to_t, name) \
  static inline to_t name(from_t x) { union { from_t f; to_t t; } c; c.f = x; return c.t; }

__CV_PUN(v2s, unsigned int, __cv_u_of_v2s)
__CV_PUN(v2u, unsigned int, __cv_u_of_v2u)
__CV_PUN(v4s, unsigned int, __cv_u_of_v4s)
__CV_PUN(v4u, unsigned int, __cv_u_of_v4u)
__CV_PUN(unsigned int, v2s, __cv_v2s_of_u)
__CV_PUN(unsigned int, v2u, __cv_v2u_of_u)
__CV_PUN(unsigned int, v4s, __cv_v4s_of_u)
__CV_PUN(unsigned int, v4u, __cv_v4u_of_u)

#undef __CV_PUN


/* ---- SIMD dot products: cv.dotsp/dotup/dotusp.{h,b} ------------------- */

#undef __builtin_dotp2
#undef __builtin_dotpu2
#undef __builtin_dotpus2
#define __builtin_dotp2(x, y)   ((int)         __builtin_riscv_cv_simd_dotsp_h (__cv_u_of_v2s(x), __cv_u_of_v2s(y)))
#define __builtin_dotpu2(x, y)  ((unsigned int)__builtin_riscv_cv_simd_dotup_h (__cv_u_of_v2u(x), __cv_u_of_v2u(y)))
#define __builtin_dotpus2(x, y) ((int)         __builtin_riscv_cv_simd_dotusp_h(__cv_u_of_v2u(x), __cv_u_of_v2s(y)))

#undef __builtin_dotp4
#undef __builtin_dotpu4
#undef __builtin_dotpus4
#define __builtin_dotp4(x, y)   ((int)         __builtin_riscv_cv_simd_dotsp_b (__cv_u_of_v4s(x), __cv_u_of_v4s(y)))
#define __builtin_dotpu4(x, y)  ((unsigned int)__builtin_riscv_cv_simd_dotup_b (__cv_u_of_v4u(x), __cv_u_of_v4u(y)))
#define __builtin_dotpus4(x, y) ((int)         __builtin_riscv_cv_simd_dotusp_b(__cv_u_of_v4u(x), __cv_u_of_v4s(y)))

/* ---- SIMD sum-of-dot-product (accumulating): cv.sdot*.{h,b} ---------- */

#undef __builtin_sumdotp2
#undef __builtin_sumdotpu2
#undef __builtin_sumdotpus2
#define __builtin_sumdotp2(x, y, z)   ((int)         __builtin_riscv_cv_simd_sdotsp_h (__cv_u_of_v2s(x), __cv_u_of_v2s(y), (unsigned int)(z)))
#define __builtin_sumdotpu2(x, y, z)  ((unsigned int)__builtin_riscv_cv_simd_sdotup_h (__cv_u_of_v2u(x), __cv_u_of_v2u(y), (unsigned int)(z)))
#define __builtin_sumdotpus2(x, y, z) ((int)         __builtin_riscv_cv_simd_sdotusp_h(__cv_u_of_v2u(x), __cv_u_of_v2s(y), (unsigned int)(z)))

#undef __builtin_sumdotp4
#undef __builtin_sumdotpu4
#undef __builtin_sumdotpus4
#define __builtin_sumdotp4(x, y, z)   ((int)         __builtin_riscv_cv_simd_sdotsp_b (__cv_u_of_v4s(x), __cv_u_of_v4s(y), (unsigned int)(z)))
#define __builtin_sumdotpu4(x, y, z)  ((unsigned int)__builtin_riscv_cv_simd_sdotup_b (__cv_u_of_v4u(x), __cv_u_of_v4u(y), (unsigned int)(z)))
#define __builtin_sumdotpus4(x, y, z) ((int)         __builtin_riscv_cv_simd_sdotusp_b(__cv_u_of_v4u(x), __cv_u_of_v4s(y), (unsigned int)(z)))

/* ---- SIMD min/max: cv.min/minu/max/maxu.{h,b} ------------------------ */

#undef __builtin_max2
#undef __builtin_max4
#undef __builtin_maxu2
#undef __builtin_maxu4
#define __builtin_max2(x, y)  __cv_v2s_of_u(__builtin_riscv_cv_simd_max_h (__cv_u_of_v2s(x), __cv_u_of_v2s(y)))
#define __builtin_max4(x, y)  __cv_v4s_of_u(__builtin_riscv_cv_simd_max_b (__cv_u_of_v4s(x), __cv_u_of_v4s(y)))
#define __builtin_maxu2(x, y) __cv_v2u_of_u(__builtin_riscv_cv_simd_maxu_h(__cv_u_of_v2u(x), __cv_u_of_v2u(y)))
#define __builtin_maxu4(x, y) __cv_v4u_of_u(__builtin_riscv_cv_simd_maxu_b(__cv_u_of_v4u(x), __cv_u_of_v4u(y)))

#undef __builtin_min2
#undef __builtin_min4
#undef __builtin_minu2
#undef __builtin_minu4
#define __builtin_min2(x, y)  __cv_v2s_of_u(__builtin_riscv_cv_simd_min_h (__cv_u_of_v2s(x), __cv_u_of_v2s(y)))
#define __builtin_min4(x, y)  __cv_v4s_of_u(__builtin_riscv_cv_simd_min_b (__cv_u_of_v4s(x), __cv_u_of_v4s(y)))
#define __builtin_minu2(x, y) __cv_v2u_of_u(__builtin_riscv_cv_simd_minu_h(__cv_u_of_v2u(x), __cv_u_of_v2u(y)))
#define __builtin_minu4(x, y) __cv_v4u_of_u(__builtin_riscv_cv_simd_minu_b(__cv_u_of_v4u(x), __cv_u_of_v4u(y)))

/* ---- SIMD pack: cv.pack, cv.packlo.b + cv.packhi.b ------------------- */

#undef __builtin_pack2
#undef __builtin_packu2
#define __builtin_pack2(x, y)  __cv_v2s_of_u(__builtin_riscv_cv_simd_packlo_h((unsigned int)(y), (unsigned int)(x)))
#define __builtin_packu2(x, y) __cv_v2u_of_u(__builtin_riscv_cv_simd_packlo_h((unsigned int)(y), (unsigned int)(x)))

#undef __builtin_pack4
#undef __builtin_packu4
#define __builtin_pack4(x, y, z, t) \
  __cv_v4s_of_u(__builtin_riscv_cv_simd_packhi_b((unsigned int)(t), (unsigned int)(z), \
                  __builtin_riscv_cv_simd_packlo_b((unsigned int)(y), (unsigned int)(x), 0u)))
#define __builtin_packu4(x, y, z, t) \
  __cv_v4u_of_u(__builtin_riscv_cv_simd_packhi_b((unsigned int)(t), (unsigned int)(z), \
                  __builtin_riscv_cv_simd_packlo_b((unsigned int)(y), (unsigned int)(x), 0u)))

/* ---- SIMD abs: cv.abs.{h,b} ------------------------------------------ */

#undef __builtin_abs2
#undef __builtin_abs4
#define __builtin_abs2(x) __cv_v2s_of_u(__builtin_riscv_cv_simd_abs_h(__cv_u_of_v2s(x)))
#define __builtin_abs4(x) __cv_v4s_of_u(__builtin_riscv_cv_simd_abs_b(__cv_u_of_v4s(x)))

/* ---- Scalar ALU: cv.clip/clipu, cv.min/max --------------------------- */
#undef __builtin_clip
#undef __builtin_clipu
#define __builtin_clip(x, precision)  __builtin_riscv_cv_alu_clip ((int)(x),           (1  << (precision)) - 1)
#define __builtin_clipu(x, precision) __builtin_riscv_cv_alu_clipu((unsigned int)(x), (1u << (precision)) - 1u)

/* ---- Bit manipulation: cv.fl1/ff1/clb/cnt/ror ------------------------ */

#undef __builtin_fl1
#undef __builtin_clb
#undef __builtin_rotr
#define __builtin_fl1(x)  __builtin_riscv_cv_bitmanip_fl1((unsigned int)(x))
#define __builtin_clb(x)  ((unsigned int)__builtin_riscv_cv_bitmanip_clb((unsigned int)(x)))
/* pulp's rotr is a rotate by one, cv.ror takes the amount in a register. */
#define __builtin_rotr(x) __builtin_riscv_cv_bitmanip_ror((unsigned int)(x), 1)


/* ---- The archi-level __XXX aliases ----------------------------------- */

#undef __MAX2
#undef __MAX4
#undef __MAXU2
#undef __MAXU4
#undef __MIN2
#undef __MIN4
#undef __MINU2
#undef __MINU4
#undef __ABS2
#undef __ABS4
#undef __PACK2
#undef __PACKU2
#undef __PACK4
#undef __PACKU4
#define __PACK2(x, y)           __builtin_pack2((x), (y))
#define __PACKU2(x, y)          __builtin_packu2((x), (y))
#define __PACK4(x, y, z, t)     __builtin_pack4((x), (y), (z), (t))
#define __PACKU4(x, y, z, t)    __builtin_packu4((x), (y), (z), (t))
#define __MAX2(x, y)  __builtin_max2((x), (y))
#define __MAX4(x, y)  __builtin_max4((x), (y))
#define __MAXU2(x, y) __builtin_maxu2((x), (y))
#define __MAXU4(x, y) __builtin_maxu4((x), (y))
#define __MIN2(x, y)  __builtin_min2((x), (y))
#define __MIN4(x, y)  __builtin_min4((x), (y))
#define __MINU2(x, y) __builtin_minu2((x), (y))
#define __MINU4(x, y) __builtin_minu4((x), (y))
#define __ABS2(x)     __builtin_abs2((x))
#define __ABS4(x)     __builtin_abs4((x))

#undef __MAX
#undef __CLIP
#undef __CLIPU
#define __MAX(a, b) __builtin_riscv_cv_alu_max((int)(a), (int)(b))
#define __CLIP(x, precision)  __builtin_clip((x), (precision))
#define __CLIPU(x, precision) __builtin_clipu((x), (precision))

#undef __FL1
#undef __CLB
#undef __FF1
#undef __ROTR
#define __FL1(x)  __builtin_fl1((x))
#define __CLB(x)  __builtin_clb((x))
/* builtins_v2_emu.h has no __FF1 at all, so this also fills that gap. */
#define __FF1(x)  __builtin_riscv_cv_bitmanip_ff1((unsigned int)(x))
#define __ROTR(x) __builtin_rotr((x))

#endif /* __cv32e40p__ */

#endif
