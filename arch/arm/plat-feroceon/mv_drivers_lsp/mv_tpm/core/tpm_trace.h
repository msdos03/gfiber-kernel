/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer.

    *   Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.

    *   Neither the name of Marvell nor the names of its contributors may be
	used to endorse or promote products derived from this software without
	specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/

/*******************************************************************************
* tpm_trace.h
*
* DESCRIPTION:
*               Traffic Processor Manager - trace definition.
*
* DEPENDENCIES:
*               None
*
* CREATED BY:   OctaviaP
*
* DATE CREATED:
*
* FILE REVISION NUMBER:
*               Revision: 1.1.1.1
*
*
*******************************************************************************/

#ifndef _TPM_TRACE_H_
#define _TPM_TRACE_H_

#include "mvOs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GLOB_TRACE      tpm_glob_trace

#define TPM_FATAL_MASK  0x80000000	/*Currently, not used */
#define TPM_ERROR_MASK  0x40000000
#define TPM_WARN_MASK   0x20000000
#define TPM_INFO_MASK   0x10000000
#define TPM_DEBUG_MASK  0x08000000

#define TPM_FATAL_TRACE_LEVEL   0x80000000	/*Currently, not used */
#define TPM_ERROR_TRACE_LEVEL   0xC0000000
#define TPM_WARN_TRACE_LEVEL    0xE0000000
#define TPM_INFO_TRACE_LEVEL    0xF0000000
#define TPM_DEBUG_TRACE_LEVEL   0xF8000000

#define TPM_ALL_TRACE_LEVEL     0xFFFF0000


#define tpm_printf      printk

#define TPM_DB_MOD          (0x00000001)
#define TPM_PNCL_MOD        (0x00000002)
#define TPM_INIT_MOD        (0x00000004)
#define TPM_HWM_MOD         (0x00000008)
#define TPM_MODL_MOD        (0x00000010)
#define TPM_TPM_LOG_MOD     (0x00000020)
#define TPM_CLI_MOD         (0x00000040)
#define TPM_PNC_HM_MOD      (0x00000080)
#define TPM_MODZ1_HM_MOD    (0x00000100)
#define TPM_MODZ2_HM_MOD    (0x00000200)
#define TPM_MTU_MOD         (0x00000400)
#define TPM_CTC_CM_MOD      (0x00000800)

#define TPM_ALL_MOD         (0x0000FFFF)

typedef enum tpm_appl_mod {
	TPM_APPL_DB_MOD,
	TPM_APPL_PNCL_MOD,
	TPM_APPL_INIT_MOD,
	TPM_APPL_HWM_MOD,
	TPM_APPL_MODL_MOD,
	TPM_APPL_TPM_LOG_MOD,
	TPM_APPL_CLI_MOD,
	TPM_APPL_PNC_HM_MOD,
	TPM_APPL_MODZ1_HM_MOD,
	TPM_APPL_MODZ2_HM_MOD,

	/* TPM_APPL_MAX_MOD must be always last */
	TPM_APPL_MAX_MOD
} tpm_appl_mod_t;

typedef enum tpm_appl_level {
	TPM_APPL_LEVEL_NONE,
	TPM_APPL_LEVEL_DEBUG,
	TPM_APPL_LEVEL_INFO,
	TPM_APPL_LEVEL_WARN,
	TPM_APPL_LEVEL_ERROR,
	TPM_APPL_LEVEL_FATAL,

	/* TPM_APPL_LEVEL_MAX must be always last */
	TPM_APPL_LEVEL_MAX
} tpm_appl_level_t;

#define TPM_OS_COND_DEBUG(module) if ((module & GLOB_TRACE) && (TPM_DEBUG_MASK & GLOB_TRACE))

#ifdef TPM_PRINT_WAIT  /* See types.h */
#define TPM_OS_DEBUG(module , format , ...) \
		{
			if ((module & GLOB_TRACE) && (TPM_DEBUG_MASK & GLOB_TRACE)) { \
				tpm_printf("%s(%d):  "format , __func__ , __LINE__ , ##__VA_ARGS__); \
				print_wait_time = (jiffies + 2); \
				while (jiffies < print_wait_time) \
					schedule(); \
			} \
		}
#else
#define TPM_OS_DEBUG(module , format , ...) \
		{ \
			if ((module & GLOB_TRACE) && (TPM_DEBUG_MASK & GLOB_TRACE)) \
				tpm_printf("%s(%d):  "format , __func__ , __LINE__ , ##__VA_ARGS__); \
		}
#endif

#define TPM_OS_INFO(module , format , ...)  \
		{ \
			if ((module & GLOB_TRACE) && (TPM_INFO_MASK & GLOB_TRACE))  \
				tpm_printf("%s(%d):  "format , __func__ , __LINE__ , ##__VA_ARGS__); \
		}
#define TPM_OS_WARN(module , format , ...)  \
		{ \
			if ((module & GLOB_TRACE) && (TPM_WARN_MASK & GLOB_TRACE))  \
				tpm_printf("(warn)  %s(%d):  "format , __func__ , __LINE__ , ##__VA_ARGS__); \
		}
#define TPM_OS_ERROR(module , format , ...) \
		{ \
			if ((module & GLOB_TRACE) && (TPM_ERROR_MASK & GLOB_TRACE)) \
				tpm_printf("(error) %s(%d):  "format , __func__ , __LINE__ , ##__VA_ARGS__); \
		}
#define TPM_OS_FATAL(module , format , ...) \
		tpm_printf("(fatal) %s(%d):  "format , __func__ , __LINE__ , ##__VA_ARGS__);

#ifdef TPM_PRINT_WAIT
#define TPM_OS_DEB_WAIT() \
		{\
			print_wait_time = (jiffies + 2);\
			while (jiffies < print_wait_time)\
				schedule();\
		}
#else
#define TPM_OS_DEB_WAIT()
#endif

#ifndef _TPM_TRACE_C_
	extern uint32_t tpm_glob_trace;

	extern unsigned long print_wait_time;
#endif

/* TODO - Change following  to compilation flags */
#define TPM_KERNEL_DEBUG

	tpm_error_code_t tpm_trace_status_print(void);
	tpm_error_code_t tpm_trace_set(uint32_t level);
	tpm_error_code_t tpm_trace_module_set(uint32_t module, uint32_t flag);

#ifdef __cplusplus
}
#endif
#endif
