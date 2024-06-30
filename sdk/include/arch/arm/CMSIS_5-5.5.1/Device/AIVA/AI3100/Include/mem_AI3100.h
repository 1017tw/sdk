/**************************************************************************//**
 * @file     mem_<Device>.h
 * @brief    CMSIS Cortex-A Memory base and size definitions (used in scatter file)
 * @version  V1.00
 * @date     10. January 2018
 ******************************************************************************/
/*
 * Copyright (c) 2009-2018 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MEM_AI3100_H   /* ToDo: replace '<Device>' with your device name */
#define MEM_AI3100_H

#define SRAM0_BASE_ADDR                         (0x10000000)
#define SRAM0_SIZE                              (256 * 1024)

/*
 * SRAM0: 0x1000_0000 ~ 0x1003_FFFF (256KB)
 */

#define __SRAM_BASE                             SRAM0_BASE_ADDR
#define __SRAM_SIZE                             SRAM0_SIZE

/* TTB L1: 1M entry, 4096 entries, 4 bytes for each entry */
#define __TTB_BASE                              SRAM0_BASE_ADDR
#define __TTB_SIZE                              (16 * 1024)         /* 16KB */

/* TTB L2: 4K page, 256 entries, 4 bytes for each entry */
#define __TABLE_L2_4K_SIZE                      (0x00000400)        /* 1KB  */


/* Device registers space */
#define __PERIPHERAL_REG_BASE_ADDR              (0x20000000) 
#define __PERIPHERAL_REG_SIZE                   (0x00400000)

/* XIP flash */
#define __XIP_FLASH_BASE                        (0x30000000)
#define __XIP_FLASH_SIZE                        (16 * 1024 * 1024)
#define __XIP_ADDR_TRANS(addr)                  ((uintptr_t)(addr + __XIP_FLASH_BASE))

/* Stack will be placed into SRAM */
#define __UND_STACK_SIZE                        (0) 
#define __ABT_STACK_SIZE                        (1024)
#define __SVC_STACK_SIZE                        (8*1024)
#define __IRQ_STACK_SIZE                        (8*1024)
#define __FIQ_STACK_SIZE                        (0)
#define __SYS_STACK_SIZE                        (8*1024)

#define __DMA_ALIGNED__(n) __attribute__ ((__aligned__(n), section(".dma")))

#endif /* MEM_<Device>_H */
