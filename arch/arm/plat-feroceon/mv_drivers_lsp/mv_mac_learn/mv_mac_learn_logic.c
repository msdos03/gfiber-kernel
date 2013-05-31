/************************************************************************
* Copyright (C) 2010, Marvell Technology Group Ltd.
* All Rights Reserved.
*
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Marvell Technology Group;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Marvell Technology Group.
*
*********************************************************************************
* Marvell GPL License Option
*
* If you received this File from Marvell, you may opt to use, redistribute and/or
* modify this File in accordance with the terms and conditions of the General
* Public License Version 2, June 1991 (the "GPL License"), a copy of which is
* available along with the File in the license.txt file or by writing to the Free
* Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
* on the worldwide web at http://www.gnu.org/licenses/gpl.txt.
*
* THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
* WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
* DISCLAIMED.  The GPL License provides additional details about this warranty
* disclaimer.
*
*********************************************************************************
* mv_mac_learn_logic.c
*
* DESCRIPTION:
*
*
*******************************************************************************/
#include "tpm_common.h"
#include "tpm_header.h"
#include "mvOs.h"
#include "mv_mac_learn_header.h"

/************************************************************************************
* mv_mac_learn_get_lu_hw()
* Get LU of MAC leanring PnC entry; but this function is NOT used for long time delay
*************************************************************************************/
int32_t mv_mac_learn_get_lu_hw(uint32_t *pnc_idx)
{
	uint32_t i = 0;
	uint32_t w32,tid;
	tpm_api_lu_conf_t lu_conf;
	tpm_db_pnc_range_conf_t range_conf;

	if (tpm_db_pnc_get_lu_conf(TPM_PNC_MAC_LEARN, &lu_conf)) {
		MVMACLEARN_ERR_PRINT("TPM lookup conf get failed\n");
		return MAC_LEARN_FAIL;
	}
	if (tpm_db_pnc_rng_conf_get(TPM_PNC_MAC_LEARN, &range_conf)){
		MVMACLEARN_ERR_PRINT("TPM PnC range info get failed\n");
		return MAC_LEARN_FAIL;
	}

	i = 0;
	while (i < CONFIG_MV_PNC_TCAM_LINES) {
		w32 = mvPncAgingLogEntryRead(lu_conf.cntr_grp, 0);
		if ((w32 & PNC_AGING_LOG_VALID_MASK) == 0) {
			MVMACLEARN_WARN_PRINT("LU Read ivalid\n");
			tid = range_conf.range_start + range_conf.api_end;
			break;
		}
		mvOsDelay(20);
		tid = w32 & PNC_AGING_LOG_CNTR_IDX_MASK;
		if (tid >= (range_conf.range_start + range_conf.api_start) && tid <= (range_conf.range_start + range_conf.api_end))
			break;
		i++;

		if(!(i%8))
			mvPncAgingTrigerSet();
	}

	if (CONFIG_MV_PNC_TCAM_LINES == i) {
		MVMACLEARN_WARN_PRINT("LU Read failed\n");
		tid = range_conf.range_start + range_conf.api_end;
	}

	*pnc_idx = tid;

	return MAC_LEARN_OK;
}

/************************************************************************************
* mv_mac_learn_get_lu
* Get LU of MAC leanring PnC entry
*output:
*	pnc_idx: PNC index of LU entry
*return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
*************************************************************************************/
int32_t mv_mac_learn_get_lu(uint32_t *pnc_idx)
{
	int32_t i;
	uint32_t hit_count = 0, temp;
	uint32_t lu_idx = 0;
	uint32_t first_lu = 0;
	mac_learn_db_entry_t mac_learn_db_entry;
	tpm_db_pnc_range_conf_t range_conf;

	if (tpm_db_pnc_rng_conf_get(TPM_PNC_MAC_LEARN, &range_conf)){
		MVMACLEARN_ERR_PRINT("TPM PnC range info get failed\n");
		return MAC_LEARN_FAIL;
	}
	/* If only one API entry */
	if (range_conf.api_start == range_conf.api_end) {
		*pnc_idx = range_conf.range_start + range_conf.api_start;
		return MAC_LEARN_OK;
	}

	for (i = range_conf.range_start + range_conf.api_start; i <= range_conf.range_start + range_conf.api_end; i++) {
		/* Static entry not be replaced */
		if (!mac_learn_db_entry_info_by_pncidx(i, NULL, &mac_learn_db_entry)) {
			if (mac_learn_db_entry.state == MAC_LEARN_FDB_STATIC)
				continue;
		} else {
			continue;
		}
		if (first_lu == 0) {
			lu_idx = i;
			hit_count = mvPncAgingCntrRead(lu_idx) & PNC_AGING_CNTR_MASK;
			first_lu = 1;
		} else {
			temp = mvPncAgingCntrRead(i) & PNC_AGING_CNTR_MASK;
			if (hit_count > temp) {
				hit_count = temp;
				lu_idx = i;
			}
		}
	}

	/* All entries are static, can not replace */
	if (first_lu == 0)
		return MAC_LEARN_FAIL;

	*pnc_idx = lu_idx;

	return MAC_LEARN_OK;
}

/************************************************************************************
* mv_mac_learn_aging
* MAC learning aging function for aging timer
*input:
*	mac_learn_aging: pointer of global variable
*return:
*	void
*************************************************************************************/
void mv_mac_learn_aging(unsigned long mac_learn_aging)
{
	uint32_t i;
	const uint32_t tpm_owner_id = TPM_MOD_OWNER_TPM;
	uint32_t pnc_idx, read_count,hit_count = 0;
	uint32_t fdb_nonstatic_valid_count;
	tpm_l2_acl_key_t del_mac_key;
	mac_learn_db_entry_t mac_learn_db_entry;
	tpm_db_pnc_range_conf_t range_conf;

	mac_learn_t *mac_learn_age = (mac_learn_t *)mac_learn_aging;

	if (tpm_db_pnc_rng_conf_get(TPM_PNC_MAC_LEARN, &range_conf)){
		MVMACLEARN_ERR_PRINT("TPM PnC range info get failed\n");
		return;
	}

	for (i = 0; i < MAC_LEARN_FDB_MAX_COUNT; i++) {
		if (!mac_learn_db_nonstatic_pnc_get_by_fdbidx(i, &pnc_idx, &hit_count)) {
			if (MAC_LEARN_FDB_INVALID_PNC == pnc_idx)
				continue;
			/*check whether PnC hit in last 1s*/
			read_count = mvPncAgingCntrRead(pnc_idx);
			if (read_count & PNC_AGING_CNTR_MASK) {
				/*clear no hit time*/
				mac_learn_db_no_time_clear_update_hit(i, read_count & PNC_AGING_CNTR_MASK);
				/*clear hit counter, avoid overflow*/
				mvPncAgingCntrWrite(pnc_idx, read_count & (~PNC_AGING_CNTR_MASK));
			} else {
				/* Increase No hit time and read it */
				mac_learn_db_no_hit_time_inc_read(i, &mac_learn_db_entry);
				if (mac_learn_db_entry.no_hit_time >= mac_learn_age->mac_learn_expire_time) {
					memset(&del_mac_key, 0, sizeof(tpm_l2_acl_key_t));
					memset(del_mac_key.mac.mac_sa_mask, 0xff, sizeof(u8)*MV_MAC_ADDR_SIZE);
					memcpy(del_mac_key.mac.mac_sa, mac_learn_db_entry.src_mac_addr, sizeof(char) * MV_MAC_ADDR_SIZE);
					/*Get tpm del lock*/
					spin_lock(&mac_learn_age->tpm_api_lock[MAC_LEARN_TPM_API_LOCK_DEL]);
					if (!tpm_del_mac_learn_rule(tpm_owner_id, &del_mac_key)) {
						/*release lock*/
						spin_unlock(&mac_learn_age->tpm_api_lock[MAC_LEARN_TPM_API_LOCK_DEL]);
						/*Update FDB first*/
						if (TPM_RANGE_TYPE_ACL == range_conf.range_type)
							mac_learn_db_pncidx_update(0, pnc_idx);
						if (mac_learn_db_entry_del(i)) {
							MVMACLEARN_ERR_PRINT("MAC learn FDB del failed\n");
							continue;
						} else {
							/* dec non static entry count */
							mac_learn_db_nonstatic_count_dec();
							mac_learn_db_nonstatic_count_get(&fdb_nonstatic_valid_count);
							/* start MAC learn again */
							if ((mac_learn_age->mac_learn_max_count - fdb_nonstatic_valid_count == 1) &&
							    (false == mac_learn_age->mac_learn_overwrite_enable)) {
								/* Trap packet with new address to CPU again */
								if (tpm_mac_learn_default_rule_act_set(tpm_owner_id, TPM_UNK_MAC_TRAP)) {
									MVMACLEARN_ERR_PRINT("MAC learn default rule action set failed\n");
									continue;
								}
							}
						}
					} else {
						/*release lock*/
						spin_unlock(&mac_learn_age->tpm_api_lock[MAC_LEARN_TPM_API_LOCK_DEL]);
						MVMACLEARN_ERR_PRINT("TPM MAC learn entry del failed\n");
						continue;
					}
				}
			}
		}
	}

	/*Continue time*/
	mod_timer(&mac_learn_age->mac_learn_timer, jiffies + HZ);
}

/************************************************************************************
* mv_mac_learn_pnc_rule_add
* Function to add new MAC leanring entry automatic in system
*input:
*	work: work queue the function on
*return:
*	void
*************************************************************************************/
void mv_mac_learn_pnc_rule_add(struct work_struct *work)
{
	tpm_init_pnc_mac_learn_enable_t pnc_mac_learn_enable;
	tpm_db_pnc_range_conf_t range_conf;
	tpm_l2_acl_key_t src_mac_key, del_mac_key;
	const uint32_t tpm_owner_id = TPM_MOD_OWNER_TPM;
	uint32_t valid_count, nonstatic_count;
	mac_learn_db_entry_t mac_learn_db_entry;
	int32_t addr_exist = 0;
	int32_t fdb_addr_exist = 0;
	uint32_t rule_num, pnc_idx;
	uint32_t fdb_idx;
	mac_learn_t *src_mac = container_of(work, mac_learn_t, mac_learn_work);

	/*check mac learn enable state*/
	if (false == src_mac->mac_learn_enable)
		return;

	/* Get pnc_mac_learn_enable from TPM XML */
	if (tpm_db_pnc_mac_learn_enable_get(&pnc_mac_learn_enable))
		return;
	if (TPM_PNC_MAC_LEARN_DISABLED == pnc_mac_learn_enable) {
		MVMACLEARN_ERR_PRINT("TPM PNC mac learn disabled\n");
		return;
	}

	/* Check non-static max count, if 0 just return */
	if (src_mac->mac_learn_max_count == 0) {
		/* Read buffer, in order to clear it to avoid bad effection for following MAC learn */
		mv_mac_learn_queue_read(src_mac_key.mac.mac_sa);
		return;
	}

	if (tpm_db_pnc_rng_conf_get(TPM_PNC_MAC_LEARN, &range_conf)){
		/* Read buffer, in order to clear it to avoid bad effection for following MAC learn */
		mv_mac_learn_queue_read(src_mac_key.mac.mac_sa);
		MVMACLEARN_ERR_PRINT("TPM PnC range info get failed\n");
		return;
	}

	memset(&src_mac_key, 0, sizeof(tpm_l2_acl_key_t));
	memset(src_mac_key.mac.mac_sa_mask, 0xff, sizeof(u8)*MV_MAC_ADDR_SIZE);
	memset(&mac_learn_db_entry, 0, sizeof(mac_learn_db_entry_t));
	/*Get source MAC address*/
	if (mv_mac_learn_queue_read(src_mac_key.mac.mac_sa))
		return;
	
	/* Check the src mac exist in FDB or not*/
	if (mac_learn_db_find_mac_addr(&src_mac_key, &fdb_addr_exist, NULL))
		return;
	/* If Addr exist in FDB, return */
	if (fdb_addr_exist == 1)
		return;

	/* Get valid entry count currently */
	if (mac_learn_db_valid_count_get(&valid_count) || mac_learn_db_nonstatic_count_get(&nonstatic_count)) {
		MVMACLEARN_ERR_PRINT("Get valid count or nonstatic count failed\n");
		return;
	}

	/* Check non-static Full or not */
	if (src_mac->mac_learn_max_count == nonstatic_count) {
		if (true == src_mac->mac_learn_overwrite_enable) {
			/* Overwrite LU entry */
			if (!mv_mac_learn_get_lu(&pnc_idx)) {
				MVMACLEARN_DEBUG_PRINT("PnC index need to be overwritten: %d\n", pnc_idx);
				/* Delete LU in PnC first */
				if (!mac_learn_db_entry_info_by_pncidx(pnc_idx, &fdb_idx, &mac_learn_db_entry)) {
					memset(&del_mac_key, 0, sizeof(tpm_l2_acl_key_t));
					memset(del_mac_key.mac.mac_sa_mask, 0xff, sizeof(u8)*MV_MAC_ADDR_SIZE);
					memcpy(del_mac_key.mac.mac_sa, mac_learn_db_entry.src_mac_addr, sizeof(char) * MV_MAC_ADDR_SIZE);
					/*get lock*/
					spin_lock(&src_mac->tpm_api_lock[MAC_LEARN_TPM_API_LOCK_DEL]);
					if (!tpm_del_mac_learn_rule(tpm_owner_id, &del_mac_key)) {
						/*release lock*/
						spin_unlock(&src_mac->tpm_api_lock[MAC_LEARN_TPM_API_LOCK_DEL]);
						/*Update FDB first*/
						if (TPM_RANGE_TYPE_ACL == range_conf.range_type)
							mac_learn_db_pncidx_update(0, pnc_idx);
						if (mac_learn_db_entry_del(fdb_idx)) {
							MVMACLEARN_ERR_PRINT("MAC learn FDB del failed\n");
							return;
						} else {
							mac_learn_db_nonstatic_count_dec();
						}
					} else {
						/*release lock*/
						spin_unlock(&src_mac->tpm_api_lock[MAC_LEARN_TPM_API_LOCK_DEL]);
						MVMACLEARN_ERR_PRINT("TPM MAC learn entry del failed\n");
						return;
					}
				} else {
					MVMACLEARN_ERR_PRINT("MAC learn FDB info get failed\n");
					return;
				}
			}else {
				MVMACLEARN_ERR_PRINT("LU MAC learn entry get failed\n");
				return;
			}
		} else {
			/* Discarded packet with new address */
			if (tpm_mac_learn_default_rule_act_set(tpm_owner_id, TPM_UNK_MAC_DROP)) {
				MVMACLEARN_ERR_PRINT("MAC learn default rule action set failed\n");
				return;
			}
			/* Return from here */
			return;
		}
	}

	/*get lock*/
	spin_lock(&src_mac->tpm_api_lock[MAC_LEARN_TPM_API_LOCK_ADD]);
	if (!tpm_add_mac_learn_rule(tpm_owner_id, &src_mac_key)) {
		/*Release lock*/
		spin_unlock(&src_mac->tpm_api_lock[MAC_LEARN_TPM_API_LOCK_ADD]);
		if (fdb_addr_exist == 0) {
			/* Get PnC index */
			if (!tpm_proc_find_mac_addr_db(&src_mac_key, &addr_exist, &rule_num, &pnc_idx)) {
				if (addr_exist) {
					memset(&mac_learn_db_entry, 0, sizeof(mac_learn_db_entry_t));
					mac_learn_db_entry.valid = MAC_LEARN_FDB_VALID;
					mac_learn_db_entry.state = MAC_LEARN_FDB_NON_STATIC;
					mac_learn_db_entry.port = MAC_LEARN_ON_GMAC;
					mac_learn_db_entry.pnc_idx = range_conf.range_start + range_conf.api_start + pnc_idx;
					mac_learn_db_entry.hit_count = 0;
					mac_learn_db_entry.no_hit_time = 0;
					memcpy(mac_learn_db_entry.src_mac_addr, src_mac_key.mac.mac_sa, sizeof(char) * MV_MAC_ADDR_SIZE);
					MVMACLEARN_DEBUG_PRINT("New non-static MAC learn entry adding, PnC idex: %d\n",mac_learn_db_entry.pnc_idx);
					/*Update FDB first*/
					if (TPM_RANGE_TYPE_ACL == range_conf.range_type)
						mac_learn_db_pncidx_update(1, mac_learn_db_entry.pnc_idx);
					/* Write to FDB */
					if (!mac_learn_db_entry_add(&mac_learn_db_entry))
						mac_learn_db_nonstatic_count_inc();
				}
			}
		}
	} else {
		/*Release lock*/
		spin_unlock(&src_mac->tpm_api_lock[MAC_LEARN_TPM_API_LOCK_ADD]);
	}
}

/************************************************************************************
* mv_mac_learn_logic_static_add
* Function to add new MAC leanring entry mannuly in system
*input:
*	static_mac_addr: source MAC address
*return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
*************************************************************************************/
int32_t mv_mac_learn_logic_static_add(char *static_mac_addr)
{
	tpm_l2_acl_key_t src_mac_key;
	mac_learn_db_entry_t mac_learn_db_entry;
	tpm_db_pnc_range_conf_t range_conf;
	const uint32_t tpm_owner_id = TPM_MOD_OWNER_TPM;
	int32_t addr_exist = 0;
	int32_t fdb_addr_exist = 0;
	uint32_t rule_num, pnc_idx, pnc_max_count, valid_count, non_static_count, non_static_max;
	bool mac_learn_enable = false;

	if (mv_mac_learn_op_enable_get(&mac_learn_enable))
		return MAC_LEARN_FAIL;
	if (false == mac_learn_enable) {
		MVMACLEARN_ERR_PRINT("MAC learn disabled\n\n");
		return MAC_LEARN_FAIL;
	}

	/*Input check*/
	if (NULL == static_mac_addr) {
		MVMACLEARN_ERR_PRINT("Invalid input\n");
		return MAC_LEARN_FAIL;
	}
	MVMACLEARN_DEBUG_PRINT("MAC addr: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", static_mac_addr[0], static_mac_addr[1], static_mac_addr[2],
									    static_mac_addr[3], static_mac_addr[4], static_mac_addr[5]);
	/*init src_mac_key*/
	memset(&src_mac_key, 0, sizeof(tpm_l2_acl_key_t));
	memset(src_mac_key.mac.mac_sa_mask, 0xff, sizeof(u8)*MV_MAC_ADDR_SIZE);
	memcpy(src_mac_key.mac.mac_sa, static_mac_addr, sizeof(char) * MV_MAC_ADDR_SIZE);

	/*Get MAC learn range info*/
	if (tpm_db_pnc_rng_conf_get(TPM_PNC_MAC_LEARN, &range_conf)){
		MVMACLEARN_ERR_PRINT("TPM PnC range info get failed\n");
		return MAC_LEARN_FAIL;
	}
	/* get entry count info */
	pnc_max_count = range_conf.api_end - range_conf.api_start + 1;
	if (mac_learn_db_valid_count_get(&valid_count) ||
	    mac_learn_db_nonstatic_count_get(&non_static_count) ||
	    mv_mac_learn_op_max_count_get(&non_static_max))
		return MAC_LEARN_FAIL;

	/* Check the src mac exist in FDB or not*/
	if (mac_learn_db_find_mac_addr(&src_mac_key, &fdb_addr_exist, &mac_learn_db_entry)) {
		MVMACLEARN_ERR_PRINT("mac_learn_db_find_mac_addr failed\n");
		return MAC_LEARN_FAIL;
	}
	/* If Addr exist in FDB, return OK*/
	if (fdb_addr_exist == 1) {
		if (MAC_LEARN_FDB_STATIC == mac_learn_db_entry.state) {
			MVMACLEARN_WARN_PRINT("Entry already exist\n");
		} else {
			/*check whether static free entry enough or not*/
			if ((valid_count - non_static_count) >= (pnc_max_count - non_static_max)) {
				MVMACLEARN_ERR_PRINT("No free entry for static entry\n");
				return MAC_LEARN_FAIL;
			}
			/* Update original entry state */
			if (mac_learn_db_entry_state_update(mac_learn_db_entry.fdb_idx, MAC_LEARN_FDB_STATIC)) {
				MVMACLEARN_ERR_PRINT("MAC learn FDB entry state update failed\n");
				return MAC_LEARN_FAIL;
			}
			mac_learn_db_nonstatic_count_dec();
		}
		return MAC_LEARN_OK;
	}
	/*check whether static free entry enough or not*/
	if ((valid_count - non_static_count) >= (pnc_max_count - non_static_max)) {
		MVMACLEARN_ERR_PRINT("No free entry for static entry\n");
		return MAC_LEARN_FAIL;
	}

	/*Add entry through TPM API*/
	/*get lock*/
	mac_learn_tpm_api_lock_get(MAC_LEARN_TPM_API_LOCK_ADD);
	if (!tpm_add_mac_learn_rule(tpm_owner_id, &src_mac_key)) {
		/*release lock*/
		mac_learn_tpm_api_lock_release(MAC_LEARN_TPM_API_LOCK_ADD);
		/* Get PnC index */
		if (!tpm_proc_find_mac_addr_db(&src_mac_key, &addr_exist, &rule_num, &pnc_idx)) {
			/*Update FDB*/
			if (addr_exist) {
				memset(&mac_learn_db_entry, 0, sizeof(mac_learn_db_entry_t));
				mac_learn_db_entry.valid = MAC_LEARN_FDB_VALID;
				mac_learn_db_entry.state = MAC_LEARN_FDB_STATIC;
				mac_learn_db_entry.port = MAC_LEARN_ON_GMAC;
				mac_learn_db_entry.pnc_idx = range_conf.range_start + range_conf.api_start + pnc_idx;
				mac_learn_db_entry.hit_count = 0;
				memcpy(mac_learn_db_entry.src_mac_addr, src_mac_key.mac.mac_sa, sizeof(char) * MV_MAC_ADDR_SIZE);
				MVMACLEARN_DEBUG_PRINT("New static MAC learn  entry adding, PnC idex: %d\n",mac_learn_db_entry.pnc_idx);
				/*Update FDB first*/
				if (TPM_RANGE_TYPE_ACL == range_conf.range_type)
					mac_learn_db_pncidx_update(1, mac_learn_db_entry.pnc_idx);
				/* Write to FDB */
				if (mac_learn_db_entry_add(&mac_learn_db_entry)) {
					MVMACLEARN_ERR_PRINT("mac_learn_db_entry_add failed\n");
					return MAC_LEARN_FAIL;
				}
			}
		} else {
			MVMACLEARN_ERR_PRINT("tpm_proc_find_mac_addr_db failed\n");
			return MAC_LEARN_FAIL;
		}
		return MAC_LEARN_OK;
	} else {
		/*release lock*/
		mac_learn_tpm_api_lock_release(MAC_LEARN_TPM_API_LOCK_ADD);
		MVMACLEARN_ERR_PRINT("tpm_add_mac_learn_rule failed\n");
		return MAC_LEARN_FAIL;
	}
}

/************************************************************************************
* mv_mac_learn_logic_static_add
* Function to delete a MAC leanring entry mannuly in system
*input:
*	static_mac_addr: source MAC address
*return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
*************************************************************************************/
int32_t mv_mac_learn_logic_static_del(char *static_mac_addr)
{
	tpm_l2_acl_key_t del_mac_key;
	tpm_db_pnc_range_conf_t range_conf;
	const uint32_t tpm_owner_id = TPM_MOD_OWNER_TPM;
	int32_t addr_exist = 0;
	int32_t fdb_addr_exist = 1;
	uint32_t rule_num, pnc_idx;
	uint32_t fdb_idx;
	bool mac_learn_enable = false;
	mac_learn_db_entry_t mac_learn_db_entry_info;

	if (mv_mac_learn_op_enable_get(&mac_learn_enable))
		return MAC_LEARN_FAIL;
	if (false == mac_learn_enable) {
		MVMACLEARN_ERR_PRINT("MAC learn disabled\n\n");
		return MAC_LEARN_FAIL;
	}

	/*Input check*/
	if (NULL == static_mac_addr) {
		MVMACLEARN_ERR_PRINT("Invalid input\n");
		return MAC_LEARN_FAIL;
	}
	MVMACLEARN_DEBUG_PRINT("MAC addr: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", static_mac_addr[0], static_mac_addr[1], static_mac_addr[2],
									    static_mac_addr[3], static_mac_addr[4], static_mac_addr[5]);
	/*init src_mac_key*/
	memset(&del_mac_key, 0, sizeof(tpm_l2_acl_key_t));
	memset(del_mac_key.mac.mac_sa_mask, 0xff, sizeof(u8)*MV_MAC_ADDR_SIZE);
	memcpy(del_mac_key.mac.mac_sa, static_mac_addr, sizeof(char) * MV_MAC_ADDR_SIZE);

	/* Check the src mac exist in FDB or not*/
	if (mac_learn_db_find_mac_addr(&del_mac_key, &fdb_addr_exist, &mac_learn_db_entry_info)) {
		MVMACLEARN_ERR_PRINT("mac_learn_db_find_mac_addr failed\n");
		return MAC_LEARN_FAIL;
	}
	/* If Addr non-exist in FDB, return OK*/
	if (fdb_addr_exist == 0) {
		MVMACLEARN_ERR_PRINT("Entry non-exist\n");
		return MAC_LEARN_FAIL;
	}

	/* If exist, check it is static mac or non-static, only static mac allow to del with API */
	if (mac_learn_db_entry_info.state == MAC_LEARN_FDB_NON_STATIC) {
		MVMACLEARN_ERR_PRINT("Non-static MAC, not allowed to del with static API\n");
		return MAC_LEARN_FAIL;
	}

	if (tpm_db_pnc_rng_conf_get(TPM_PNC_MAC_LEARN, &range_conf)){
		MVMACLEARN_ERR_PRINT("TPM PnC range info get failed\n");
		return MAC_LEARN_FAIL;
	}

	/* Get PnC index */
	if (!tpm_proc_find_mac_addr_db(&del_mac_key, &addr_exist, &rule_num, &pnc_idx)) {
		if (addr_exist == 0) {
			MVMACLEARN_ERR_PRINT("TPM DB inconsistent with MAC learn FDB\n");
			return MAC_LEARN_FAIL;
		}
		pnc_idx = range_conf.range_start + range_conf.api_start + pnc_idx;
		MVMACLEARN_DEBUG_PRINT("PnC index: %d\n", pnc_idx);
		/*Get FDB entry by PnCidx*/
		if (!mac_learn_db_entry_info_by_pncidx(pnc_idx, &fdb_idx, NULL)) {
			MVMACLEARN_DEBUG_PRINT("FDB index: %d\n", fdb_idx);
			/*get lock*/
			mac_learn_tpm_api_lock_get(MAC_LEARN_TPM_API_LOCK_DEL);
			if (!tpm_del_mac_learn_rule(tpm_owner_id, &del_mac_key)) {
				/*release lock*/
				mac_learn_tpm_api_lock_release(MAC_LEARN_TPM_API_LOCK_DEL);
				/*Update FDB first*/
				if (TPM_RANGE_TYPE_ACL == range_conf.range_type)
					mac_learn_db_pncidx_update(0, pnc_idx);
				if (mac_learn_db_entry_del(fdb_idx)) {
					MVMACLEARN_ERR_PRINT("MAC learn FDB del failed\n");
					return MAC_LEARN_FAIL;
				} else {
					/*start learn again, if fdb full, maybe learn is disabled*/
					if (tpm_mac_learn_default_rule_act_set(tpm_owner_id, TPM_UNK_MAC_TRAP)) {
						MVMACLEARN_ERR_PRINT("MAC learn default rule action set failed\n");
						return MAC_LEARN_FAIL;
					}
				}
			} else {
				/*release lock*/
				mac_learn_tpm_api_lock_release(MAC_LEARN_TPM_API_LOCK_DEL);
				MVMACLEARN_ERR_PRINT("tpm_del_mac_learn_rule failed\n");
				return MAC_LEARN_FAIL;
			}
		} else {
			MVMACLEARN_ERR_PRINT("mac_learn_db_entry_info_by_pncidx failed\n");
			return MAC_LEARN_FAIL;
		}
		/*del success*/
		return MAC_LEARN_OK;
	} else {
		MVMACLEARN_ERR_PRINT("tpm_proc_find_mac_addr_db failed\n");
		return MAC_LEARN_FAIL;
	}
}
