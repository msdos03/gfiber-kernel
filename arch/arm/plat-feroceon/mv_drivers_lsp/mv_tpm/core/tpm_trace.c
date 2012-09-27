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
* tpm_trace.c
*
* DESCRIPTION:
*               Traffic Processor Manager = TPM
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
#define _TPM_TRACE_C_

#include "tpm_common.h"
#include "tpm_header.h"

/* Global variables */
uint32_t tpm_glob_trace = TPM_WARN_TRACE_LEVEL | TPM_ALL_MOD;
/*uint32_t tpm_glob_trace = TPM_DEBUG_TRACE_LEVEL | TPM_ALL_MOD; */

unsigned long print_wait_time;

/*******************************************************************************
* tpm_trace_enable_module()
*
* DESCRIPTION:      Enables tracing for a TPM module
*
* INPUTS:
* module         - Module to enable tracing for (see tpm_trace.h for list)

* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
static tpm_error_code_t tpm_trace_enable_module(uint32_t module)
{
	tpm_error_code_t rc = TPM_RC_OK;

	if (module == TPM_APPL_MAX_MOD)
		tpm_glob_trace |= TPM_ALL_MOD; /* TPM trace is enabled for all applications */
	else {
		switch (module) {
		case TPM_APPL_DB_MOD:
			tpm_glob_trace |= TPM_DB_MOD;
			break;

		case TPM_APPL_PNCL_MOD:
			tpm_glob_trace |= TPM_PNCL_MOD;
			break;

		case TPM_APPL_INIT_MOD:
			tpm_glob_trace |= TPM_INIT_MOD;
			break;

		case TPM_APPL_HWM_MOD:
			tpm_glob_trace |= TPM_HWM_MOD;
			break;

		case TPM_APPL_MODL_MOD:
			tpm_glob_trace |= TPM_MODL_MOD;
			break;

		case TPM_APPL_TPM_LOG_MOD:
			tpm_glob_trace |= TPM_TPM_LOG_MOD;
			break;

		case TPM_APPL_CLI_MOD:
			tpm_glob_trace |= TPM_CLI_MOD;
			break;

		case TPM_APPL_PNC_HM_MOD:
			tpm_glob_trace |= TPM_PNC_HM_MOD;
			break;

		case TPM_APPL_MODZ1_HM_MOD:
			tpm_glob_trace |= TPM_MODZ1_HM_MOD;
			break;

		case TPM_APPL_MODZ2_HM_MOD:
			tpm_glob_trace |= TPM_MODZ2_HM_MOD;
			break;

		default:
			rc = ERR_GENERAL;
			printk(KERN_ERR "Invalid application module - %d\n", module);
			break;
		}
	}

	return rc;
}

/*******************************************************************************
* tpm_trace_disable_module()
*
* DESCRIPTION:     Disables tracing for a TPM module
*
* INPUTS:
* module         - Module to disable tracing for (see tpm_trace.h for list)

* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
static tpm_error_code_t tpm_trace_disable_module(uint32_t module)
{
	tpm_error_code_t rc = TPM_RC_OK;

	if (module == TPM_APPL_MAX_MOD)
		tpm_glob_trace &= ~(TPM_ALL_MOD); /* TPM trace is disabled for all applications */
	else {
		switch (module) {
		case TPM_APPL_DB_MOD:
			tpm_glob_trace &= ~(TPM_DB_MOD);
			break;

		case TPM_APPL_PNCL_MOD:
			tpm_glob_trace &= ~(TPM_PNCL_MOD);
			break;

		case TPM_APPL_INIT_MOD:
			tpm_glob_trace &= ~(TPM_INIT_MOD);
			break;

		case TPM_APPL_HWM_MOD:
			tpm_glob_trace &= ~(TPM_HWM_MOD);
			break;

		case TPM_APPL_MODL_MOD:
			tpm_glob_trace &= ~(TPM_MODL_MOD);
			break;

		case TPM_APPL_TPM_LOG_MOD:
			tpm_glob_trace &= ~(TPM_TPM_LOG_MOD);
			break;

		case TPM_APPL_CLI_MOD:
			tpm_glob_trace &= ~(TPM_CLI_MOD);
			break;

		case TPM_APPL_PNC_HM_MOD:
			tpm_glob_trace &= ~(TPM_PNC_HM_MOD);
			break;

		case TPM_APPL_MODZ1_HM_MOD:
			tpm_glob_trace &= ~(TPM_MODZ1_HM_MOD);
			break;

		case TPM_APPL_MODZ2_HM_MOD:
			tpm_glob_trace &= ~(TPM_MODZ2_HM_MOD);
			break;

		default:
			rc = ERR_GENERAL;
			printk(KERN_ERR "Invalid application module - %d\n", module);
			break;
		}
	}

	return rc;
}

/*******************************************************************************
* tpm_trace_set()
*
* DESCRIPTION:     Disables tracing for a TPM module
*
* INPUTS:
*           level  - trace level

* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_trace_set(uint32_t level)
{
	tpm_error_code_t rc = TPM_RC_OK;
	uint32_t temp_gl_trace;

	if (level >= TPM_APPL_LEVEL_MAX) {
		printk(KERN_ERR "Invalid level - %d\n", level);
		rc = ERR_GENERAL;
	} else {
		if (level == TPM_APPL_LEVEL_NONE)
			tpm_glob_trace &= ~(TPM_ALL_TRACE_LEVEL);
		else {
			temp_gl_trace = tpm_glob_trace & TPM_ALL_MOD;

			switch (level) {
			case TPM_APPL_LEVEL_DEBUG:
				tpm_glob_trace = temp_gl_trace | TPM_DEBUG_TRACE_LEVEL;
				break;

			case TPM_APPL_LEVEL_INFO:
				tpm_glob_trace = temp_gl_trace | TPM_INFO_TRACE_LEVEL;
				break;

			case TPM_APPL_LEVEL_WARN:
				tpm_glob_trace = temp_gl_trace | TPM_WARN_TRACE_LEVEL;
				break;

			case TPM_APPL_LEVEL_ERROR:
				tpm_glob_trace = temp_gl_trace | TPM_ERROR_TRACE_LEVEL;
				break;

			case TPM_APPL_LEVEL_FATAL:
				tpm_glob_trace = temp_gl_trace | TPM_FATAL_TRACE_LEVEL;
				break;
			}
		}
	}

	return rc;
}

/*******************************************************************************
* tpm_trace_module_set()
*
* DESCRIPTION:     Set trace module
*
* INPUTS:
* module         - Module to disable tracing for (see tpm_trace.h for list)

* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_trace_module_set(uint32_t module, uint32_t en_dis)
{
	tpm_error_code_t rc = TPM_RC_OK;

	if (en_dis)
		rc = tpm_trace_enable_module(module);
	else
		rc = tpm_trace_disable_module(module);

	return rc;
}

tpm_error_code_t tpm_trace_status_print(void)
{
	tpm_error_code_t rc = TPM_RC_OK;
	uint16_t module;

	module = tpm_glob_trace & TPM_ALL_MOD;

	printk(KERN_WARNING "===================================\n");
	printk(KERN_WARNING "Trace levels:\n");
	printk(KERN_WARNING "===================================\n");
	printk(KERN_WARNING "Fatal errors                     %s\n", (tpm_glob_trace&TPM_FATAL_MASK) ? "+" : "-");
	printk(KERN_WARNING "Errors                           %s\n", (tpm_glob_trace&TPM_ERROR_MASK) ? "+" : "-");
	printk(KERN_WARNING "Warnings                         %s\n", (tpm_glob_trace&TPM_WARN_MASK) ? "+" : "-");
	printk(KERN_WARNING "Information messages             %s\n", (tpm_glob_trace&TPM_INFO_MASK) ? "+" : "-");
	printk(KERN_WARNING "Debug messages                   %s\n", (tpm_glob_trace&TPM_DEBUG_MASK) ? "+" : "-");
	printk(KERN_WARNING "===================================\n");
	printk(KERN_WARNING "Application modules:\n");
	printk(KERN_WARNING "===================================\n");
	printk(KERN_WARNING "DB                               %s\n", (module&TPM_DB_MOD) ? "+" : "-");
	printk(KERN_WARNING "PNCL                             %s\n", (module&TPM_PNCL_MOD) ? "+" : "-");
	printk(KERN_WARNING "INIT                             %s\n", (module&TPM_INIT_MOD) ? "+" : "-");
	printk(KERN_WARNING "HWM                              %s\n", (module&TPM_HWM_MOD) ? "+" : "-");
	printk(KERN_WARNING "MODL                             %s\n", (module&TPM_MODL_MOD) ? "+" : "-");
	printk(KERN_WARNING "TPM LOG                          %s\n", (module&TPM_TPM_LOG_MOD) ? "+" : "-");
	printk(KERN_WARNING "CLI                              %s\n", (module&TPM_CLI_MOD) ? "+" : "-");
	printk(KERN_WARNING "PNC HM                           %s\n", (module&TPM_PNC_HM_MOD) ? "+" : "-");
	printk(KERN_WARNING "MODZ1 HM                         %s\n", (module&TPM_MODZ1_HM_MOD) ? "+" : "-");
	printk(KERN_WARNING "MODZ2 HM                         %s\n", (module&TPM_MODZ2_HM_MOD) ? "+" : "-");
	printk(KERN_WARNING "===================================\n");

	return rc;
}

