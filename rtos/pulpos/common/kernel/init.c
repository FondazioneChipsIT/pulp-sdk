/*
 * Copyright (C) 2019 GreenWaves Technologies
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
 * Authors: Germain Haugou, GreenWaves Technologies (germain.haugou@greenwaves-technologies.com)
 */

#include "pmsis.h"

#include "io.h"

volatile void *cluster_entry;
PI_L1 char *cluster_stacks;

static volatile int cluster_running;
static volatile int cluster_retval;

struct pi_device cluster_dev;

typedef void (*fptr)(void);

static fptr ctor_list[1] __attribute__((section(".ctors.start"))) = { (fptr) -1 };
static fptr dtor_list[1] __attribute__((section(".dtors.start"))) = { (fptr) -1 };


static void pos_init_do_ctors(void)
{
    fptr *fpp;

    for(fpp = ctor_list+1;  *fpp != 0;  ++fpp)
    {
        (**fpp)();
    }
}



static void pos_init_do_dtors(void)
{
    fptr *fpp;
    for(fpp = dtor_list + 1;  *fpp != 0;  ++fpp)
    {
        (**fpp)();
    }
}

static void cluster_core_init()
{
    eu_evt_maskSet((1<<PULP_DISPATCH_EVENT) | (1<<PULP_MUTEX_EVENT) | (1<<PULP_HW_BAR_EVENT));
#ifdef ARCHI_HMR
    // Enable resynch and synch requests
    eu_irq_maskSet(1<<24 | 1<<23);
    rt_irq_set_handler(24, pos_hmr_tmr_irq);
    rt_irq_set_handler(23, pos_hmr_synch);
    hal_spr_write(0x304, 1<<24|1<<23);
    hal_irq_enable();

    eu_bar_setup(eu_bar_addr(0), hmr_get_active_cores(0));
#else
    eu_bar_setup(eu_bar_addr(0), (1<<ARCHI_CLUSTER_NB_PE) - 1);
#endif
}


extern int main(int argc, const char * const argv[]);

void cluster_entry_stub()
{
    cluster_core_init();

    pi_cl_team_barrier();
    int retval = ((int (*)())cluster_entry)();
    pi_cl_team_barrier();

    if (hal_core_id() == 0)
    {
        cluster_retval = retval;
        cluster_running = 0;
        #ifdef ARCHI_NO_FC
        hal_cluster_ctrl_return_set(hal_cluster_id(), cluster_retval);
        hal_cluster_ctrl_eoc_set(1);
        exit(cluster_retval);
        #endif
    }

    eu_evt_maskClr(0xffffffff);
    eu_evt_wait();
    while(1);
}

void cluster_start(int cid, int (*entry)())
{
    // Store cluster entry point, ctr0 will jump here
    cluster_entry = entry;

    // Init FLL
    #ifndef ARCHI_NO_FC
    pos_fll_init(POS_FLL_CL);
    #endif
    // Initialize cluster L1 memory allocator
    pos_alloc_init_l1(cid);

    // Activate icache ---> TEMPORARY: UNTIL WE DECIDE WHICH ICACHE TO USE
    hal_icache_cluster_enable(cid);

    cluster_stacks = pi_l1_malloc(&cluster_dev, ARCHI_CLUSTER_NB_PE*CLUSTER_STACK_SIZE);
    if (cluster_stacks == NULL)
        return;
    cluster_running = 1;

    eoc_fetch_enable_remote(cid, (1<<ARCHI_CLUSTER_NB_PE) - 1);
}

void pos_init_start()
{
#if PULP_CHIP_FAMILY == CHIP_GAP
  // Always allow JTAG accesses for now as security is not implemented
  hal_pmu_bypass_set (ARCHI_REG_FIELD_SET (hal_pmu_bypass_get (), 1, 11, 1) );
#endif

  INIT_TRACE(POS_LOG_INFO, "Starting runtime initialization\n");

  pos_irq_init();

  pos_soc_init();

  #ifdef ARCHI_NO_FC
  pos_soc_event_init();
  #endif

  // Initialize first the memory allocators and the utils so that they are
  // available for constructors, especially to let them declare
  // callbacks
  //__rt_utils_init();
  pos_allocs_init();

  // Scheduler is initialized now to let other modules use it early
#ifndef ARCHI_NO_FC
  pos_sched_init();
#endif
  // Call global and static constructors
  // Each module may do private initializations there
  pos_init_do_ctors();

  // Now that the system is ready, activate IO
  pos_io_start();

  // Now now the minimal init are done, we can activate interruptions
  hal_irq_enable();

#ifdef ARCHI_NO_FC
  cluster_start(hal_cluster_id(), main);
#endif
}


void pos_init_stop()
{
    // Close IO to flush them
    pos_io_stop();

    /* Call global and static destructors */
    pos_init_do_dtors();
}
