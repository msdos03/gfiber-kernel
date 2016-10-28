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
* tpm_init.h
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
#ifndef _TPM_INIT_H_
#define _TPM_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#include "boardEnv/mvBoardEnvLib.h"

/************/
/*  APIs    */
/************/

int32_t tpm_init_pncranges_set(tpm_reset_level_enum_t reset_type);
int32_t tpm_init_system_init(tpm_reset_level_enum_t reset_type);
int32_t tpm_init_system_mib_reset(tpm_reset_level_enum_t reset_type);
int32_t tpm_init_api_rng_init_all(void);
int32_t tpm_module_init(void);
int32_t tpm_module_start(tpm_init_t *tpm_init);
void tpm_init_params_get(tpm_init_t *tpm_init_params);
int32_t tpm_init_get_gmac_queue_rate_limit(void);
int32_t tpm_init_mh(tpm_init_mh_src_t ds_mh_set_conf,
			uint32_t gmac0_mh_en,
			uint32_t gmac1_mh_en,
			tpm_init_gmac_conn_conf_t *gmac_port_conf);
int32_t tpm_init_ipg(int32_t mod_value);
int32_t tpm_init_gmac_loopback(tpm_gmacs_enum_t port, uint8_t enable);
int32_t tpm_init_qos(void);

#endif /* _TPM_INIT_H_ */
