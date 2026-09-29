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

#ifndef ARCHI_MAILBOXES_H
#define ARCHI_MAILBOXES_H

#define ARCHI_MAILBOX_IRQ_SND_STAT_OFFSET   0x0
#define ARCHI_MAILBOX_IRQ_SND_SET_OFFSET    0x4
#define ARCHI_MAILBOX_IRQ_SND_CLR_OFFSET    0x8
#define ARCHI_MAILBOX_IRQ_SND_EN_OFFSET     0xC
#define ARCHI_MAILBOX_IRQ_RCV_STAT_OFFSET   0x40
#define ARCHI_MAILBOX_IRQ_RCV_SET_OFFSET    0x44
#define ARCHI_MAILBOX_IRQ_RCV_CLR_OFFSET    0x48
#define ARCHI_MAILBOX_IRQ_RCV_EN_OFFSET     0x4C
#define ARCHI_MAILBOX_LETTER0_OFFSET        0x80
#define ARCHI_MAILBOX_LETTER1_OFFSET        0x84

// Cluster runtime entry-control protocol carried in mailbox letters.
// LETTER0 selects whether the current entry point is kept or LETTER1 provides a
// replacement entry point for the next cluster wakeup.
#define ARCHI_MAILBOX_ENTRY_KEEP            0x0
#define ARCHI_MAILBOX_ENTRY_LOAD            0x1

#endif
