#include "pulp.h"

extern int __real_main(void);

#if defined(ARCHI_NO_FC) && defined(ARCHI_HAS_CLUSTER)

static int compat_main_ret;
static int (*compat_entry)(void) = __real_main;

static void compat_pe_entry(void *arg)
{
    int ret = compat_entry();
    if (pi_core_id() == 0)
        compat_main_ret = ret;
}

static void compat_cluster_entry(void *arg)
{
    pi_cl_team_fork(pi_cl_cluster_nb_cores(), compat_pe_entry, NULL);
}

// pulp-runtime's crt0 runs main on every PE. Reproduce that with the SDK's
// standalone entry: send_task allocates the task stacks and pushes
// pos_set_slave_stack to the dispatcher, which forking directly would skip.
int __wrap_main(void)
{
    struct pi_device cluster_dev;
    struct pi_cluster_conf conf;
    struct pi_cluster_task task;

    pi_cluster_conf_init(&conf);
    conf.id = 0;
    pi_open_from_conf(&cluster_dev, &conf);

    if (pi_cluster_open(&cluster_dev))
        return -1;

#ifdef ARCHI_HAS_MAILBOXES
    // Persistent worker, same protocol as pulp-runtime cluster_wait_entry()
    while (1)
    {
        // Other events in core 0's mask can wake it: wait for the mailbox one
        while (!(eu_evt_maskWaitAndClr(1 << ARCHI_CL_EVT_MBOX) & (1 << ARCHI_CL_EVT_MBOX)));
        hal_mailboxes_clear_receive_irq();
        // Ibex drives the irq as an edge: drop the event it re-latched
        eu_evt_clr(1 << ARCHI_CL_EVT_MBOX);

        if (hal_mailboxes_read_letter0() == ARCHI_MAILBOX_ENTRY_LOAD)
            compat_entry = (int (*)(void))hal_mailboxes_read_letter1();

        pi_cluster_task(&task, compat_cluster_entry, NULL);
        pi_cluster_send_task_to_cl(&cluster_dev, &task);

        hal_cluster_ctrl_return_set(0, compat_main_ret);
        hal_cluster_ctrl_eoc_set(1);
        hal_mailboxes_write_return_value(compat_main_ret);
        hal_mailboxes_ring_doorbell();
    }
#else
    pi_cluster_task(&task, compat_cluster_entry, NULL);
    pi_cluster_send_task_to_cl(&cluster_dev, &task);
    pi_cluster_close(&cluster_dev);

    return compat_main_ret;
#endif
}

#else

int __wrap_main(void)
{
    return __real_main();
}

#endif
