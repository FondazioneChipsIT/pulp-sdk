/*
 * Copyright (C) 2020 ETH Zurich and University of Bologna
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
 * Authors: Nazareno Bruschi, Unibo (nazareno.bruschi@unibo.it)
 */

#include "pmsis.h"

static PI_L2 pos_hyper_t pos_hyper[HYPER_NB_CS * ARCHI_UDMA_NB_HYPER];
static PI_L2 int hyper_open_count=0;
static PI_L2 int hyper_channel;
static PI_L2 pos_udma_channel_t hyper_tx_channel;
static PI_L2 pos_udma_channel_t hyper_rx_channel;

static void pos_hyper_setup(pos_hyper_t *hyper)
{
  if (hyper->type == PI_HYPER_TYPE_FLASH)
  {
    plp_hyper_setup(hyper->hyper_id, CONFIG_HYPERFLASH_EN_LATENCY_ADD, HYPER_MEM_SEL_FLASH, CONFIG_HYPERFLASH_T_LATENCY_ACCESS);
    plp_hyper_set_reg(UDMA_HYPER_BASE_ADDR(0) + REG_PAGE_BOUND, 0x04);
  }
  else if (hyper->type == PI_HYPER_TYPE_RAM)
  {
    plp_hyper_setup(hyper->hyper_id, CONFIG_HYPERRAM_EN_LATENCY_ADD, HYPER_MEM_SEL_RAM, CONFIG_HYPERRAM_T_LATENCY_ACCESS);
    plp_hyper_set_reg(UDMA_HYPER_BASE_ADDR(0) + REG_PAGE_BOUND, 0x03);
  }
}

static void pos_hyper_wait_done(pos_hyper_t *hyper)
{
  while (plp_udma_busy(UDMA_HYPER_TX_ADDR(hyper->hyper_id)) & plp_udma_busy(UDMA_HYPER_RX_ADDR(hyper->hyper_id)))
  {
    pos_irq_wait_for_interrupt();
  }

  for(int i=0; i<HYPER_NB_CHANNELS; i++)
  {
    plp_hyper_wait(hyper->hyper_id, i);
  }
}

// A copy keeps all its parameters in its task until it starts:
// data[0] L2 address, data[1] size, data[2] HyperBus address, data[3] flags,
// data[4] 2D length, data[5] 2D stride, data[6] pos_hyper_t.
#define POS_HYPER_RX      (1 << 0)
#define POS_HYPER_TWD_EXT (1 << 1) // 2D on the HyperBus side
#define POS_HYPER_TWD_L2  (1 << 2) // 2D on the L2 side

// Allocate the transaction ID and program its registers only when the copy
// starts. TRANS_ID_ALLOC returns the smallest ID with no transfer running, so
// an ID taken for a copy that is still queued would be handed out again, and
// the next copy would overwrite the queued copy's address and 2D registers.
static void pos_hyper_launch(pi_task_t *task)
{
  pos_hyper_t *hyper = (pos_hyper_t *)task->data[6];
  uint32_t flags = task->data[3];
  int rx = flags & POS_HYPER_RX;

  unsigned int twd_cmd[HYPER_NB_TWD_REGS] = {
    (flags & POS_HYPER_TWD_EXT) ? 1 : 0, task->data[4], task->data[5],
    (flags & POS_HYPER_TWD_L2) ? 1 : 0, task->data[4], task->data[5]};
  unsigned int ctl_cmd[HYPER_NB_CTL_REGS] = {rx ? 0x5 : 0x1, task->data[2]};

  pos_hyper_setup(hyper);

  int tran_id = plp_hyper_id_alloc(hyper->hyper_id);
  hyper->tran_id = tran_id;

  plp_hyper_set_twd_param(hyper->hyper_id, twd_cmd, tran_id);
  plp_hyper_set_ctl_param(hyper->hyper_id, ctl_cmd, tran_id);

  uint32_t channel = UDMA_HYPER_BASE_ADDR(hyper->hyper_id) +
    (rx ? UDMA_HYPER_CHANNEL_RX(tran_id) : UDMA_HYPER_CHANNEL_TX(tran_id));
  plp_hyper_enqueue(channel, task->data[0], task->data[1], UDMA_CHANNEL_CFG_EN | UDMA_CHANNEL_CFG_SIZE_8);
}

// One copy in flight in total, reads and writes share this queue: the
// end-of-transfer event (RX or TX) then always belongs to pendings[0], and a
// read and a write never share a transaction ID. Each ID has one set of 2D and
// command registers, so a write kicked while a read runs on the same ID breaks
// the queued transfer (seen on GVSoC with many small double-buffered tiles).
// The HyperBus serves one transfer at a time anyway. Later copies wait in a FIFO.
static void pos_hyper_enqueue(pos_udma_channel_t *channel, pi_task_t *task)
{
  int irq = hal_irq_disable();

  if (channel->pendings[0] == NULL)
  {
    channel->pendings[0] = task;
    pos_hyper_launch(task);
  }
  else
  {
    task->next = NULL;
    if (channel->waitings_first == NULL)
      channel->waitings_first = task;
    else
      channel->waitings_last->next = task;
    channel->waitings_last = task;
  }

  hal_irq_restore(irq);
}

void pos_hyper_handle_copy(int event, void *arg)
{
    pos_udma_channel_t *channel = arg;

    pi_task_t *done = channel->pendings[0];
    pi_task_t *next = channel->waitings_first;

    channel->pendings[0] = next;
    if (next)
    {
        channel->waitings_first = next->next;
        pos_hyper_launch(next);
    }

    pos_task_push_locked(done);
}

static void pos_hyper_copy_async(struct pi_device *device, uint32_t flags, uint32_t hyper_addr, void *addr,
                                 uint32_t size, uint32_t length, uint32_t stride, struct pi_task *task)
{
  pos_hyper_t *hyper = (pos_hyper_t *)device->data;

  task->data[0] = (uint32_t)addr;
  task->data[1] = size;
  task->data[2] = hyper_addr;
  task->data[3] = flags;
  task->data[4] = length;
  task->data[5] = stride;
  task->data[6] = (uint32_t)hyper;

  pos_hyper_enqueue(hyper->rx_channel, task);
}

void pos_hyper_create_channel(pos_udma_channel_t *channel, int channel_id, int soc_event)
{
    pos_soc_event_register_callback(soc_event, pos_hyper_handle_copy, (void *)channel);
    channel->pendings[0] = NULL;
    channel->pendings[1] = NULL;
    channel->waitings_first = NULL;
    channel->base = hal_udma_channel_base(channel_id);
}

void pi_hyper_conf_init(struct pi_hyper_conf *conf)
{
  conf->id = 0;
}

int32_t pi_hyper_open(struct pi_device *device)
{
  int irq = hal_irq_disable();

  struct pi_hyper_conf *conf = (struct pi_hyper_conf *)device->config;

  unsigned char hyper_id = conf->id;
  int periph_id = ARCHI_UDMA_HYPER_ID(hyper_id);
  hyper_channel = UDMA_EVENT_ID(periph_id);
  int cs = conf->cs;

  pos_hyper_t *hyper = &pos_hyper[hyper_id + cs];

  hyper->hyper_id = conf->id;
  hyper->type = conf->type;
  hyper->cs = cs;
  hyper->tran_id = 0;

  hyper->rx_channel = &hyper_rx_channel;
  hyper->tx_channel = &hyper_tx_channel;

  device->data = (void *)hyper;

  if (hyper_open_count == 0)
  {
    pos_hyper_create_channel(hyper->rx_channel, UDMA_CHANNEL_ID(periph_id), hyper_channel + ARCHI_UDMA_HYPER_EOT_RX_EVT);
    pos_hyper_create_channel(hyper->tx_channel, UDMA_CHANNEL_ID(periph_id)+1, hyper_channel + ARCHI_UDMA_HYPER_EOT_TX_EVT);
    // Writes complete through the same shared queue as reads
    pos_soc_event_register_callback(hyper_channel + ARCHI_UDMA_HYPER_EOT_TX_EVT, pos_hyper_handle_copy, (void *)hyper->rx_channel);
  }

  hyper_open_count++;

  plp_udma_cg_set(plp_udma_cg_get() | (1<<periph_id));

  soc_eu_fcEventMask_setEvent(hyper_channel+ ARCHI_UDMA_HYPER_EOT_RX_EVT);
  soc_eu_fcEventMask_setEvent(hyper_channel + ARCHI_UDMA_HYPER_EOT_TX_EVT);

  hal_irq_restore(irq);

  return 0;
}

void pi_hyper_close(struct pi_device *device)
{
  int irq = hal_irq_disable();

  pos_hyper_t *hyper = (pos_hyper_t *)device->data;

  struct pi_hyper_conf *conf = (struct pi_hyper_conf *)device->config;
  unsigned char hyper_id = conf->id;
  int periph_id = ARCHI_UDMA_HYPER_ID(hyper_id);

  hyper_open_count--;

  if (hyper_open_count == 0)
  {
    pos_hyper_wait_done(hyper);
    plp_udma_cg_set(plp_udma_cg_get() & ~(1<<periph_id));
  }

  hal_irq_restore(irq);
}

int pi_hyper_ioctl(struct pi_device *device, uint32_t cmd, void *arg)
{
  switch (cmd)
  {
    case PI_HYPER_IOCTL_SET_TRAN_ID:
    {
      uint32_t tran_id = *(uint32_t *)arg;
      if (tran_id >= HYPER_NB_CHANNELS)
        return -1;
      else
      {
        pos_hyper_t *hyper = (pos_hyper_t *)device->data;
        hyper->tran_id = tran_id;
      }
      break;
    }
  }
  return 0;
}

void pi_hyper_set_regs(struct pi_device *device, uint32_t cmd, void *arg)
{
  pos_hyper_t *hyper = (pos_hyper_t *)device->data;
  uint32_t offset = UDMA_HYPER_BASE_ADDR(hyper->hyper_id);

  switch (cmd)
  {
    case PI_HYPER_T_LATENCY_ACC:
      offset += REG_T_LATENCY_ACCESS;
      break;
    case PI_HYPER_EN_LATENCY_ADD:
      offset += REG_EN_LATENCY_ADD;
      break;
    case PI_HYPER_T_CS_MAX:
      offset += REG_T_CS_MAX;   
      break;
    case PI_HYPER_MEM_SEL:
      offset += MEM_SEL;
      break;
    default:
      break;
  }

  switch(cmd)
  {
    case PI_HYPER_CFG:
      plp_hyper_set_cfg(hyper->hyper_id, *(unsigned short *)arg, hyper->tran_id);
      break;
    case PI_HYPER_TWD:
      plp_hyper_set_twd_param(hyper->hyper_id, (unsigned int *)arg, hyper->tran_id);
      break;
    case PI_HYPER_PAGEBOUND:
      plp_hyper_set_pagebound(hyper->hyper_id, *(unsigned int *)arg);
      break;
    case PI_HYPER_CTL:
      plp_hyper_set_ctl_param(hyper->hyper_id, (unsigned int *)arg, hyper->tran_id);
      break;
    default:
      plp_hyper_set_reg(offset, *(unsigned int *)arg);    
      break;
  }
}

int pi_hyper_ctl_status_regs(struct pi_device *device, uint32_t cmd, void *arg)
{
  pos_hyper_t *hyper = (pos_hyper_t *)device->data;

  if (arg != NULL)
  {
    hyper->tran_id = plp_hyper_id_alloc(hyper->hyper_id);

    pi_hyper_write(device, cmd, arg, 4);
    return 0;
  }
  else
  {
    int value;
    pi_hyper_read(device, cmd, &value, 4);
    return value;
  }
  return 0;
}

void pi_hyper_read(struct pi_device *device, uint32_t hyper_addr, void *addr, uint32_t size)
{
  pi_task_t task;
  pi_hyper_read_async(device, hyper_addr, addr, size, pi_task_block(&task));
  pi_task_wait_on(&task);
}

void pi_hyper_read_async(struct pi_device *device, uint32_t hyper_addr, void *addr, uint32_t size, struct pi_task *task)
{
  pos_hyper_copy_async(device, POS_HYPER_RX, hyper_addr, addr, size, 0, 0, task);
}

void pi_hyper_read_2d(struct pi_device *device, uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride, uint32_t length)
{
  pi_task_t task;
  pi_hyper_read_2d_async(device, hyper_addr, addr, size, stride, length, pi_task_block(&task));
  pi_task_wait_on(&task);  
}

void pi_hyper_read_2d_async(struct pi_device *device, uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride, uint32_t length, struct pi_task *task)
{
  pos_hyper_copy_async(device, POS_HYPER_RX | POS_HYPER_TWD_EXT, hyper_addr, addr, size, length, stride, task);
}

void pi_hyper_read_bi2d_async(struct pi_device *device, uint32_t hyper_addr, void *addr, uint32_t size, uint32_t dir, uint32_t stride, uint32_t length, struct pi_task *task)
{
  uint32_t twd = (dir & 0x1) ? POS_HYPER_TWD_L2 : POS_HYPER_TWD_EXT;
  pos_hyper_copy_async(device, POS_HYPER_RX | twd, hyper_addr, addr, size, length, stride, task);
}

void pi_hyper_write(struct pi_device *device, uint32_t hyper_addr, void *addr, uint32_t size)
{
  pos_hyper_t *hyper = (pos_hyper_t *)device->data;

  unsigned int twd_cmd[HYPER_NB_TWD_REGS] = {0,0,0,0,0,0};
  unsigned int ctl_cmd[HYPER_NB_CTL_REGS] = {0x0, hyper_addr};

  pos_hyper_setup(hyper);

  while(plp_hyper_nb_tran(hyper->hyper_id, hyper->tran_id)>HYPER_FIFO_DEPTH-1){}

  pi_hyper_set_regs(device, PI_HYPER_TWD, (unsigned int *)twd_cmd);
  pi_hyper_set_regs(device, PI_HYPER_CTL, (unsigned int *)ctl_cmd);
  pi_hyper_set_regs(device, PI_HYPER_CFG, (unsigned short *)addr);

  plp_hyper_enqueue(UDMA_HYPER_BASE_ADDR(hyper->hyper_id) + UDMA_HYPER_CHANNEL_TX(hyper->tran_id), 0x0, 0x0, UDMA_CHANNEL_CFG_EN | UDMA_CHANNEL_CFG_SIZE_8);
}

void pi_hyper_write_async(struct pi_device *device, uint32_t hyper_addr, void *addr, uint32_t size, struct pi_task *task)
{
  pos_hyper_copy_async(device, 0, hyper_addr, addr, size, 0, 0, task);
}

void pi_hyper_write_2d(struct pi_device *device, uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride, uint32_t length)
{
  pi_task_t task;
  pi_hyper_write_2d_async(device, hyper_addr, addr, size, stride, length, pi_task_block(&task));
  pi_task_wait_on(&task); 
}

void pi_hyper_write_2d_async(struct pi_device *device, uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride, uint32_t length, struct pi_task *task)
{
  pos_hyper_copy_async(device, POS_HYPER_TWD_EXT, hyper_addr, addr, size, length, stride, task);
}

void pi_hyper_write_bi2d_async(struct pi_device *device, uint32_t hyper_addr, void *addr, uint32_t size, uint32_t dir, uint32_t stride, uint32_t length, struct pi_task *task)
{
  uint32_t twd = (dir & 0x1) ? POS_HYPER_TWD_L2 : POS_HYPER_TWD_EXT;
  pos_hyper_copy_async(device, twd, hyper_addr, addr, size, length, stride, task);
}

int pi_hyper_id_alloc(struct pi_device *device)
{
  pos_hyper_t *hyper = (pos_hyper_t *)device->data;  
  return plp_hyper_id_alloc(hyper->hyper_id);
}
