/*
 * Copyright (C) 2026 ETH Zurich, University of Bologna and Fondazione Chips-IT
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

#ifndef __HAL_MAILBOXES_H__
#define __HAL_MAILBOXES_H__


static inline void hal_write_to_mailbox(int offset, int value){
  pulp_write32(ARCHI_SOC_MAILBOXES_ADDR+ARCHI_SOC_MAILBOX_OFFSET+offset, value);
}

static inline int hal_read_from_mailbox(int offset) {
  return pulp_read32(ARCHI_SOC_MAILBOXES_ADDR + ARCHI_SOC_MAILBOX_OFFSET + offset);
}

static inline void hal_mailboxes_ring_doorbell() {
  hal_write_to_mailbox(ARCHI_MAILBOX_IRQ_SND_SET_OFFSET, 1);
  hal_write_to_mailbox(ARCHI_MAILBOX_IRQ_SND_EN_OFFSET, 1);
}

static inline void hal_mailboxes_write_return_value(int value) {
  hal_write_to_mailbox(ARCHI_MAILBOX_LETTER0_OFFSET, value);
}

static inline int hal_mailboxes_read_letter0(void) {
  return hal_read_from_mailbox(ARCHI_MAILBOX_LETTER0_OFFSET);
}

static inline int hal_mailboxes_read_letter1(void) {
  return hal_read_from_mailbox(ARCHI_MAILBOX_LETTER1_OFFSET);
}

static inline void hal_mailboxes_enable_receive_irq(void) {
  hal_write_to_mailbox(ARCHI_MAILBOX_IRQ_RCV_EN_OFFSET, 1);
}

static inline void hal_mailboxes_disable_receive_irq(void) {
  hal_write_to_mailbox(ARCHI_MAILBOX_IRQ_RCV_EN_OFFSET, 0);
}

static inline void hal_mailboxes_clear_receive_irq(void) {
  hal_write_to_mailbox(ARCHI_MAILBOX_IRQ_RCV_CLR_OFFSET, 1);
}

static inline int hal_mailboxes_receive_irq_pending(void) {
  return hal_read_from_mailbox(ARCHI_MAILBOX_IRQ_RCV_STAT_OFFSET);
}


#endif
