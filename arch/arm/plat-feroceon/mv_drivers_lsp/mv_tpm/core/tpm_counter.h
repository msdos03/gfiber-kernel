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
* tpm_api.h
*
* DESCRIPTION:
*               Traffic Processor Manager = TPM
*
* DEPENDENCIES:
*               None
*
* CREATED BY:   Victor
*
* DATE CREATED:
*
* FILE REVISION NUMBER:
*               Revision: 1.1
*
*
*******************************************************************************/

#ifndef _TPM_COUNTER_H_
#define _TPM_COUNTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define TPM_PNC_AGING_CNTR_OFFS             0
#define TPM_PNC_AGING_CNTR_MAX              0x3ffffff
#define TPM_PNC_AGING_CNTR_MASK             (TPM_PNC_AGING_CNTR_MAX << TPM_PNC_AGING_CNTR_OFFS)

#define TPM_PNC_AGING_LOG_CNTR_IDX_OFFS     0
#define TPM_PNC_AGING_LOG_CNTR_IDX_MASK     (0x3FF << TPM_PNC_AGING_LOG_CNTR_IDX_OFFS)

#define TPM_PNC_AGING_LOG_VALID_BIT         31
#define TPM_PNC_AGING_LOG_VALID_MASK        (1 << TPM_PNC_AGING_LOG_VALID_BIT)

#define TPM_PNC_AGING_TID_NUM_PER_SCAN      8

/*******************************************************************************
* tpm_tcam_set_cntr_group()
*
* DESCRIPTION: The API set the counter group of aging counter
*
* INPUTS:   tid   - TCAM aging entry
*           group - TCAM aging counter group number
*
* OUTPUTS:
*           None
* RETURNS:
*           None
*
* COMMENTS:
*           None
*
*******************************************************************************/
void tpm_tcam_set_cntr_group(int32_t tid, int32_t group);

/*******************************************************************************
* tpm_tcam_get_cntr_group()
*
* DESCRIPTION: The API get the counter group of aging counter
*
* INPUTS:   tid   - TCAM aging entry
*
* OUTPUTS:
*           None
* RETURNS:
*           group - TCAM aging counter group number the PNC belong to
*
* COMMENTS:
*           None
*
*******************************************************************************/
MV_U32 tpm_tcam_get_cntr_group(int32_t tid);

/*******************************************************************************
* tpm_tcam_set_lu_mask()
*
* DESCRIPTION: The API set the LU mask
*
* INPUTS:   tid   - TCAM entry number
*           mask  - mask flag, 0: do not mask, 1:mask LU scan
*
* OUTPUTS:
*           None
* RETURNS:
*           None
*
* COMMENTS:
*           None
*
*******************************************************************************/
void tpm_tcam_set_lu_mask(int32_t tid, int32_t mask);

/*******************************************************************************
* tpm_tcam_get_lu_mask()
*
* DESCRIPTION: The API get the LU mask flag
*
* INPUTS:   tid   - TCAM entry number
*
* OUTPUTS:
*           None
* RETURNS:
*           mask flag, 0: do not mask, non-zero: mask LU scan
*
* COMMENTS:
*           None
*
*******************************************************************************/
MV_U32 tpm_tcam_get_lu_mask(int32_t tid);

/*******************************************************************************
* tpm_count_get_pnc_lu_entry()
*
* DESCRIPTION: The API get least used PnC rule by in specific PnC range
*
* INPUTS:   owner_id        - APP owner id  should be used for all API calls
*           api_type       - TPM API group type
*           lu_num          - The required number of least used PnC entries
*           lu_reset        - Whether need to reset counter after read LU
*
*
* OUTPUTS:
*           valid_num       - The valid number of least used PnC entries
*           count_array     - The least used PnC entry index  and hit counter array
*           unrelated_num   - The unlelated number of least used PnC entries
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_count_get_pnc_lu_entry(uint32_t owner_id,
					    tpm_api_type_t api_type,
					    uint16_t lu_num,
					    uint8_t lu_reset,
					    uint16_t *valid_num,
					    tpm_api_entry_count_t *count_array,
					    uint16_t *unrelated_num);

/*******************************************************************************
* tpm_count_get_pnc_all_hit_counters
*
* DESCRIPTION: The API returns all PnC hit counters per API type lower than a given threshold
*
* INPUTS:   owner_id            - APP owner id  should be used for all API calls
*           api_type            - TPM API group type
*           high_thresh_pkts    - High threashold watermark, counters lower than will be returned
*           counters_reset      - Reset API group type counters after read (0-false, 1-true)
*           valid_counters      - The count_array size (entry number, not byte count)
*
* OUTPUTS:  valid_counters      - The valid number of entries copied to count_array
*           count_array         - The PnC entries for the API type lower than high_thresh_pkts
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_count_get_pnc_all_hit_counters(uint32_t owner_id,
						    tpm_api_type_t api_type,
						    uint32_t high_thresh_pkts,
						    uint8_t counters_reset,
						    uint16_t *valid_counters,
						    tpm_api_entry_count_t *count_array);

/*******************************************************************************
* tpm_count_set_pnc_counter_mask()
*
* DESCRIPTION: The API set the least used scanner ability, to mask or enable it
*
* INPUTS:   owner_id     - APP owner id  should be used for all API calls
*           api_type    - TPM API group type
*           rule_idx     - The PnC rule index returned when created PnC rules
*           lu_rule_mask - The least used scanner mask, 1: maskscanner ,
*                          0:enable scanner
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_count_set_pnc_counter_mask(uint32_t owner_id,
						tpm_api_type_t api_type,
						uint32_t rule_idx,
						uint32_t lu_rule_mask);

/*******************************************************************************
* tpm_count_get_pnc_hit_count()
*
* DESCRIPTION: The API get the hit counter according to rule_idx
*
* INPUTS:   owner_id     - APP owner id  should be used for all API calls
*           api_type    - TPM API group type
*           rule_idx     - The PnC rule index returned when created PnC rules
*           hit_reset    - Should the API reset the hit counters after after reading
*
* OUTPUTS:
*           rule_idx     - The hitted times of specific PnC rule
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_count_get_pnc_hit_count(uint32_t owner_id,
					     tpm_api_type_t api_type,
					     uint32_t rule_idx,
					     uint8_t  hit_reset,
					     uint32_t *hit_count);

/*******************************************************************************
* tpm_count_set_pnc_lu_threshold()
*
* DESCRIPTION: The API set the theshold packets number for least used scanner
*
* INPUTS:   owner_id       - APP owner id  should be used for all API calls
*           api_type      - TPM API group type
*           lu_thresh_pkts - The theshold number(hitted packets number) to be used
*                            for least used scanner
*
* OUTPUTS:
*            None
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_count_set_pnc_lu_threshold(uint32_t owner_id,
						tpm_api_type_t api_type,
						uint32_t lu_thresh_pkts);

/*******************************************************************************
* tpm_count_reset_pnc_age_group()
*
* DESCRIPTION: The API reset the hitted counter of all the PnC entries of specific
*              group
*
* INPUTS:   owner_id     - APP owner id  should be used for all API calls
*           api_type    - TPM API group type
*
* OUTPUTS:
*            None
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_count_reset_pnc_age_group(uint32_t owner_id,
					       tpm_api_type_t api_type);

#ifdef __cplusplus
}
#endif
#endif				/* _TPM_COUNTER_H_ */
