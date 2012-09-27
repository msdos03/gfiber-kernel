/*******************************************************************************
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
******************************************************************************/

/******************************************************************************
**  FILE        : mv_mac_learn_mod.c                                         **
**                                                                           **
**  DESCRIPTION : This file implements MAC learn module init                 **
*******************************************************************************
*                                                                             *
*  MODIFICATION HISTORY:                                                      *
*                                                                             *
*   12June2012  Evan Wang   created                                           *
* =========================================================================== *
******************************************************************************/
#include "tpm_common.h"
#include "tpm_header.h"

#include "mv_mac_learn_header.h"

/* Global variable */
mac_learn_t mc_mac_learn;
uint32_t mac_learn_glob_trace = 0;

/*******************************************************************************
* mac_learn_queue_work_insert()
*
* DESCRIPTION:  Insert a work into work queue for mac learning.
*
* INPUTS:
* None
*
* OUTPUTS:
* None
*
* RETURNS:
* void
* COMMENTS:
*
*******************************************************************************/
int mac_learn_queue_work_insert(void)
{
	if (true == mc_mac_learn.queue_init)
		return queue_work(mc_mac_learn.mac_learn_queue, &mc_mac_learn.mac_learn_work);
	return 0;
}

/*******************************************************************************
* mac_learn_tpm_api_lock_get()
*
* DESCRIPTION:  Get spin lock for tpm api.
*
* INPUTS:
* lock_type: TPM api, add or del
*
* OUTPUTS:
* None
*
* RETURNS:
* void
* COMMENTS:
*
*******************************************************************************/
void mac_learn_tpm_api_lock_get(mac_learn_tpm_lock_t lock_type)
{
	/*get lock*/
	spin_lock(&mc_mac_learn.tpm_api_lock[lock_type]);
}

/*******************************************************************************
* mac_learn_tpm_api_lock_release()
*
* DESCRIPTION:  Release spin lock for tpm api.
*
* INPUTS:
* lock_type: TPM api, add or del
*
* OUTPUTS:
* None
*
* RETURNS:
* void
* COMMENTS:
*
*******************************************************************************/
void mac_learn_tpm_api_lock_release(mac_learn_tpm_lock_t lock_type)
{
	/*release lock*/
	spin_unlock(&mc_mac_learn.tpm_api_lock[lock_type]);
}

/*******************************************************************************
* mv_mac_learn_queue_write()
*
* DESCRIPTION:  Write the mac address to learn into buffer queue.
*
* INPUTS:
* src_mac: pointer to mac address to learn
*
* OUTPUTS:
* None
*
* RETURNS:
* success--MAC_LEARN_OK
* failed --MAC_LEARN_FAIL
* COMMENTS:
*
*******************************************************************************/
int mv_mac_learn_queue_write(char *src_mac)
{
	uint32_t i;

	if (down_trylock(&mc_mac_learn.sc_mac_queue.queue_sem))
		return MAC_LEARN_FAIL;

	for (i = 0; i < MAC_LEARN_QUEUE_SIZE; i ++) {
		if (!memcmp(mc_mac_learn.sc_mac_queue.mc_src_mac[i].mac_addr_entry, src_mac, MV_MAC_ADDR_SIZE)) {
			up(&mc_mac_learn.sc_mac_queue.queue_sem);
			return MAC_LEARN_FAIL;
		}
	}

	if (mc_mac_learn.sc_mac_queue.queue_full) {
		up(&mc_mac_learn.sc_mac_queue.queue_sem);
		return MAC_LEARN_FAIL;
	}

	memcpy(mc_mac_learn.sc_mac_queue.mc_src_mac[mc_mac_learn.sc_mac_queue.wr_idx].mac_addr_entry,
	       src_mac, sizeof(char) * MV_MAC_ADDR_SIZE);

	/* insert work queue, if failed remove MAC from cycle buffer */
	if (1 != mac_learn_queue_work_insert()) {
		memset(mc_mac_learn.sc_mac_queue.mc_src_mac[mc_mac_learn.sc_mac_queue.wr_idx].mac_addr_entry, 0,
		       sizeof(char) * MV_MAC_ADDR_SIZE);
		up(&mc_mac_learn.sc_mac_queue.queue_sem);
		return MAC_LEARN_FAIL;
	}

	mc_mac_learn.sc_mac_queue.wr_idx++;
	if (mc_mac_learn.sc_mac_queue.wr_idx == MAC_LEARN_QUEUE_SIZE)
		mc_mac_learn.sc_mac_queue.wr_idx = 0;
	if (mc_mac_learn.sc_mac_queue.wr_idx == mc_mac_learn.sc_mac_queue.rd_idx)
		mc_mac_learn.sc_mac_queue.queue_full = true;

	if (mc_mac_learn.sc_mac_queue.queue_empty)
		mc_mac_learn.sc_mac_queue.queue_empty = false;

	up(&mc_mac_learn.sc_mac_queue.queue_sem);

	return MAC_LEARN_OK;
}

/*******************************************************************************
* mv_mac_learn_queue_read()
*
* DESCRIPTION:  Read the mac address to learn in buffer queue.
*
* INPUTS:
* None
*
* OUTPUTS:
* src_mac: pointer to mac address to learn
*
* RETURNS:
* success--MAC_LEARN_OK
* failed --MAC_LEARN_FAIL
* COMMENTS:
*
*******************************************************************************/
int mv_mac_learn_queue_read(char *src_mac)
{
	if (down_interruptible(&mc_mac_learn.sc_mac_queue.queue_sem))
		return MAC_LEARN_FAIL;

	if (mc_mac_learn.sc_mac_queue.queue_empty) {
		up(&mc_mac_learn.sc_mac_queue.queue_sem);
		return MAC_LEARN_FAIL;
	}

	memcpy(src_mac, mc_mac_learn.sc_mac_queue.mc_src_mac[mc_mac_learn.sc_mac_queue.rd_idx].mac_addr_entry,
	       sizeof(char) * MV_MAC_ADDR_SIZE);
	memset(mc_mac_learn.sc_mac_queue.mc_src_mac[mc_mac_learn.sc_mac_queue.rd_idx].mac_addr_entry, 0,
	       sizeof(char) * MV_MAC_ADDR_SIZE);
	mc_mac_learn.sc_mac_queue.rd_idx++;
	if (mc_mac_learn.sc_mac_queue.rd_idx == MAC_LEARN_QUEUE_SIZE)
		mc_mac_learn.sc_mac_queue.rd_idx = 0;
	if (mc_mac_learn.sc_mac_queue.wr_idx == mc_mac_learn.sc_mac_queue.rd_idx)
		mc_mac_learn.sc_mac_queue.queue_empty = true;

	if (mc_mac_learn.sc_mac_queue.queue_full)
		mc_mac_learn.sc_mac_queue.queue_full = false;

	up(&mc_mac_learn.sc_mac_queue.queue_sem);

	return MAC_LEARN_OK;
}

/*******************************************************************************
* mv_mac_learn_op_enable_set()
*
* DESCRIPTION:  Set the enable state of mac learning.
*
* INPUTS:
* mac_learn_enable: enable state
*
* OUTPUTS:
* None
*
* RETURNS:
* success--MAC_LEARN_OK
* COMMENTS:
*
*******************************************************************************/
int mv_mac_learn_op_enable_set(bool mac_learn_enable)
{
	const uint32_t tpm_owner_id = TPM_MOD_OWNER_TPM;
	tpm_db_pnc_range_conf_t range_conf;
	tpm_init_pnc_mac_learn_enable_t pnc_mac_learn_enable;

	/*check original value and new value, if equal, return OK*/
	if (mac_learn_enable == mc_mac_learn.mac_learn_enable)
		return MAC_LEARN_OK;

	if (true == mac_learn_enable) {
		/*check TPM init succeed or not, before enable MAC learning*/
		if (!tpm_db_init_done_get()) {
			MVMACLEARN_ERR_PRINT("TPM initialization not success\n");
			return MAC_LEARN_FAIL;
		}
		/*check TPM XML pnc_mac_learn_enable*/
		tpm_db_pnc_mac_learn_enable_get(&pnc_mac_learn_enable);
		if (TPM_PNC_MAC_LEARN_DISABLED == pnc_mac_learn_enable) {
			MVMACLEARN_ERR_PRINT("TPM XML para pnc_mac_learn_enable not enabled\n");
			return MAC_LEARN_FAIL;
		}
		/*Set MAX count to min(range_size, FDB_max_count)*/
		if (tpm_db_pnc_rng_conf_get(TPM_PNC_MAC_LEARN, &range_conf)){
			MVMACLEARN_ERR_PRINT("TPM PnC range info get failed\n");
			return MAC_LEARN_FAIL;
		}
		if (range_conf.api_end - range_conf.api_start + 1 > MAC_LEARN_FDB_MAX_COUNT)
			mc_mac_learn.mac_learn_max_count = MAC_LEARN_FDB_MAX_COUNT / 2;
		else
			mc_mac_learn.mac_learn_max_count = (range_conf.api_end - range_conf.api_start + 1) / 2;
		MVMACLEARN_DEBUG_PRINT("non-static entry max count: %d\n",mc_mac_learn.mac_learn_max_count);
		/* Trap packet to CPU */
		if (tpm_mac_learn_default_rule_act_set(tpm_owner_id, TPM_UNK_MAC_TRAP)) {
			MVMACLEARN_ERR_PRINT("TPM default rule action set failed\n");
			return MAC_LEARN_FAIL;
		}
		/*Add timer*/
		add_timer(&mc_mac_learn.mac_learn_timer);
	} else {
		/*delete timer*/
		del_timer(&mc_mac_learn.mac_learn_timer);
		/*Forward packet to GMAC1*/
		if (tpm_mac_learn_default_rule_act_set(tpm_owner_id, TPM_UNK_MAC_CONTINUE)) {
			MVMACLEARN_ERR_PRINT("TPM default rule action set failed\n");
			return MAC_LEARN_FAIL;
		}
		/*Reset FDB and delete PNC rule*/
		if (mac_learn_db_reset()) {
			MVMACLEARN_ERR_PRINT("FDB reset failed\n");
			return MAC_LEARN_FAIL;
		}
	}

	mc_mac_learn.mac_learn_enable = mac_learn_enable;

	return MAC_LEARN_OK;
}

/*******************************************************************************
* mv_mac_learn_op_enable_get()
*
* DESCRIPTION:  Get the enable state of mac learning.
*
* INPUTS:
* None
*
* OUTPUTS:
* mac_learn_enable: enable state
*
* RETURNS:
* success--MAC_LEARN_OK
* COMMENTS:
*
*******************************************************************************/
int mv_mac_learn_op_enable_get(bool *mac_learn_enable)
{
	*mac_learn_enable = mc_mac_learn.mac_learn_enable;

	return MAC_LEARN_OK;
}

/*******************************************************************************
* mv_mac_learn_op_overwrite_set()
*
* DESCRIPTION:  Set the enable state of overwrite if non-static up to max.
*
* INPUTS:
* overwrite_enable: enable state
*
* OUTPUTS:
* None
*
* RETURNS:
* success--MAC_LEARN_OK
* COMMENTS:
*
*******************************************************************************/
int mv_mac_learn_op_overwrite_set(bool overwrite_enable)
{
	const uint32_t tpm_owner_id = TPM_MOD_OWNER_TPM;

	if ((true == overwrite_enable) && true == (mc_mac_learn.mac_learn_enable)) {
		/* Trap packet to CPU when enable overwrite */
		if (tpm_mac_learn_default_rule_act_set(tpm_owner_id, TPM_UNK_MAC_TRAP)) {
			MVMACLEARN_ERR_PRINT("MAC learn default rule action set failed\n");
			return MAC_LEARN_FAIL;
		}
	}
	mc_mac_learn.mac_learn_overwrite_enable = overwrite_enable;

	return MAC_LEARN_OK;
}

/*******************************************************************************
* mv_mac_learn_op_overwrite_get()
*
* DESCRIPTION:  Get the enable state of overwrite if non-static up to max.
*
* INPUTS:
* None
*
* OUTPUTS:
* overwrite_enable: enable state
*
* RETURNS:
* success--MAC_LEARN_OK
* COMMENTS:
*
*******************************************************************************/
int mv_mac_learn_op_overwrite_get(bool *overwrite_enable)
{
	*overwrite_enable = mc_mac_learn.mac_learn_overwrite_enable;

	return MAC_LEARN_OK;
}

/*******************************************************************************
* mv_mac_learn_op_max_count_set()
*
* DESCRIPTION:  Set the max count of non-static entry.
*
* INPUTS:
* mac_learn_max_count: max count
*
* OUTPUTS:
* None
*
* RETURNS:
* success--MAC_LEARN_OK
* COMMENTS:
*
*******************************************************************************/
int mv_mac_learn_op_max_count_set(uint32_t mac_learn_max_count)
{
	const uint32_t tpm_owner_id = TPM_MOD_OWNER_TPM;
	uint32_t non_static_count, valid_count, pnc_count;
	tpm_db_pnc_range_conf_t range_conf;

	if (true == mc_mac_learn.mac_learn_enable) {
		MVMACLEARN_DEBUG_PRINT("Old max count: %d, New max count: %d\n", mc_mac_learn.mac_learn_enable, mac_learn_max_count);
		if (mac_learn_max_count < mc_mac_learn.mac_learn_max_count) {
			if (!mac_learn_db_nonstatic_count_get(&non_static_count)) {
				MVMACLEARN_DEBUG_PRINT("Current non-static num: %d\n", non_static_count);
				if (mac_learn_max_count < non_static_count) {
					/*delete timer*/
					del_timer(&mc_mac_learn.mac_learn_timer);
					/*Clear non-static in FDB and delete PNC rule*/
					mc_mac_learn.mac_learn_enable = false;
					if (mac_learn_db_nonstatic_clear()) {
						mc_mac_learn.mac_learn_enable = true;
						/*add timer*/
						add_timer(&mc_mac_learn.mac_learn_timer);
						MVMACLEARN_ERR_PRINT("FDB non-static entry clear failed\n");
						return MAC_LEARN_FAIL;
					}
					mc_mac_learn.mac_learn_max_count = mac_learn_max_count;
					mc_mac_learn.mac_learn_enable = true;
					/*add timer*/
					add_timer(&mc_mac_learn.mac_learn_timer);
				} else
					mc_mac_learn.mac_learn_max_count = mac_learn_max_count;
			} else {
				return MAC_LEARN_FAIL;
			}
		} else {
			/* Get available PNC number */
			if (tpm_db_pnc_rng_conf_get(TPM_PNC_MAC_LEARN, &range_conf)){
				MVMACLEARN_ERR_PRINT("TPM PnC range info get failed\n");
				return MAC_LEARN_FAIL;
			}
			pnc_count = range_conf.api_end - range_conf.api_start + 1;
			if (mac_learn_db_valid_count_get(&valid_count) || mac_learn_db_nonstatic_count_get(&non_static_count))
				return MAC_LEARN_FAIL;
			if (mac_learn_max_count > (pnc_count - (valid_count - non_static_count))) {
				MVMACLEARN_ERR_PRINT("Non enough free entry left\n");
				return MAC_LEARN_FAIL;
			}
			mc_mac_learn.mac_learn_max_count = mac_learn_max_count;
		}
		/*Trap packet to CPU*/
		if (tpm_mac_learn_default_rule_act_set(tpm_owner_id, TPM_UNK_MAC_TRAP)) {
			MVMACLEARN_ERR_PRINT("TPM default rule action set failed\n");
			return MAC_LEARN_FAIL;
		}
	} else {
		mc_mac_learn.mac_learn_max_count = mac_learn_max_count;
	}

	return MAC_LEARN_OK;
}

/*******************************************************************************
* mv_mac_learn_op_max_count_get()
*
* DESCRIPTION:  Get the max count of non-static entry.
*
* INPUTS:
* None
*
* OUTPUTS:
* mac_learn_max_count: max count
*
* RETURNS:
* success--MAC_LEARN_OK
* COMMENTS:
*
*******************************************************************************/
int mv_mac_learn_op_max_count_get(uint32_t *mac_learn_max_count)
{
	*mac_learn_max_count = mc_mac_learn.mac_learn_max_count;

	return MAC_LEARN_OK;
}

/*******************************************************************************
* mv_mac_learn_op_expire_set()
*
* DESCRIPTION:  Set the non-static entry expired time.
*
* INPUTS:
* mac_learn_expire_time: expired time (s)
*
* OUTPUTS:
* None
*
* RETURNS:
* success--MAC_LEARN_OK
* COMMENTS:
*
*******************************************************************************/
int mv_mac_learn_op_expire_set(uint32_t mac_learn_expire_time)
{
	/*Set new expired time*/
	mc_mac_learn.mac_learn_expire_time = mac_learn_expire_time;

	return MAC_LEARN_OK;
}

/*******************************************************************************
* mv_mac_learn_op_expire_get()
*
* DESCRIPTION:  Get the non-static entry expired time.
*
* INPUTS:
* None
*
* OUTPUTS:
* mac_learn_expire_time: expired time (s)
*
* RETURNS:
* success--MAC_LEARN_OK
* COMMENTS:
*
*******************************************************************************/
int mv_mac_learn_op_expire_get(uint32_t *mac_learn_expire_time)
{
	*mac_learn_expire_time = mc_mac_learn.mac_learn_expire_time;

	return MAC_LEARN_OK;
}

/*******************************************************************************
* mv_mac_learn_op_trace_level_set()
*
* DESCRIPTION:  Set the trace level for print info.
*
* INPUTS:
* trace_level: trace level for print info
*
* OUTPUTS:
* None
*
* RETURNS:
* success--MAC_LEARN_OK
* COMMENTS:
*
*******************************************************************************/
int mv_mac_learn_op_trace_level_set(uint32_t trace_level)
{
	/*Set trace level*/
	mac_learn_glob_trace = trace_level;

	return MAC_LEARN_OK;
}

/*******************************************************************************
* mv_mac_learn_var_init()
*
* DESCRIPTION: Init variable mac_learn_t *mc_mac_learn.
*
* INPUTS:
* mc_mac_learn    Variable to init
* aging_func      Function pointer for aging of non-static MAC learning entry
* queue_work_func Function pointer for work queue, used to receive packet to CPU
*
* OUTPUTS:
* None
*
* RETURNS:
* Init success--MAC_LEARN_OK
* Init failed --MAC_LEARN_FAIL
* COMMENTS:
*
*******************************************************************************/
int mv_mac_learn_var_init(mac_learn_t *mc_mac_learn, void (*aging_func)(unsigned long mac_learn_aging), void (*queue_work_func)(struct work_struct *work))
{
	/* Input Check */
	if (NULL == mc_mac_learn || NULL == aging_func) {
		MVMACLEARN_ERR_PRINT("Invalid input, pointer NULL\n");
		return MAC_LEARN_FAIL;
	}

	/* Default init to disable mac learning */
	mc_mac_learn->mac_learn_enable = false;
	/* Default init to disable overwrite action after full */
	mc_mac_learn->mac_learn_overwrite_enable = false;
	/* Max count set to API range size */
	mc_mac_learn->mac_learn_max_count = MAC_LEARN_NON_STATIC_DEFAULT;
	/* Default expired time set to 300s*/
	mc_mac_learn->mac_learn_expire_time = MAC_LEARN_EX_TIME_DEFAULT;
	/* Init source MAC queue */
	memset(mc_mac_learn->sc_mac_queue.mc_src_mac, 0, sizeof(src_mac_addr_t)*MAC_LEARN_QUEUE_SIZE);
	mc_mac_learn->sc_mac_queue.rd_idx = 0;
	mc_mac_learn->sc_mac_queue.wr_idx = 0;
	mc_mac_learn->sc_mac_queue.queue_empty = true;
	mc_mac_learn->sc_mac_queue.queue_full = false;
	init_MUTEX(&mc_mac_learn->sc_mac_queue.queue_sem);
	/* Init timer for aging */
	init_timer(&mc_mac_learn->mac_learn_timer);
	mc_mac_learn->mac_learn_timer.function = aging_func;
	mc_mac_learn->mac_learn_timer.data = (unsigned long)mc_mac_learn;
	mc_mac_learn->mac_learn_timer.expires = jiffies + HZ;
	/*Init overwrite time*/
	mc_mac_learn->mac_learn_overwrite_time = jiffies;
	/* Init work queue for PnC rule adding */
	mc_mac_learn->mac_learn_queue = create_workqueue("mac_work_queue");
	INIT_WORK(&mc_mac_learn->mac_learn_work, queue_work_func);
	mc_mac_learn->queue_init = true;

	/*Init tpm api spin lock*/
	spin_lock_init(&mc_mac_learn->tpm_api_lock[MAC_LEARN_TPM_API_LOCK_ADD]);
	spin_lock_init(&mc_mac_learn->tpm_api_lock[MAC_LEARN_TPM_API_LOCK_DEL]);

	/* Debug Info */
	MVMACLEARN_DEBUG_PRINT("MAC learn default init: enable = %d, mac_learn_max_count = %d, mac_learn_expire_time = %d\n",
				mc_mac_learn->mac_learn_enable, mc_mac_learn->mac_learn_max_count, mc_mac_learn->mac_learn_expire_time);

	return MAC_LEARN_OK;
}

/*******************************************************************************
* mv_mac_learn_module_exit()
*
* DESCRIPTION: Finish all initialization for module insmod.
*
* INPUTS:
* None
*
* OUTPUTS:
* None
*
* RETURNS:
* Init success--MAC_LEARN_OK
* Init failed --MAC_LEARN_FAIL
* COMMENTS:
*
*******************************************************************************/
static int __init mv_mac_learn_module_init(void)
{
	/* Init Global Valriable */
	if (mv_mac_learn_var_init(&mc_mac_learn, mv_mac_learn_aging, mv_mac_learn_pnc_rule_add)) {
		MVMACLEARN_ERR_PRINT("MAC learn global variable init failed\n");
		return MAC_LEARN_FAIL;
	}
	/* Init FDB */
	if (mac_learn_fdb_init()) {
		MVMACLEARN_ERR_PRINT("MAC learn global variable init failed\n");
		return MAC_LEARN_FAIL;
	}
	/* Init Netdev */
	if (mv_mac_learn_netdev_init()) {
		MVMACLEARN_ERR_PRINT("MAC learn netdev init failed\n");
		return MAC_LEARN_FAIL;
	}
	/*Init sysfs*/
	if(mv_mac_learn_sysfs_init()) {
		MVMACLEARN_ERR_PRINT("MAC learn sysfs init failed\n");
		return MAC_LEARN_FAIL;
	}

	return MAC_LEARN_OK;
}

/*******************************************************************************
* mv_mac_learn_module_exit()
*
* DESCRIPTION: Release related resources when module del.
*
* INPUTS:
* None
*
* OUTPUTS:
* None
*
* RETURNS:
* void
*
* COMMENTS:
*
*******************************************************************************/
static void __exit mv_mac_learn_module_exit(void)
{
	/*release sysfs*/
	mv_mac_learn_sysfs_exit();
}

device_initcall_sync(mv_mac_learn_module_init);

module_exit(mv_mac_learn_module_exit);

MODULE_AUTHOR("Evan Wang");
MODULE_DESCRIPTION("MAC learn module for Media Convert");
MODULE_LICENSE("GPL");