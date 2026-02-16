/*
 * Copyright (C) 2017 ETH Zurich, University of Bologna and GreenWaves Technologies
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 * Authors: Germain Haugou, ETH (germain.haugou@iis.ee.ethz.ch)
 */

#include "pmsis.h"
#include <stdio.h>

int main()
{
    if (pi_core_id() == 0) {
        printf("Hello from cluster \n");
    }

    *(int*)(0x10000000)=0xABBAABBA;
    return 0;
}

