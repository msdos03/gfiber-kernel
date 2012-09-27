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
* tpm_ctc_cm.h
*
* DESCRIPTION:
*               header file for tpm_ctc_cm.c
*
* DEPENDENCIES:
*               None
*
* CREATED BY:   jinghua
*
* DATE CREATED: 7 Feb 2012
*
* FILE REVISION NUMBER:
*               Revision: 1.2
*
*
*******************************************************************************/
#ifndef _TPM_CTC_CM_H_
#define _TPM_CTC_CM_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************/
/*               ENUMERATIONS                                */
/*************************************************************/

/*************************************************************/
/*               DEFINITIONS                                 */
/*************************************************************/

/*******************************************************************************
* tpm_ctc_cm_acl_rule_add()
*
* DESCRIPTION:    Main function for adding CnM API rule.
*
* INPUTS:
*           All inputs/outputs are same as API call
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_ctc_cm_acl_rule_add(uint32_t owner_id,
					 tpm_src_port_type_t src_port,
					 uint32_t precedence,
					 tpm_parse_fields_t l2_parse_rule_bm,
					 tpm_parse_fields_t ipv4_parse_rule_bm,
					 tpm_parse_fields_t ipv6_parse_rule_bm,
					 tpm_l2_acl_key_t *l2_key,
					 tpm_ipv4_acl_key_t *ipv4_key,
					 tpm_ipv6_acl_key_t *ipv6_key,
					 tpm_pkt_frwd_t *pkt_frwd,
					 tpm_pkt_action_t pkt_act,
					 uint32_t pbits);

/*******************************************************************************
* tpm_ctc_cm_acl_rule_del()
*
* DESCRIPTION:    Main function for deleting CnM API rule.
*
* INPUTS:
*		  All inputs/outputs are same as API call
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_ctc_cm_acl_rule_del(uint32_t owner_id,
					 tpm_src_port_type_t src_port,
					 uint32_t precedence);
/*******************************************************************************
* tpm_ctc_cm_ipv6_acl_rule_add()
*
* DESCRIPTION:    Main function for adding CnM API rule.
*
* INPUTS:
*           All inputs/outputs are same as API call
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_ctc_cm_ipv6_acl_rule_add(uint32_t owner_id,
					      tpm_src_port_type_t src_port,
					      uint32_t precedence,
					      tpm_parse_fields_t ipv6_parse_rule_bm,
					      tpm_ipv6_acl_key_t *ipv6_key,
					      tpm_pkt_frwd_t *pkt_frwd,
					      tpm_pkt_action_t pkt_act,
					      uint32_t pbits);

bool tpm_ctc_cm_ipv4_rule2cm(tpm_parse_flags_t parse_flags_bm, tpm_rule_action_t *rule_action);
bool tpm_ctc_cm_l3_rule2cm_sram(tpm_parse_flags_t parse_flags_bm, tpm_rule_action_t *rule_action);
bool tpm_ctc_cm_l3_rule2cm_tcam(tpm_parse_flags_t parse_flags_bm, tpm_l3_type_key_t *l3_key, tpm_rule_action_t *rule_action);
/*******************************************************************************
* tpm_ctc_cm_set_ipv6_parse_win()
*
* DESCRIPTION:    Main function for adding CnM API rule.
*
* INPUTS:
*           All inputs/outputs are same as API call
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_ctc_cm_set_ipv6_parse_win(uint32_t owner_id, tpm_ctc_cm_ipv6_parse_win_t ipv6_parse_window);

#ifdef __cplusplus
}
#endif
#endif    /* _TPM_CTC_CM_H_ */
