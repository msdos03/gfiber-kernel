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
* tpm_api.c
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
#include <linux/version.h>
#include "tpm_common.h"
#include "tpm_header.h"
#include "mvOs.h"

/*#define TPM_COUNTER_DEBUG_TIME*/

#define IF_ERROR(ret)	\
		if (ret != TPM_OK) {\
			TPM_OS_ERROR(TPM_PNCL_MOD, " recvd ret_code(%d)\n", ret);\
			return(ret);\
		}

spinlock_t tpmTcamAgingLock;

static uint32_t gs_tcam_array[TPM_MAX_LU_ENTRY_NUM];
static uint32_t gs_valid_tcam_array[TPM_MAX_LU_ENTRY_NUM];

/*******************************************************************************
* tpm_tcam_get_lu_entry()
*
* DESCRIPTION: The API get least used PnC rule by in specific aging couter group
*
* INPUTS:   group  - TCAM hit counter group number
*
* OUTPUTS:
*           None
* RETURNS:
*           LU entry
*
* COMMENTS:
*           None
*
*******************************************************************************/
static inline MV_U32 tpm_tcam_get_lu_entry(int32_t group)
{
	unsigned long flags;
	MV_U32 lu_entry;

	/* Lock the TCAM hit counter */
	spin_lock_irqsave(&tpmTcamAgingLock, flags);

	/* Read LU entry */
	lu_entry = mvPncAgingLogEntryRead(group, 0);

	/* Unlock TCAM hit counter */
	spin_unlock_irqrestore(&tpmTcamAgingLock, flags);

	return lu_entry;
}

/*******************************************************************************
* tpm_tcam_set_triger()
*
* DESCRIPTION: The API set the triger to re-start LU scan
*
* INPUTS:   None
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
static inline void tpm_tcam_set_triger(void)
{
	unsigned long flags;

	/* Lock the TCAM hit counter */
	spin_lock_irqsave(&tpmTcamAgingLock, flags);

	/* Set triger */
	mvPncAgingTrigerSet();

	/* Unlock TCAM hit counter */
	spin_unlock_irqrestore(&tpmTcamAgingLock, flags);

	return;
}

/*******************************************************************************
* tpm_tcam_get_aging_cntr()
*
* DESCRIPTION: The API get hit counter of specific TCAM entry
*
* INPUTS:   tid   - - TCAM entry number
*
* OUTPUTS:
*           None
* RETURNS:
*           Hit counter
*
* COMMENTS:
*           None
*
*******************************************************************************/
static inline MV_U32 tpm_tcam_get_aging_cntr(int32_t tid)
{
	unsigned long flags;
	MV_U32 hit_count;

	/* Lock the TCAM hit counter */
	spin_lock_irqsave(&tpmTcamAgingLock, flags);

	/* read aging hit counter by TID */
	hit_count = mvPncAgingCntrRead(tid);

	/* Unlock TCAM hit counter */
	spin_unlock_irqrestore(&tpmTcamAgingLock, flags);

	return hit_count;
}

/*******************************************************************************
* tpm_tcam_clear_aging_cntr()
*
* DESCRIPTION: The API clear hit counter of specific TCAM entry
*
* INPUTS:   tid   - - TCAM entry number
*
* OUTPUTS:
*           None
* RETURNS:
*           Hit counter
*
* COMMENTS:
*           None
*
*******************************************************************************/
static inline void tpm_tcam_clear_aging_cntr(int32_t tid)
{
	unsigned long flags;

	/* Lock the TCAM hit counter */
	spin_lock_irqsave(&tpmTcamAgingLock, flags);

	/* clear aging hit counter by TID */
	mvPncAgingCntrClear(tid);

	/* Unlock TCAM hit counter */
	spin_unlock_irqrestore(&tpmTcamAgingLock, flags);

	return;
}

/*******************************************************************************
* tpm_tcam_clear_lu_read_flag()
*
* DESCRIPTION: The API clear the LU read flag to let the TCAM entries to be
*              scaned again
*
* INPUTS:   tid   - TCAM entry number
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
static inline void tpm_tcam_clear_lu_read_flag(int32_t tid)
{
	unsigned long flags;

	/* Lock the TCAM hit counter */
	spin_lock_irqsave(&tpmTcamAgingLock, flags);

	/* Clear LU read flag */
	mvPncAgingLuReadClear(tid);

	/* Unlock TCAM hit counter */
	spin_unlock_irqrestore(&tpmTcamAgingLock, flags);

	return;
}

/*******************************************************************************
* tpm_tcam_clear_lu_read_cntr()
*
* DESCRIPTION: The API clear the hit counter of LU read TCAM entry
*
* INPUTS:   tid   - TCAM entry number
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
static inline void tpm_tcam_clear_lu_read_cntr(int32_t tid)
{
	unsigned long flags;

	/* Lock the TCAM hit counter */
	spin_lock_irqsave(&tpmTcamAgingLock, flags);

	/* Clear hit counter of LU entry */
	mvPncAgingCouterClear(tid);

	/* Unlock TCAM hit counter */
	spin_unlock_irqrestore(&tpmTcamAgingLock, flags);

	return;
}

/*******************************************************************************
* tpm_tcam_set_lu_threshold()
*
* DESCRIPTION: The API set the LU threshold
*
* INPUTS:   group - TCAM aging counter group number
*           thesh - threshold value in packet number
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
static inline void tpm_tcam_set_lu_threshold(int32_t group, int32_t thesh)
{
	unsigned long flags;

	/* Lock the TCAM hit counter */
	spin_lock_irqsave(&tpmTcamAgingLock, flags);

	/* Set aging LU threshold */
	mvPncAgingLuThreshSet(group, thesh);

	/* Unlock TCAM hit counter */
	spin_unlock_irqrestore(&tpmTcamAgingLock, flags);

	return;
}

/*******************************************************************************
* tpm_tcam_clear_lu_group()
*
* DESCRIPTION: The API clear the hit counter of all the TCAM entries in one group
*
* INPUTS:   group - TCAM aging counter group number
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
static inline void tpm_tcam_clear_lu_group(int32_t group)
{
	unsigned long flags;

	/* Lock the TCAM hit counter */
	spin_lock_irqsave(&tpmTcamAgingLock, flags);

	/* Clear aging counter group */
	mvPncAgingGroupCntrClear(group);

	/* Unlock TCAM hit counter */
	spin_unlock_irqrestore(&tpmTcamAgingLock, flags);

	return;
}

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
void tpm_tcam_set_cntr_group(int32_t tid, int32_t group)
{
	unsigned long flags;

	/* Lock the TCAM hit counter */
	spin_lock_irqsave(&tpmTcamAgingLock, flags);

	/* Set aging LU counter group */
	mvPncAgingCntrGroupSet(tid, group);

	/* Unlock TCAM hit counter */
	spin_unlock_irqrestore(&tpmTcamAgingLock, flags);

	return;
}

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
MV_U32 tpm_tcam_get_cntr_group(int32_t tid)
{
	uint32_t w32;

	/* Reading counter reg */
	w32 = mvPncAgingCntrRead(tid);
	w32 = (w32 & PNC_AGING_GROUP_ALL_MASK) >> PNC_AGING_GROUP_OFFS;

	return w32;
}

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
void tpm_tcam_set_lu_mask(int32_t tid, int32_t mask)
{
	unsigned long flags;

	/* Lock the TCAM hit counter */
	spin_lock_irqsave(&tpmTcamAgingLock, flags);

	/* Set LU scan mask */
	mvPncAgingCntrLuMaskSet(tid, mask);

	/* Unlock TCAM hit counter */
	spin_unlock_irqrestore(&tpmTcamAgingLock, flags);

	return;
}

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
MV_U32 tpm_tcam_get_lu_mask(int32_t tid)
{
	uint32_t w32;

	/* Reading counter reg */
	w32 = mvPncAgingCntrRead(tid);
	w32 &= PNC_AGING_SKIP_LU_SCAN_MASK;

	return w32;
}

/*******************************************************************************
* tpm_count_get_api_section_range_id()
*
* DESCRIPTION: The API get API section and PnC range ID according to API group
*              type
*
* INPUTS:   owner_id     - APP owner id  should be used for all API calls
*           api_type     - TPM API group type
*
* OUTPUTS:
*           api_section  - API section ID
*           range_id     - PnC range ID

* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_count_get_api_section_range_id(uint32_t owner_id,
						    tpm_api_type_t api_type,
						    tpm_api_sections_t *api_section,
						    tpm_pnc_ranges_t *range_id)
{
	tpm_api_sections_t l_api_section;
	tpm_pnc_ranges_t l_range_id;
	int32_t ret_code = TPM_RC_OK;

	/* Check whether the API group is supported, currently only support three types */
	if ((api_type != TPM_API_L2_PRIM) && (api_type != TPM_API_L3_TYPE)
	    && (api_type != TPM_API_IPV4)) {
		TPM_OS_ERROR(TPM_PNCL_MOD, "api_type[%d] is not supported \n", api_type);
		return ERR_GENERAL;
	}

	/* Convert to API section ID */
	switch (api_type) {
	case TPM_API_L2_PRIM:
		l_api_section = TPM_L2_PRIM_ACL;
		break;
	case TPM_API_L3_TYPE:
		l_api_section = TPM_L3_TYPE_ACL;
		break;
	case TPM_API_IPV4:
		l_api_section = TPM_IPV4_ACL;
		break;
	default:
		l_api_section = TPM_IPV4_ACL;
		break;
	}

	/* Get PnC range ID */
	ret_code = tpm_db_api_section_main_pnc_get(l_api_section, &l_range_id);
	IF_ERROR(ret_code);

	/* Save API section ID and PnC range ID */
	if (api_section != NULL)
		*api_section = l_api_section;
	if (range_id != NULL)
		*range_id = l_range_id;

	return TPM_RC_OK;
}

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
					    uint16_t *unrelated_num)
{
	tpm_api_lu_conf_t lu_conf;
	tpm_api_sections_t api_section;
	tpm_pnc_ranges_t range_id;
	int16_t l_lu_num = 0;
	uint16_t l_valid_num = 0;
	uint16_t l_invalid_num = 0;
	uint32_t l_rule_idx = 0;
	uint32_t l_tcam_val = 0;
	uint32_t l_tcam_num = 0;
	uint32_t range_start = 0;
	uint32_t range_end = 0;
	uint32_t w32 = 0;
	int32_t count_group = 0;
	int32_t ret_code = TPM_RC_OK;
	tpm_error_code_t tpm_ret_code = TPM_RC_OK;
	uint32_t l_retry_time  = 0;
#ifdef TPM_COUNTER_DEBUG_TIME
	unsigned long t_start;
	unsigned long t_dif;
#endif

	TPM_OS_DEBUG(TPM_PNCL_MOD, "%s in, owner_id[%d], api_type[%d], lu_num[%d], lu_reset[%d]\n",
		     __func__, owner_id, api_type, lu_num, lu_reset);

	/* Get API section and PnC range ID */
	ret_code = tpm_count_get_api_section_range_id(owner_id, api_type, &api_section, &range_id);
	IF_ERROR(ret_code);

	/* Check the required number */
	if (lu_num > TPM_MAX_LU_ENTRY_NUM) {
		TPM_OS_ERROR(TPM_PNCL_MOD, "lu_num[%d] is illegal, allowed number[0~%d] \n", lu_num,
			     TPM_MAX_LU_ENTRY_NUM);
		return(ERR_GENERAL);
	}

	/* Get counter group number and mask all flag */
	ret_code = tpm_db_pnc_get_lu_conf(range_id, &lu_conf);
	IF_ERROR(ret_code);
	count_group = lu_conf.cntr_grp;

	/* Get range start number */
	ret_code = tpm_db_pnc_rng_get_range_start_end(range_id, &range_start, &range_end);
	IF_ERROR(ret_code);

	/* Get valid LU entries */
	memset(gs_tcam_array, 0, sizeof(gs_tcam_array));
	memset(gs_valid_tcam_array, 0, sizeof(gs_valid_tcam_array));

#ifdef TPM_COUNTER_DEBUG_TIME
	t_start = MV_REG_READ(0xAC40C);
#endif
	for (l_lu_num = 0; l_lu_num < CONFIG_MV_PNC_TCAM_LINES; l_lu_num++) {
		w32 = tpm_tcam_get_lu_entry(count_group);
		if ((w32 & TPM_PNC_AGING_LOG_VALID_MASK) == 0) {
			l_retry_time++;
			if (l_retry_time > 1)
				break;

			tpm_tcam_set_triger();
			mvOsUDelay(100);
			if (l_lu_num > 0)
				l_lu_num--;
			else
				l_lu_num = -1;

			continue;
		} else
			l_retry_time = 0;

		l_tcam_val = w32 & TPM_PNC_AGING_LOG_CNTR_IDX_MASK;

		gs_tcam_array[l_tcam_num++] = l_tcam_val;

		/*printk("l_tcam_val[%d], range_start[%d], range_end[%d] \n", l_tcam_val, range_start, range_end); */

		/* Check whether the TCAM is in current PnC range */
		if ((l_tcam_val < range_start) || (l_tcam_val > range_end))
			l_invalid_num++;
		else {
			/* Get rule index */
			ret_code = tpm_db_api_tcam_rule_idx_get(api_section, l_tcam_val, &l_rule_idx);
			if (ret_code == TPM_DB_OK) {
				/*IF_ERROR(ret_code); */

				/* Save rule index, hit counter and valid TCAM index */
				(count_array + l_valid_num)->rule_idx = l_rule_idx;
				(count_array + l_valid_num)->hit_count = tpm_tcam_get_aging_cntr(l_tcam_val)&TPM_PNC_AGING_CNTR_MASK;
				gs_valid_tcam_array[l_valid_num++] = l_tcam_val;

				/* The required number */
				if (l_valid_num >= lu_num)
					break;
			} else {
				l_invalid_num++;
			}
		}
		/* Triger LU */
		if (!((l_tcam_num) % TPM_PNC_AGING_TID_NUM_PER_SCAN)) {
			tpm_tcam_set_triger();
			mvOsUDelay(100);
		}
	}

	/* Save valid and unrelated number */
	*valid_num = l_valid_num;
	*unrelated_num = l_invalid_num;

	/* Clear LU read flag */
	for (l_lu_num = 0; l_lu_num < l_tcam_num; l_lu_num++)
		tpm_tcam_clear_lu_read_flag(gs_tcam_array[l_lu_num]);

	/* Reset counter if needed */
	if (lu_reset != 0) {
		tpm_ret_code = tpm_count_reset_pnc_age_group(owner_id, api_type);
		if ((TPM_RC_OK != tpm_ret_code)) {
			TPM_OS_ERROR(TPM_PNCL_MOD, "Failed to call tpm_count_reset_pnc_age_group, ret_code[%d] \n",
				     tpm_ret_code);
			return(ERR_GENERAL);
		}
/* Followings are old codes, just remain them for reference */
/*
	for(l_lu_num = 0; l_lu_num < l_valid_num; l_lu_num++)
		tpm_tcam_clear_lu_read_cntr(gs_valid_tcam_array[l_lu_num]);
*/
	}

#ifdef TPM_COUNTER_DEBUG_TIME
	t_dif = MV_REG_READ(0xAC40C) - t_start;
	printk(KERN_ERR "(victor) %s(%d):  %ld\n", __func__, __LINE__, t_dif);
#endif
	/* Debug info */
	TPM_OS_DEBUG(TPM_PNCL_MOD, "%s out, valid_num[%d], unrelated_num[%d]\n",
		     __func__, *valid_num, *unrelated_num);
	for (l_lu_num = 0; l_lu_num < *valid_num; l_lu_num++) {
		TPM_OS_DEBUG(TPM_PNCL_MOD, "Index[%d] rule_idx_array[%d], hit_count[%d]\n",
			     l_lu_num, (count_array + l_lu_num)->rule_idx, (count_array + l_lu_num)->hit_count);
	}

	return(TPM_RC_OK);
}

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
						    tpm_api_entry_count_t *count_array)
{
	tpm_api_sections_t api_section;
	tpm_pnc_ranges_t   range_id;
	uint16_t           count_array_size;
	uint32_t           rule_idx;
	uint32_t           tcam_val;
	uint32_t           range_start;
	uint32_t           range_end;
	uint32_t           hit_counter;
	uint16_t           cntrs_nr;
	int32_t            ret_code;

	TPM_OS_DEBUG(TPM_PNCL_MOD, "%s in, owner_id[%d], api_type[%d], counters_reset[%d] high_thresh_pkts[%d]\n",
		     __func__, owner_id, api_type, counters_reset, high_thresh_pkts);

	/* Get API section and PnC range ID */
	ret_code = tpm_count_get_api_section_range_id(owner_id, api_type, &api_section, &range_id);
	IF_ERROR(ret_code);

	/* Get range start number */
	ret_code = tpm_db_pnc_rng_get_range_start_end(range_id, &range_start, &range_end);
	IF_ERROR(ret_code);

	if (*valid_counters > TPM_MAX_PNC_COUNTER_NUM) {
	    TPM_OS_ERROR(TPM_PNCL_MOD, "valid_counters[%d] is illegal, allowed number[1~%d] \n",
			    (*valid_counters), TPM_MAX_PNC_COUNTER_NUM);
	    return(ERR_GENERAL);
	}
	/* save the array size */
	count_array_size = *valid_counters;
	cntrs_nr = 0;

	/* traverse all pnc tcam lines */
	for (tcam_val = range_start; tcam_val <= range_end; tcam_val++) {
	/* Get rule index */
		ret_code = tpm_db_api_tcam_rule_idx_get(api_section, tcam_val, &rule_idx);
		if (ret_code == TPM_DB_OK) {
			/* read counter from HW  */
			hit_counter = tpm_tcam_get_aging_cntr(tcam_val);
			hit_counter = (hit_counter & PNC_AGING_CNTR_MASK) >> PNC_AGING_CNTR_OFFS;

			/* skip low counters */
			if (hit_counter >= high_thresh_pkts)
				continue;

			/* Save rule index, hit counter and valid TCAM index */
			count_array[cntrs_nr].rule_idx = rule_idx;
			count_array[cntrs_nr].hit_count = hit_counter;

			cntrs_nr++;

			/* The table is full */
			if (count_array_size == cntrs_nr)
				break;
		} else {
			continue;
		}
	}

	/* update the number of entries updated in count_array */
	*valid_counters = cntrs_nr;

	/* Reset counter if needed */
	if (counters_reset != 0) {
		tpm_error_code_t tpm_ret_code = tpm_count_reset_pnc_age_group(owner_id, api_type);

		if (TPM_RC_OK != tpm_ret_code) {
			TPM_OS_ERROR(TPM_PNCL_MOD, "Failed to call tpm_count_reset_pnc_age_group, ret_code[%d] \n", tpm_ret_code);
			return(ERR_GENERAL);
		}
	}

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_count_set_pnc_counter_mask()
*
* DESCRIPTION: The API set the least used scanner ability, to mask or enable it
*
* INPUTS:   owner_id     - APP owner id  should be used for all API calls
*           api_type    - TPM API group type
*           rule_idx     - The PnC rule index returned when created PnC rules
*           lu_rule_mask - The least used scanner mask, 1: mask scanner,
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
						uint32_t lu_rule_mask)
{
	tpm_api_sections_t api_section;
	tpm_pnc_ranges_t range_id;
	int32_t tcam_num = 0;
	int32_t ret_code = TPM_RC_OK;

	TPM_OS_DEBUG(TPM_PNCL_MOD, "%s in, owner_id[%d], api_type[%d], rule_idx[%d], lu_rule_mask[%d]\n",
		     __func__, owner_id, api_type, rule_idx, lu_rule_mask);

	/* Get API section and PnC range ID */
	ret_code = tpm_count_get_api_section_range_id(owner_id, api_type, &api_section, &range_id);
	IF_ERROR(ret_code);

	/* Get TCAM number */
	ret_code = tpm_db_api_tcam_num_get(api_section, rule_idx, &tcam_num);
	IF_ERROR(ret_code);

	/* Set to PnC TCAM aging counter register */
	tpm_tcam_set_lu_mask((int32_t) tcam_num, (int32_t) lu_rule_mask);

	return (TPM_RC_OK);
}

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
					     uint32_t *hit_count)
{
	tpm_api_sections_t api_section;
	tpm_pnc_ranges_t range_id;
	int32_t tcam_num = 0;
	int32_t ret_code = TPM_RC_OK;

	TPM_OS_DEBUG(TPM_PNCL_MOD, "%s in, owner_id[%d], api_type[%d], rule_idx[%d]\n",
		     __func__, owner_id, api_type, rule_idx);

	/* Get API section and PnC range ID */
	ret_code = tpm_count_get_api_section_range_id(owner_id, api_type, &api_section, &range_id);
	IF_ERROR(ret_code);

	/* Get TCAM number */
	ret_code = tpm_db_api_tcam_num_get(api_section, rule_idx, &tcam_num);
	IF_ERROR(ret_code);

	/* Set to PnC TCAM aging counter register */
	*hit_count = tpm_tcam_get_aging_cntr((int32_t) tcam_num);

	*hit_count &= TPM_PNC_AGING_CNTR_MASK;

	if (hit_reset)
		tpm_tcam_clear_aging_cntr((int32_t) tcam_num);

	TPM_OS_DEBUG(TPM_PNCL_MOD, "%s out, tcam_num[%d], hit_count[%d\n", __func__, tcam_num, *hit_count);

	return (TPM_RC_OK);
}

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
						uint32_t lu_thresh_pkts)
{
	tpm_api_lu_conf_t lu_conf;
	tpm_pnc_ranges_t range_id;
	int32_t ret_code = TPM_RC_OK;

	TPM_OS_DEBUG(TPM_PNCL_MOD, "%s in, owner_id[%d], api_type[%d], lu_thresh_pkts[%d]\n",
		     __func__, owner_id, api_type, lu_thresh_pkts);

	/* Get PnC range ID */
	ret_code = tpm_count_get_api_section_range_id(owner_id, api_type, NULL, &range_id);
	IF_ERROR(ret_code);

	/* Get counter group number */
	ret_code = tpm_db_pnc_get_lu_conf(range_id, &lu_conf);
	IF_ERROR(ret_code);

	/* Check LU threshold */
	if (lu_thresh_pkts > TPM_MAX_LU_THRESH_VAL) {
		TPM_OS_ERROR(TPM_PNCL_MOD, "lu_thresh_pkts[%d]is illegal, exceeds maximum value(%d)\n",
			     lu_thresh_pkts, TPM_PNC_AGING_CNTR_MAX);
		return (ERR_GENERAL);
	}

	tpm_tcam_set_lu_threshold(lu_conf.cntr_grp, lu_thresh_pkts);

	return (TPM_RC_OK);
}

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
					       tpm_api_type_t api_type)
{
	tpm_api_lu_conf_t lu_conf;
	tpm_pnc_ranges_t range_id;
	int32_t ret_code = TPM_RC_OK;

	TPM_OS_DEBUG(TPM_PNCL_MOD, "%s in, owner_id[%d], api_type[%d]\n", __func__, owner_id, api_type);

	/* Get PnC range ID */
	ret_code = tpm_count_get_api_section_range_id(owner_id, api_type, NULL, &range_id);
	IF_ERROR(ret_code);

	/* Get counter group number */
	ret_code = tpm_db_pnc_get_lu_conf(range_id, &lu_conf);
	IF_ERROR(ret_code);

	tpm_tcam_clear_lu_group(lu_conf.cntr_grp);

	return (TPM_RC_OK);
}
