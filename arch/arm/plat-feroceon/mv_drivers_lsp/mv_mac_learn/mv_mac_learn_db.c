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
* mv_mac_learn_db.c
*
* DESCRIPTION:
*
*
*******************************************************************************/
#include "tpm_common.h"
#include "tpm_header.h"

#include "mv_mac_learn_header.h"

/*Global variable*/
mac_learn_fdb_t mac_learn_fdb;

/************************************************************************************
*mac_learn_db_nohit_time_clear
* clearn no hit time to 0 for all non-static entry in FDB
*return:
*	success--MAC_LEARN_FDB_OK
************************************************************************************/
int32_t mac_learn_db_nohit_time_clear(void)
{
	uint32_t i;

	for (i = 0; i < MAC_LEARN_FDB_MAX_COUNT; i++) {
		/*Get spin lock*/
		spin_lock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);
		if (MAC_LEARN_FDB_VALID == mac_learn_fdb.mac_learn_db_entry[i].valid &&
		    MAC_LEARN_FDB_NON_STATIC == mac_learn_fdb.mac_learn_db_entry[i].state)
			mac_learn_fdb.mac_learn_db_entry[i].no_hit_time = 0;
		/* Release spin lock */
		spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);
	}

	return MAC_LEARN_FDB_OK;
}

/************************************************************************************
*mac_learn_db_reset
* Reset FDB and delete all MAC learning PNC entry, including non-static and static
*return:
*	success--MAC_LEARN_FDB_OK
*	failed --MAC_LEARN_FDB_FAIL
************************************************************************************/
int32_t mac_learn_db_reset(void)
{
	tpm_l2_acl_key_t del_mac_key;
	const uint32_t tpm_owner_id = TPM_MOD_OWNER_TPM;
	uint32_t i;

	for (i = 0; i < MAC_LEARN_FDB_MAX_COUNT; i++) {
		/*Get spin lock*/
		spin_lock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);
		if (MAC_LEARN_FDB_VALID == mac_learn_fdb.mac_learn_db_entry[i].valid) {
			memset(&del_mac_key, 0, sizeof(tpm_l2_acl_key_t));
			memset(del_mac_key.mac.mac_sa_mask, 0xff, sizeof(u8)*MV_MAC_ADDR_SIZE);
			memcpy(del_mac_key.mac.mac_sa, mac_learn_fdb.mac_learn_db_entry[i].src_mac_addr, sizeof(char) * MV_MAC_ADDR_SIZE);
			/*Get spin lock*/
			mac_learn_tpm_api_lock_get(MAC_LEARN_TPM_API_LOCK_DEL);
			if (!tpm_del_mac_learn_rule(tpm_owner_id, &del_mac_key)) {
				/* Release spin lock */
				mac_learn_tpm_api_lock_release(MAC_LEARN_TPM_API_LOCK_DEL);
				/*invalid FDB entry*/
				mac_learn_fdb.mac_learn_db_entry[i].valid = MAC_LEARN_FDB_INVALID;
				atomic_dec(&mac_learn_fdb.fdb_valid_count);
				if (MAC_LEARN_FDB_NON_STATIC == mac_learn_fdb.mac_learn_db_entry[i].state)
					atomic_dec(&mac_learn_fdb.fdb_non_static_count);
			} else {
				MVMACLEARN_ERR_PRINT("TPM entry delete failed\n");
				/* Release spin lock */
				mac_learn_tpm_api_lock_release(MAC_LEARN_TPM_API_LOCK_DEL);
				spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);

				return MAC_LEARN_FDB_FAIL;
			}
		}
		/* Release spin lock */
		spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);
	}

	return MAC_LEARN_FDB_OK;
}

/************************************************************************************
*mac_learn_db_nonstatic_clear
* Clearn all non-static entry in PNC and FDB
*return:
*	success--MAC_LEARN_FDB_OK
*	failed --MAC_LEARN_FDB_FAIL
************************************************************************************/
int32_t mac_learn_db_nonstatic_clear()
{
	tpm_l2_acl_key_t del_mac_key;
	const uint32_t tpm_owner_id = TPM_MOD_OWNER_TPM;
	uint32_t pnc_idx = 1;
	uint32_t i;

	for (i = 0; i < MAC_LEARN_FDB_MAX_COUNT; i++) {
		if (MAC_LEARN_FDB_VALID == mac_learn_fdb.mac_learn_db_entry[i].valid &&
		    MAC_LEARN_FDB_NON_STATIC == mac_learn_fdb.mac_learn_db_entry[i].state) {
			/*Get spin lock*/
			spin_lock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);

			memset(&del_mac_key, 0, sizeof(tpm_l2_acl_key_t));
			memset(del_mac_key.mac.mac_sa_mask, 0xff, sizeof(u8)*MV_MAC_ADDR_SIZE);
			memcpy(del_mac_key.mac.mac_sa, mac_learn_fdb.mac_learn_db_entry[i].src_mac_addr, sizeof(char) * MV_MAC_ADDR_SIZE);
			pnc_idx = mac_learn_fdb.mac_learn_db_entry[i].pnc_idx;
			if (!tpm_del_mac_learn_rule(tpm_owner_id, &del_mac_key)) {
				/*invalid FDB entry*/
				mac_learn_fdb.mac_learn_db_entry[i].valid = MAC_LEARN_FDB_INVALID;
				atomic_dec(&mac_learn_fdb.fdb_valid_count);
				if (MAC_LEARN_FDB_NON_STATIC == mac_learn_fdb.mac_learn_db_entry[i].state)
					atomic_dec(&mac_learn_fdb.fdb_non_static_count);
			} else {
				MVMACLEARN_ERR_PRINT("TPM entry delete failed\n");
				/* Release spin lock */
				spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);

				return MAC_LEARN_FDB_FAIL;
			}

			/* Release spin lock */
			spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);

			/* Update PnC index */
			if (mac_learn_db_pncidx_update(0, pnc_idx)) {
				MVMACLEARN_ERR_PRINT("TPM PNC index update failed(%d)\n", i);
				return MAC_LEARN_FDB_FAIL;
			}
		}
	}

	return MAC_LEARN_FDB_OK;
}

/************************************************************************************
*mac_learn_db_no_hit_time_get
* Get no-hit time for a non-static MAC learning entry
* input:
*	fdb_idx: entry index in FDB
*output:
*	no_hit_time: no hit time, unit: second
*return:
*	success--MAC_LEARN_FDB_OK
*	failed --MAC_LEARN_FDB_FAIL
************************************************************************************/
int32_t mac_learn_db_no_hit_time_get(uint32_t fdb_idx, uint32_t *no_hit_time)
{
	spin_lock(&mac_learn_fdb.mac_learn_db_entry_lock[fdb_idx]);
	if (MAC_LEARN_FDB_VALID == mac_learn_fdb.mac_learn_db_entry[fdb_idx].valid) {
		if (MAC_LEARN_FDB_NON_STATIC == mac_learn_fdb.mac_learn_db_entry[fdb_idx].state) {
			*no_hit_time = mac_learn_fdb.mac_learn_db_entry[fdb_idx].no_hit_time;

			spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[fdb_idx]);
			return MAC_LEARN_FDB_OK;
		}
	}
	spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[fdb_idx]);
	*no_hit_time = 0;

	return MAC_LEARN_FDB_FAIL;
}

/************************************************************************************
*mac_learn_db_no_time_clear_update_hit
* Clear no-hit time for a non-static MAC learning entry and update its pnc hit count
* input:
*	fdb_idx: entry index in FDB
*	hit_count: new pnc hit count for non-static entry
*return:
*	void
************************************************************************************/
void mac_learn_db_no_time_clear_update_hit(uint32_t fdb_idx, uint32_t hit_count)
{
	spin_lock(&mac_learn_fdb.mac_learn_db_entry_lock[fdb_idx]);
	if (MAC_LEARN_FDB_VALID == mac_learn_fdb.mac_learn_db_entry[fdb_idx].valid) {
		if (MAC_LEARN_FDB_NON_STATIC == mac_learn_fdb.mac_learn_db_entry[fdb_idx].state) {
			mac_learn_fdb.mac_learn_db_entry[fdb_idx].hit_count = hit_count;
			mac_learn_fdb.mac_learn_db_entry[fdb_idx].no_hit_time = 0;
		}
	}
	spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[fdb_idx]);
}

/************************************************************************************
*mac_learn_db_no_time_clear_update_hit
* Clear no-hit time for a non-static MAC learning entry and update its pnc hit count
* input:
*	fdb_idx: entry index in FDB
*	hit_count: new pnc hit count for non-static entry
*return:
*	void
************************************************************************************/
void mac_learn_db_no_hit_time_inc_read(uint32_t fdb_idx, mac_learn_db_entry_t *mac_learn_db_entry)
{
	spin_lock(&mac_learn_fdb.mac_learn_db_entry_lock[fdb_idx]);
	if (MAC_LEARN_FDB_VALID == mac_learn_fdb.mac_learn_db_entry[fdb_idx].valid) {
		if (MAC_LEARN_FDB_NON_STATIC == mac_learn_fdb.mac_learn_db_entry[fdb_idx].state) {
			mac_learn_fdb.mac_learn_db_entry[fdb_idx].no_hit_time++;
			memcpy(mac_learn_db_entry, &mac_learn_fdb.mac_learn_db_entry[fdb_idx], sizeof(mac_learn_db_entry_t));
		}
	}
	spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[fdb_idx]);
}

/************************************************************************************
*mac_learn_db_nonstatic_pnc_get_by_fdbidx
* Clear no-hit time for a non-static MAC learning entry and update its pnc hit count
* input:
*	fdb_idx: entry index in FDB
* output:
*	pnc_idx: the PNC index in TCAM of MAC learning entry
*	hit_count: current hit count of non-static entry
*return:
*	success--MAC_LEARN_FDB_OK
*	failed --MAC_LEARN_FDB_FAIL
************************************************************************************/
int32_t mac_learn_db_nonstatic_pnc_get_by_fdbidx(uint32_t fdb_idx, uint32_t *pnc_idx, uint32_t *hit_count)
{
	/* Input check */
	if (NULL == pnc_idx || NULL == hit_count)
		return MAC_LEARN_FDB_FAIL;

	spin_lock(&mac_learn_fdb.mac_learn_db_entry_lock[fdb_idx]);
	if (MAC_LEARN_FDB_VALID == mac_learn_fdb.mac_learn_db_entry[fdb_idx].valid) {
		if (MAC_LEARN_FDB_NON_STATIC == mac_learn_fdb.mac_learn_db_entry[fdb_idx].state) {
			*pnc_idx = mac_learn_fdb.mac_learn_db_entry[fdb_idx].pnc_idx;
			*hit_count = mac_learn_fdb.mac_learn_db_entry[fdb_idx].hit_count;

			spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[fdb_idx]);
			return MAC_LEARN_FDB_OK;
		}
	}
	spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[fdb_idx]);

	*pnc_idx = MAC_LEARN_FDB_INVALID_PNC;

	return MAC_LEARN_FDB_OK;
}

/************************************************************************************
* mac_learn_db_find_mac_addr
* Find a source MAC address exist in FDB or not, if exist return info of related entry
* input:
*	src_mac_key: TPM L2 key contain souce MAC address
* output:
*	addr_exist: Indicate the MAC exist in FDB or not; 1--exist, 0--non-exist
*	mac_learn_db_entry: related MAC learning entry info if MAC exist
*return:
*	success--MAC_LEARN_FDB_OK
*	failed --MAC_LEARN_FDB_FAIL
************************************************************************************/
int32_t mac_learn_db_find_mac_addr(tpm_l2_acl_key_t *src_mac_key, int32_t *addr_exist, mac_learn_db_entry_t *mac_learn_db_entry)
{
	uint32_t i;

	if (NULL == src_mac_key)
		return MAC_LEARN_FDB_FAIL;

	for (i = 0; i < MAC_LEARN_FDB_MAX_COUNT; i++) {
		/*Get spin lock*/
		spin_lock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);
		if (MAC_LEARN_FDB_VALID == mac_learn_fdb.mac_learn_db_entry[i].valid) {
			if (!memcmp(src_mac_key->mac.mac_sa, mac_learn_fdb.mac_learn_db_entry[i].src_mac_addr, sizeof(char) * MV_MAC_ADDR_SIZE)) {
				if (addr_exist)
					*addr_exist = 1;
				if (mac_learn_db_entry) {
					memcpy(mac_learn_db_entry, &mac_learn_fdb.mac_learn_db_entry[i], sizeof(mac_learn_db_entry_t));
					mac_learn_db_entry->fdb_idx = i;
				}
				/* Release spin lock */
				spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);
				break;
			}
		}
		/* Release spin lock */
		spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);
	}
	if (i == MAC_LEARN_FDB_MAX_COUNT) {
		if (addr_exist)
			*addr_exist = 0;
	}

	return MAC_LEARN_FDB_OK;
}

/************************************************************************************
* mac_learn_db_pncidx_update
* Update PnC index when PNC entry changed in MAC learn range
* input:
*	rule_add: update reason; 1--rule add, 0--rule delete
*	pnc_index: current PNC index before update
*return:
*	success--MAC_LEARN_FDB_OK
*	failed --MAC_LEARN_FDB_FAIL
************************************************************************************/
int32_t mac_learn_db_pncidx_update(uint8_t rule_add, uint32_t pnc_index)
{
	uint32_t i;

	for (i = 0; i < MAC_LEARN_FDB_MAX_COUNT; i++) {
		/*Get spin lock*/
		spin_lock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);
		if (MAC_LEARN_FDB_VALID == mac_learn_fdb.mac_learn_db_entry[i].valid) {
			if (rule_add) {
				if (pnc_index <= mac_learn_fdb.mac_learn_db_entry[i].pnc_idx)
					mac_learn_fdb.mac_learn_db_entry[i].pnc_idx++;
			} else {
				if (pnc_index < mac_learn_fdb.mac_learn_db_entry[i].pnc_idx)
					mac_learn_fdb.mac_learn_db_entry[i].pnc_idx--;
			}
		}
		/* Release spin lock */
		spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);
	}

	return MAC_LEARN_FDB_OK;
}

/************************************************************************************
* mac_learn_db_free_inx_get
* Get the free FDB entry, the first entry find in FDB
* output:
*	free_idx: Free FBD index find
*return:
*	success--MAC_LEARN_FDB_OK
*	failed --MAC_LEARN_FDB_FAIL
************************************************************************************/
int32_t mac_learn_db_free_inx_get(uint32_t *free_idx)
{
	uint32_t i;

	for (i = 0; i < MAC_LEARN_FDB_MAX_COUNT; i++) {
		/*Get spin lock*/
		spin_lock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);
		if (MAC_LEARN_FDB_INVALID == mac_learn_fdb.mac_learn_db_entry[i].valid) {
			mac_learn_fdb.mac_learn_db_entry[i].valid = MAC_LEARN_FDB_RESERVED;
			*free_idx = i;
			/* Release spin lock */
			spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);
			break;
		}
		/* Release spin lock */
		spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);
	}

	if(i == MAC_LEARN_FDB_MAX_COUNT) {
		*free_idx = 0xFFFF;
		return MAC_LEARN_FDB_FAIL;
	}
	else
		return MAC_LEARN_FDB_OK;
}

/************************************************************************************
* mac_learn_db_entry_info_by_pncidx
* Get the info the a valid FDB entry by its PNC index
* input:
*	pnc_idx: PnC index of MAC learning entry
* output:
*	free_idx: Free FBD index find
*return:
*	success--MAC_LEARN_FDB_OK
*	failed --MAC_LEARN_FDB_FAIL
************************************************************************************/
int32_t mac_learn_db_entry_info_by_pncidx(uint32_t pnc_idx, uint32_t *fdb_idx, mac_learn_db_entry_t *mac_learn_db_entry)
{
	uint32_t i;

	for (i = 0; i < MAC_LEARN_FDB_MAX_COUNT; i++) {
		/*Get spin lock*/
		spin_lock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);
		if (MAC_LEARN_FDB_VALID == mac_learn_fdb.mac_learn_db_entry[i].valid) {
			if (mac_learn_fdb.mac_learn_db_entry[i].pnc_idx == pnc_idx) {
				if (NULL != fdb_idx)
					*fdb_idx = i;
				if (NULL != mac_learn_db_entry)
					memcpy(mac_learn_db_entry, &mac_learn_fdb.mac_learn_db_entry[i], sizeof(mac_learn_db_entry_t));
				/* Release spin lock */
				spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);
				break;
			}
		}
		/* Release spin lock */
		spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[i]);
	}

	if(i == MAC_LEARN_FDB_MAX_COUNT)
		return MAC_LEARN_FDB_FAIL;
	else
		return MAC_LEARN_FDB_OK;
}

/************************************************************************************
* mac_learn_db_entry_add
* Add a new entry to FDB
* input:
*	mac_learn_db_entry: FDB entry to add
*return:
*	success--MAC_LEARN_FDB_OK
*	failed --MAC_LEARN_FDB_FAIL
************************************************************************************/
int32_t mac_learn_db_entry_add(mac_learn_db_entry_t *mac_learn_db_entry)
{
	uint32_t free_idx;
	if (mac_learn_db_free_inx_get(&free_idx)) {
		MVMACLEARN_ERR_PRINT("No Free FDB entry\n");
		return MAC_LEARN_FDB_FAIL;
	}
	/*Get spin lock*/
	spin_lock(&mac_learn_fdb.mac_learn_db_entry_lock[free_idx]);
	memcpy(&mac_learn_fdb.mac_learn_db_entry[free_idx], mac_learn_db_entry, sizeof(mac_learn_db_entry_t));
	/* Release spin lock */
	spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[free_idx]);
	/* Increase valid count */
	atomic_inc(&mac_learn_fdb.fdb_valid_count);

	return MAC_LEARN_FDB_OK;
}

/************************************************************************************
* mac_learn_db_entry_add
* Delete an existing entry in FDB
* input:
*	del_idx: FDB index to delete
*return:
*	success--MAC_LEARN_FDB_OK
*	failed --MAC_LEARN_FDB_FAIL
************************************************************************************/
int32_t mac_learn_db_entry_del(uint32_t del_idx)
{
	if (del_idx >= MAC_LEARN_FDB_MAX_COUNT) {
		MVMACLEARN_ERR_PRINT("Invalid input\n");
		return MAC_LEARN_FDB_FAIL;
	}

	/*Get spin lock*/
	spin_lock(&mac_learn_fdb.mac_learn_db_entry_lock[del_idx]);
	mac_learn_fdb.mac_learn_db_entry[del_idx].valid = MAC_LEARN_FDB_INVALID;
	/* Release spin lock */
	spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[del_idx]);
	/* Decrease valid count */
	atomic_dec(&mac_learn_fdb.fdb_valid_count);

	return MAC_LEARN_FDB_OK;
}

/************************************************************************************
* mac_learn_db_valid_count_get
* Get the count of valid entry in FDB
* output:
*	valid_ount: valid entry count
*return:
*	success--MAC_LEARN_FDB_OK
************************************************************************************/
int32_t mac_learn_db_valid_count_get(uint32_t *valid_ount)
{
	*valid_ount = atomic_read(&mac_learn_fdb.fdb_valid_count);

	return MAC_LEARN_FDB_OK;
}

/************************************************************************************
* mac_learn_db_nonstatic_count_inc
* Increase count of non-static entry in FDB by 1
*return:
*	void
************************************************************************************/
void mac_learn_db_nonstatic_count_inc(void)
{
	atomic_inc(&mac_learn_fdb.fdb_non_static_count);
}

/************************************************************************************
* mac_learn_db_nonstatic_count_dec
* Decrease count of non-static entry in FDB by 1
*return:
*	void
************************************************************************************/
void mac_learn_db_nonstatic_count_dec(void)
{
	atomic_dec(&mac_learn_fdb.fdb_non_static_count);
}

/************************************************************************************
* mac_learn_db_nonstatic_count_get
* Get count of non-static entry in FDB
*return:
*	success--MAC_LEARN_FDB_OK
************************************************************************************/
int32_t mac_learn_db_nonstatic_count_get(uint32_t *nonstatic_ount)
{
	*nonstatic_ount = atomic_read(&mac_learn_fdb.fdb_non_static_count);

	return MAC_LEARN_FDB_OK;
}

/************************************************************************************
* mac_learn_db_valid_print
* Print current all valid MAC learning entry in FDB
*return:
*	void
************************************************************************************/
void mac_learn_db_valid_print(void)
{
	uint32_t i, j;
	char entry_state[11];

	/*print header*/
	printk("+--------------+---------------------+--------------+\n");
	printk("+-----State----+-----Source_MAC------+---PNC_Idex---+\n");
	printk("+--------------+---------------------+--------------+\n");

	for (i = 0; i < MAC_LEARN_FDB_MAX_COUNT; i++) {
		if(MAC_LEARN_FDB_VALID == mac_learn_fdb.mac_learn_db_entry[i].valid) {
			if (MAC_LEARN_FDB_STATIC == mac_learn_fdb.mac_learn_db_entry[i].state)
				strcpy(entry_state, "Static    ");
			else
				strcpy(entry_state, "Non-Static");
			printk("+--%s--+--", entry_state);
			for (j = 0; j < 6; j++)
				printk("%2.2x ", mac_learn_fdb.mac_learn_db_entry[i].src_mac_addr[j]);
			printk("-+------%2d------+\n", mac_learn_fdb.mac_learn_db_entry[i].pnc_idx);
		}
	}
	printk("+--------------+---------------------+--------------+\n");
}

/************************************************************************************
* mac_learn_db_entry_state_update
* Update entry state of an existing entry
* input:
*	fdb_idx: FDB entry index
*	entry_state: FDB entry state
*return:
*	success--MAC_LEARN_FDB_OK
*	failed --MAC_LEARN_FDB_FAIL
************************************************************************************/
int32_t mac_learn_db_entry_state_update(uint32_t fdb_idx, uint8_t entry_state)
{
	/* Para check */
	if (fdb_idx >= MAC_LEARN_FDB_MAX_COUNT) {
		MVMACLEARN_ERR_PRINT("MAC learn FDB entry index(%d) invalid\n", fdb_idx);
		return MAC_LEARN_FDB_FAIL;
	}

	if ((MAC_LEARN_FDB_STATIC != entry_state) && (MAC_LEARN_FDB_NON_STATIC != entry_state)) {
		MVMACLEARN_ERR_PRINT("MAC learn FDB entry state(%d) invalid\n", entry_state);
		return MAC_LEARN_FDB_FAIL;
	}

	/* Get spin lock */
	spin_lock(&mac_learn_fdb.mac_learn_db_entry_lock[fdb_idx]);
	/* Update to expected state */
	mac_learn_fdb.mac_learn_db_entry[fdb_idx].state = entry_state;
	/* Release spin lock */
	spin_unlock(&mac_learn_fdb.mac_learn_db_entry_lock[fdb_idx]);

	return MAC_LEARN_FDB_OK;
}

/***************************************************************************************************************
* mac_learn_fdb_init()
*
* DESCRIPTION: Perform DB Initialization. Set all structures to 0, and all valid fields to MAC_LEARN_FDB_INVALID
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns MAC_LEARN_FDB_OK. On error different types are returned
*
* COMMENTS:
*
******************************************************************************************************************/
int32_t mac_learn_fdb_init(void)
{
	uint32_t i;

	/* Erase DB */
	memset(&mac_learn_fdb, 0, sizeof(mac_learn_fdb_t));

	/*init valid_count*/
	atomic_set(&mac_learn_fdb.fdb_valid_count, 0);
	atomic_set(&mac_learn_fdb.fdb_non_static_count, 0);

	for (i = 0; i < MAC_LEARN_FDB_MAX_COUNT; i++) {
		/* Invalid all FDB entry */
		mac_learn_fdb.mac_learn_db_entry[i].valid = MAC_LEARN_FDB_INVALID;
		/* Init Spinlock for each entry */
		spin_lock_init(&mac_learn_fdb.mac_learn_db_entry_lock[i]);
	}

	mac_learn_fdb.mac_learn_fdb_init = true;

	return MAC_LEARN_FDB_OK;
}