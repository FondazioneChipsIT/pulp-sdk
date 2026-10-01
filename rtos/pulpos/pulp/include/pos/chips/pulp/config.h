/*
 * Copyright (C) 2020 ETH Zurich
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
 * Authors: Germain Haugou, ETH Zurich (germain.haugou@iis.ee.ethz.ch)
 */

#ifndef __POS__CHIPS__PULP__CONFIG_H__
#define __POS__CHIPS__PULP__CONFIG_H__

#include "archi/pulp_defs.h"

#define PULP_CHIP CHIP_PULP
#define PULP_CHIP_FAMILY CHIP_PULP
#define CONFIG_PULP 1
#define PULP_CHIP_STR pulp
#define PULP_CHIP_FAMILY_STR pulp

/* Neither provider: builtins_v2_emu.h then supplies the generic forms. */
#if defined(PULP_NO_XPULP)
#elif defined(__cv32e40p__)
#define ARCHI_CORE_HAS_COREV_V2 1
#else
#define ARCHI_CORE_HAS_PULPV2 1
#endif

#define ARCHI_CORE_HAS_1_10 1

#endif
