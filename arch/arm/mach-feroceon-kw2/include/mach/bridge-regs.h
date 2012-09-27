/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * Just any arbitrary offset to the start of the vmalloc VM area: the
 * current 8MB value just means that there will be a 8MB "hole" after the
 * physical memory until the kernel virtual memory starts.  That means that
 * any out-of-bounds memory accesses will hopefully be caught.
 * The vmalloc() routines leaves a hole of 4kB between each vmalloced
 * area for the same reason. ;)
 */

/*
 * arch/arm/mach-feroceon-kw/include/mach/bridge-regs.h
 *
 * Mbus-L to Mbus Bridge Registers
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef __ASM_ARCH_BRIDGE_REGS_H
#define __ASM_ARCH_BRIDGE_REGS_H


#include "ctrlEnv/sys/mvCpuIfRegs.h"
#include "ctrlEnv/mvCtrlEnvSpec.h"


#define FEROCEON_REGS_VIRT_BASE        (INTER_REGS_BASE | MAX_AHB_TO_MBUS_REG_BASE)

#define CPU_CONTROL                    (FEROCEON_REGS_VIRT_BASE | CPU_CTRL_STAT_REG)
#define TIMER_VIRT_BASE                (FEROCEON_REGS_VIRT_BASE | MV_CNTMR_REGS_OFFSET)
#define RSTOUTn_MASK                   (FEROCEON_REGS_VIRT_BASE | CPU_RSTOUTN_MASK_REG)
#define BRIDGE_CAUSE                   (FEROCEON_REGS_VIRT_BASE | CPU_AHB_MBUS_CAUSE_INT_REG)

#define WDT_RESET_OUT_EN               0x00000002
#define WDT_INT_REQ                    0x0008


#endif

