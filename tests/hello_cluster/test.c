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
    printf ("Hello from cluster %d | core %d \n", pi_cluster_id(), pi_core_id());
    pi_cl_team_barrier();
    return 0;
}

