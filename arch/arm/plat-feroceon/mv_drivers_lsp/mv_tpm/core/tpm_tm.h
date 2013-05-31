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
* tpm_tm.h
*
*
*  MODULE : TPM
*
*  DESCRIPTION : This file config tpm
*
*  MODIFICATION HISTORY:
*           15Aug10   Orenbh  - initial version created.
*
* FILE REVISION NUMBER:
*       Revision: 1.1.1.1
*******************************************************************************/

#ifndef _TPM_TM_H_
#define _TPM_TM_H_

#define  IN
#define  OUT

/*******************************************************************************
* tpm_tm_set_wan_egr_queue_sched()
*
* DESCRIPTION:      Configures the scheduling mode per WAN queue.
*
* INPUTS:
* owner_id          - APP owner id  should be used for all API calls.
* sched_ent         - entity for setting the scheduling mode: ex:TPM_TRG_PORT_WAN
* sched_mode        - scheduler mode per port: strict(0) / wrr(1)
* queue_id          - queue number
* wrr_weight        - weight value when WRR scheduling (1-256)
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_tm_set_wan_egr_queue_sched(IN uint32_t owner_id,
						IN tpm_trg_port_type_t sched_ent,
						IN tpm_pp_sched_type_t sched_mode,
						IN uint8_t queue_id,
						IN uint16_t wrr_weight);

/*******************************************************************************
* tpm_tm_set_wan_sched_egr_rate_lim()
*
* DESCRIPTION:      Configures the egress rate limit of upstream traffic.
*
* INPUTS:
* owner_id          - APP owner id  should be used for all API calls.
* sched_ent         - entity for setting the rate limit: ex:TPM_TRG_PORT_WAN
* rate_limit_val    - egress rate limit value
* bucket_size       - bucket size value
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_tm_set_wan_sched_egr_rate_lim(IN uint32_t owner_id,
						IN tpm_trg_port_type_t sched_ent,
						IN uint32_t rate_limit_val,
						IN uint32_t bucket_size);

/*******************************************************************************
* tpm_tm_set_wan_queue_egr_rate_lim()
*
* DESCRIPTION:      Configures the upstream traffic egress rate limit for a specific
*                   queue of an upstream scheduling entity.
*
* INPUTS:
* owner_id          - APP owner id  should be used for all API calls.
* sched_ent         - entity for setting the rate limit: ex:TPM_TRG_PORT_WAN
* queue_id          - queue number
* rate_limit_val    - egress rate limit value
* bucket_size       - bucket size value
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_tm_set_wan_queue_egr_rate_lim(IN uint32_t owner_id,
						IN tpm_trg_port_type_t sched_ent,
						IN uint32_t queue_id,
						IN uint32_t rate_limit_val,
						IN uint32_t bucket_size);
/*******************************************************************************
* tpm_tm_set_wan_ingr_queue_sched()
*
* DESCRIPTION:      Configures the scheduling mode per all downstream traffic from the WAN.
*
* INPUTS:
* owner_id          - APP owner id  should be used for all API calls.
* sched_mode        - scheduler mode per port: strict(0) / wrr(1)
* queue_id          - queue number
* wrr_weight        - weight value when WRR scheduling (1-256)
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_tm_set_wan_ingr_queue_sched(IN uint32_t owner_id,
						IN tpm_pp_sched_type_t sched_mode,
						IN uint8_t queue_id,
						IN uint16_t wrr_weight);

/*******************************************************************************
* tpm_tm_set_wan_ingr_rate_lim()
*
* DESCRIPTION:      Configures the rate limit of all downstream traffic from the WAN.
*
* INPUTS:
* owner_id          - APP owner id  should be used for all API calls.
* rate_limit_val    - ingress rate limit value
* bucket_size       - bucket size value
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_tm_set_wan_ingr_rate_lim(IN uint32_t owner_id,
						IN uint32_t rate_limit_val,
						IN uint32_t bucket_size);

/*******************************************************************************
* tpm_tm_set_wan_q_ingr_rate_lim()
*
* DESCRIPTION:      Configures the egress rate limit of a specific queue for
*                   downstream traffic.
*
* INPUTS:
* owner_id          - APP owner id  should be used for all API calls.
* queue_id          - queue number
* rate_limit_val    - ingress rate limit value
* bucket_size       - bucket size value
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_tm_set_wan_q_ingr_rate_lim(IN uint32_t owner_id,
						IN uint32_t queue_id,
						IN uint32_t rate_limit_val,
						IN uint32_t bucket_size);

/*******************************************************************************
* tpm_tm_set_gmac0_ingr_rate_lim()
*
* DESCRIPTION:      Configures the ingress rate limit of US for MC only
*
* INPUTS:
*       owner_id          - APP owner id  should be used for all API calls.
*       rate_limit_val    - ingress rate limit value
*       bucket_size       - bucket size value
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns TPM_RC_OK. On error different types are returned
*       according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_tm_set_gmac0_ingr_rate_lim(IN uint32_t owner_id,
						IN uint32_t rate_limit_val,
						IN uint32_t bucket_size);

/*******************************************************************************
* tpm_tm_set_tx_port_rate_lim()
*
* DESCRIPTION:      Configures the rate limit of tx port wanted.
*
* INPUTS:
*       owner_id          - APP owner id  should be used for all API calls.
*       port              - port want to do rate limit
*       rate_limit_val    - ingress rate limit value
*       bucket_size       - bucket size value
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns TPM_RC_OK. On error different types are returned
*       according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_tm_set_tx_port_rate_lim(IN uint32_t owner_id,
					     IN uint32_t port,
					     IN uint32_t rate_limit_val,
					     IN uint32_t bucket_size);

#endif /* _TPM_TM_H_ */
