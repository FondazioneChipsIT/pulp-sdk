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

#ifndef __ARCHI_RISCV_BUILTINS_PULP_COMPAT_H__
#define __ARCHI_RISCV_BUILTINS_PULP_COMPAT_H__

#ifdef __cv32e40p__

#define __CV_ASU(x) (((union { __typeof__(x) __v; unsigned int __u; }){ .__v = (x) }).__u)


/* ---- Accumulating dot products: cv.sdotsp.b / cv.sdotusp.b ------------ */

#define __builtin_pulp_sdotsp4(a, b, c) \
  ((int)__builtin_riscv_cv_simd_sdotsp_b(__CV_ASU(a), __CV_ASU(b), (unsigned int)(c)))
#define __builtin_pulp_sdotusp4(a, b, c) \
  ((int)__builtin_riscv_cv_simd_sdotusp_b(__CV_ASU(a), __CV_ASU(b), (unsigned int)(c)))
#define __builtin_pulp_sdotup4(a, b, c) \
  ((unsigned int)__builtin_riscv_cv_simd_sdotup_b(__CV_ASU(a), __CV_ASU(b), (unsigned int)(c)))


/* ---- SIMD min/max/avg on bytes: cv.{min,max,minu,maxu,avgu}.b -------- */

#define __builtin_pulp_max4(a, b)  __cv_v4s_of_u(__builtin_riscv_cv_simd_max_b (__CV_ASU(a), __CV_ASU(b)))
#define __builtin_pulp_maxu4(a, b) __cv_v4u_of_u(__builtin_riscv_cv_simd_maxu_b(__CV_ASU(a), __CV_ASU(b)))
#define __builtin_pulp_min4(a, b)  __cv_v4s_of_u(__builtin_riscv_cv_simd_min_b (__CV_ASU(a), __CV_ASU(b)))
#define __builtin_pulp_minu4(a, b) __cv_v4u_of_u(__builtin_riscv_cv_simd_minu_b(__CV_ASU(a), __CV_ASU(b)))
#define __builtin_pulp_avgu4(a, b) __cv_v4u_of_u(__builtin_riscv_cv_simd_avgu_b(__CV_ASU(a), __CV_ASU(b)))


/* ---- Scalar min/max: cv.min / cv.max / cv.minu / cv.maxu ------------- */

#define __builtin_pulp_maxsi(a, b)  __builtin_riscv_cv_alu_max ((int)(a), (int)(b))
#define __builtin_pulp_minsi(a, b)  __builtin_riscv_cv_alu_min ((int)(a), (int)(b))
#define __builtin_pulp_maxusi(a, b) __builtin_riscv_cv_alu_maxu((unsigned int)(a), (unsigned int)(b))
#define __builtin_pulp_minusi(a, b) __builtin_riscv_cv_alu_minu((unsigned int)(a), (unsigned int)(b))


/* ---- Saturation: cv.clip / cv.clipu ---------------------------------- */


#define __builtin_pulp_clip_r(x, bound)  __builtin_riscv_cv_alu_clip ((int)(x), (bound))
#define __builtin_pulp_clipu_r(x, bound) __builtin_riscv_cv_alu_clipu((unsigned int)(x), (bound))


/* ---- Bit manipulation ------------------------------------------------- */

#define __builtin_pulp_fl1(x) __builtin_riscv_cv_bitmanip_fl1((unsigned int)(x))

#define __builtin_pulp_bextract(x, size, off) \
  ((int)(((int)((unsigned int)(x) << (32 - (size) - (off)))) >> (32 - (size))))
#define __builtin_pulp_bextractu(x, size, off) \
  ((unsigned int)(((unsigned int)(x) >> (off)) & ((1u << (size)) - 1u)))

#define __builtin_pulp_binsert(dst, not_mask_imm, src, mask_imm, off) \
  ((((unsigned int)(dst)) & ((unsigned int)(not_mask_imm))) | \
   ((((unsigned int)(src)) << (off)) & ((unsigned int)(mask_imm))))


/* ---- Byte pack: cv.packlo.b + cv.packhi.b ---------------------------- */

/* builtins_cv32e40p.h already chains the two half-packs; reuse it. */
#define __builtin_pulp_pack4(x, y, z, t) __builtin_pack4((x), (y), (z), (t))


/*
 * Deliberately NOT defined, so a build that needs them fails loudly instead of
 * silently losing precision or performance:
 *
 * - The 4-bit and 2-bit packed SIMD forms: sdotsp8/sdotusp8/sdotup8,
 *   sdotsp16/sdotusp16/sdotup16, max8/maxu8/max16/maxu16, min8/minu8/
 *   min16/minu16, avgu8/avgu16. These are XpulpNN instructions; CV32E40P
 *   implements only the 8-bit and 16-bit element widths (Xcvsimd), so there is
 *   no hardware and no intrinsic to map onto.
 * - The MacLoad family: mlinitspr_v3, mlupdatespr_v3, mlassignspr_v3,
 *   mlsdotsup{4,8,16}_v3, mlsdotsp{4,8,16}_v3. These drive the XpulpNN
 *   load-and-MAC SPRs, which CV32E40P does not have.
 *
 * All of the above are referenced only by macro bodies in pulp-nn-mixed's
 * pulp_nn_utils.h that the XpulpV2/32bit sources never expand (verified: zero
 * uses across those 750 files), and an unexpanded macro body naming an
 * undefined identifier is harmless.
 *
 * Also absent: the fixed-point mulsN/macsN family and scalar abs/addN/subN --
 * see the note at the bottom of builtins_cv32e40p.h. corev.def exposes only
 * cv_mac_mac and cv_mac_msu.
 */

#endif /* __cv32e40p__ */

#endif
