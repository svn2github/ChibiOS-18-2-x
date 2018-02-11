/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio.

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    mmu.c
 * @brief   MMU code.
 *
 * @addtogroup MMU
 * @{
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "mmu.h"
#include "armparams.h"
#include "ARMCA5.h"
#if defined(__GNUC__) || defined(__DOXYGEN__)
#include "cmsis_gcc.h"
#else
#include "cmsis_armcc.h"
#endif
#include "ccportab.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/

/*
 * Use 1MB granularity. It best fits the SAMA5D2 memory layout.
 *
 * The MMU table contains 4096 entries.
 * Each entry in the mmu table (short descriptor, type 1MB section) is 32 bit wide (total 16kB),
 * and is structured in this way (with examples):
 *
 * +---------+--+-+--+-+-----+--------+-------+-+------+--+-+-+-+---+
 * |3       2|1 |1|1 |1|1    |1      1|1     1| |      |  | | | |   |
 * |1       0|9 |8|7 |6|5    |4      2|1     0|9|8    5|4 |3|2|1|0  |
 * +---------+--+-+--+-+-----+--------+-------+-+------+--+-+-+-+---+
 * | section |NS|0|nG|S|AP[2]|TEX[2:0]|AP[1:0]| |domain|XN|C|B|1|PXN|
 * +---------+--+-+--+-+-----+--------+-------+-+------+--+-+-+-+---+
 * |         |0 |0|0 |1|0    |111     |11     |0|1111  |0 |1|1|1|0  | == normal, cacheable
 * |         |0 |0|0 |1|0    |100     |11     |0|1111  |0 |0|0|1|0  | == normal, no-cacheable
 * |         |0 |0|0 |1|0    |000     |11     |0|1111  |0 |0|1|1|0  | == device
 * |         |0 |0|0 |1|0    |000     |11     |0|1111  |0 |0|0|1|0  | == strongly-ordered
 * |         |0 |0|0 |0|0    |000     |00     |0|0000  |0 |0|0|0|0  | == undefined
 *
 * Domains are 'manager'. Accesses are not checked against the permission bits in tlb.
 */
static uint32_t mmuTable[4096] CC_ALIGN(16384);

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Core/MMU Module initialization.
 * @note    This function is implicitly invoked on system initialization,
 *          there is no need to explicitly initialize the module.
 *
 * @notapi
 */
void __core_init(void) {
  uint32_t pm;

  /*
   * Invalidate L1 D Cache if it was disabled
   */
  pm = __get_SCTLR();
  if ((pm & SCTLR_C_Msk) == 0) {
    __L1C_CleanInvalidateCache(DCISW_INVALIDATE);
  }
  /*
   * Default, undefined regions
   */
  for (pm = 0; pm < 4096; ++pm)
    mmuTable[pm] = TTE_SECT_UNDEF;
  /*
   * ROM region
   *
   * 0x00000000
   */
  mmuTable[0]   = TTE_SECT_SECTION(0x00000000) |
                  TTE_SECT_MEM_NO_CACHEABLE |
                  TTE_SECT_RO_ACCESS |
                  TTE_SECT_DOM(0x0F) |
                  TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * NFC SRAM region
   *
   * 0x00100000
   */
  mmuTable[1]   = TTE_SECT_SECTION(0x00100000) |
                  TTE_SECT_DEVICE |
                  TTE_SECT_RW_ACCESS |
                  TTE_SECT_DOM(0x0F) |
                  TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * SRAM region
   *
   * 0x00200000
   */
  mmuTable[2]   = TTE_SECT_SECTION(0x00200000) |
                  TTE_SECT_MEM_CACHEABLE |
                  TTE_SECT_RW_ACCESS |
                  TTE_SECT_DOM(0x0F) |
                  TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * UDPHS RAM region
   *
   * 0x00300000
   */
  mmuTable[3]   = TTE_SECT_SECTION(0x00300000) |
                  TTE_SECT_DEVICE |
                  TTE_SECT_RW_ACCESS |
                  TTE_SECT_DOM(0x0F) |
                  TTE_SECT_EXE_NEVER |
                  TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * UHPHS region
   *
   * 0x00400000
   */
  mmuTable[4]   = TTE_SECT_SECTION(0x00400000) |
                  TTE_SECT_DEVICE |
                  TTE_SECT_RW_ACCESS |
                  TTE_SECT_DOM(0x0F) |
                  TTE_SECT_EXE_NEVER |
                  TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * UDPHS region
   *
   * 0x00500000
   */
  mmuTable[5]   = TTE_SECT_SECTION(0x00500000) |
                  TTE_SECT_DEVICE |
                  TTE_SECT_RW_ACCESS |
                  TTE_SECT_DOM(0x0F) |
                  TTE_SECT_EXE_NEVER |
                  TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * AXIMX region
   *
   * 0x00600000
   */
  mmuTable[6]   = TTE_SECT_SECTION(0x00600000) |
                  TTE_SECT_DEVICE |
                  TTE_SECT_RW_ACCESS |
                  TTE_SECT_DOM(0x0F) |
                  TTE_SECT_EXE_NEVER |
                  TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * DAP region
   *
   * 0x00700000
   */
  mmuTable[7]   = TTE_SECT_SECTION(0x00700000) |
                  TTE_SECT_DEVICE |
                  TTE_SECT_RW_ACCESS |
                  TTE_SECT_DOM(0x0F) |
                  TTE_SECT_EXE_NEVER |
                  TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * L2CC region, low
   *
   * 0x00a00000
   */
  mmuTable[0xa] = TTE_SECT_SECTION(0x00a00000) |
                  TTE_SECT_DEVICE |
                  TTE_SECT_RW_ACCESS |
                  TTE_SECT_DOM(0x0F) |
                  TTE_SECT_EXE_NEVER |
                  TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * L2CC region, hi
   *
   * 0x00b00000
   */
  mmuTable[0xb] = TTE_SECT_SECTION(0x00b00000) |
                  TTE_SECT_DEVICE |
                  TTE_SECT_RW_ACCESS |
                  TTE_SECT_DOM(0x0F) |
                  TTE_SECT_EXE_NEVER |
                  TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * EBI regions
   *
   * 0x10000000 - 0x1fffffff
   */
  for (pm = 0x100; pm < 0x200; pm++)
    mmuTable[pm] =  TTE_SECT_SECTION(pm << 20) |
                    TTE_SECT_MEM_STRONGLY_ORD |
                    TTE_SECT_RW_ACCESS |
                    TTE_SECT_DOM(0x0F) |
                    TTE_SECT_EXE_NEVER |
                    TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * DDR regions
   *
   * 0x20000000 - 0x3fffffff
   */
  for (pm = 0x200; pm < 0x400; pm++)
    mmuTable[pm] =  TTE_SECT_SECTION(pm << 20) |
                    TTE_SECT_MEM_CACHEABLE |
                    TTE_SECT_RW_ACCESS |
                    TTE_SECT_DOM(0x0F) |
                    TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * DDR AESB regions
   *
   * 0x40000000 - 0x5fffffff
   */
  for (pm = 0x400; pm < 0x600; pm++)
    mmuTable[pm] =  TTE_SECT_SECTION(pm << 20) |
                    TTE_SECT_MEM_CACHEABLE |
                    TTE_SECT_RW_ACCESS |
                    TTE_SECT_DOM(0x0F) |
                    TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * EBI 1, 2 and 3 regions
   *
   * 0x60000000 - 0x8fffffff
   */
  for (pm = 0x600; pm < 0x900; pm++)
    mmuTable[pm] =  TTE_SECT_SECTION(pm << 20) |
                    TTE_SECT_MEM_STRONGLY_ORD |
                    TTE_SECT_RW_ACCESS |
                    TTE_SECT_DOM(0x0F) |
                    TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * QSPI0/1 AESB MEM regions
   *
   * 0x90000000 - 0x9fffffff
   */
  for (pm = 0x900; pm < 0xa00; pm++)
    mmuTable[pm] =  TTE_SECT_SECTION(pm << 20) |
                    TTE_SECT_MEM_STRONGLY_ORD |
                    TTE_SECT_RW_ACCESS |
                    TTE_SECT_DOM(0x0F) |
                    TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * SDMMC0/1 regions
   *
   * 0xa0000000 - 0xbfffffff
   */
  for (pm = 0xa00; pm < 0xc00; pm++)
    mmuTable[pm] =  TTE_SECT_SECTION(pm << 20) |
                    TTE_SECT_MEM_STRONGLY_ORD |
                    TTE_SECT_RW_ACCESS |
                    TTE_SECT_DOM(0x0F) |
                    TTE_SECT_EXE_NEVER |
                    TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * NFC regions
   *
   * 0xc0000000 - 0xcfffffff
   */
  for (pm = 0xc00; pm < 0xd00; pm++)
    mmuTable[pm] =  TTE_SECT_SECTION(pm << 20) |
                    TTE_SECT_MEM_STRONGLY_ORD |
                    TTE_SECT_RW_ACCESS |
                    TTE_SECT_DOM(0x0F) |
                    TTE_SECT_EXE_NEVER |
                    TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * QSPI0/1 MEM regions
   *
   * 0xd0000000 - 0xdfffffff
   */
  for (pm = 0xd00; pm < 0xe00; pm++)
    mmuTable[pm] =  TTE_SECT_SECTION(pm << 20) |
                    TTE_SECT_MEM_STRONGLY_ORD |
                    TTE_SECT_RW_ACCESS |
                    TTE_SECT_DOM(0x0F) |
                    TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * Internal peripherals regions
   *
   * 0xf0000000
   * 0xf8000000
   * 0xfc000000
   */
  mmuTable[0xf00] = TTE_SECT_SECTION(0xf0000000) |
                  TTE_SECT_DEVICE |
                  TTE_SECT_RW_ACCESS |
                  TTE_SECT_DOM(0x0F) |
                  TTE_SECT_EXE_NEVER |
                  TTE_SECT_S | TTE_TYPE_SECT;
  mmuTable[0xf80] = TTE_SECT_SECTION(0xf8000000) |
                  TTE_SECT_DEVICE |
                  TTE_SECT_RW_ACCESS |
                  TTE_SECT_DOM(0x0F) |
                  TTE_SECT_EXE_NEVER |
                  TTE_SECT_S | TTE_TYPE_SECT;
  mmuTable[0xfc0] = TTE_SECT_SECTION(0xfc000000) |
                  TTE_SECT_DEVICE |
                  TTE_SECT_RW_ACCESS |
                  TTE_SECT_DOM(0x0F) |
                  TTE_SECT_EXE_NEVER |
                  TTE_SECT_S | TTE_TYPE_SECT;
  /*
   * Invalidate TLB and L1 I cache
   * Enable caches and MMU
   */
  MMU_InvalidateTLB();
  __set_TTBR0((uint32_t)mmuTable|0x5B);
  __set_DACR(0xC0000000);
  __DSB();
  __ISB();

  /*
   * L1 I cache invalidate and enable
   */
  pm = __get_SCTLR();
  if ((pm & SCTLR_I_Msk) == 0) {
    __set_ICIALLU(0);
    __set_SCTLR(pm | SCTLR_I_Msk);
  }
  /*
   * MMU enable
   */
  pm = __get_SCTLR();
  if ((pm & SCTLR_M_Msk) == 0)
    __set_SCTLR(pm | SCTLR_M_Msk);
  /*
   * L1 D cache enable
   */
  pm = __get_SCTLR();
  if ((pm & SCTLR_C_Msk) == 0) {
    __set_SCTLR(pm | SCTLR_C_Msk);
  }
}

/** @} */
