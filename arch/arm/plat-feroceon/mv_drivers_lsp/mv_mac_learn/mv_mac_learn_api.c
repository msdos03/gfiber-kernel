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
* mv_mac_learn_api.c
*
* DESCRIPTION:
*
*
*******************************************************************************/
#include "tpm_common.h"
#include "tpm_header.h"

#include "mv_mac_learn_header.h"

/***********************************************************
* mv_mac_learn_static_entry_add
* API for adding static learn entry
* input:
*	static_mac_addr: source MAC address
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
***********************************************************/
int32_t mv_mac_learn_static_entry_add(char *static_mac_addr)
{
	int32_t ret;

	ret = mv_mac_learn_logic_static_add(static_mac_addr);
	if (ret) {
		MVMACLEARN_ERR_PRINT("MAC learn static entry adding failed\n");
		return MAC_LEARN_FAIL;
	}

	return MAC_LEARN_OK;
}
EXPORT_SYMBOL(mv_mac_learn_static_entry_add);

/***********************************************************
* mv_mac_learn_static_entry_del
* API for del static learn entry
* input:
*	static_mac_addr: source MAC address
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
************************************************************/
int32_t mv_mac_learn_static_entry_del(char *static_mac_addr)
{
	int32_t ret;

	ret = mv_mac_learn_logic_static_del(static_mac_addr);
	if (ret) {
		MVMACLEARN_ERR_PRINT("MAC learn static entry del failed\n");
		return MAC_LEARN_FAIL;
	}

	return MAC_LEARN_OK;
}
EXPORT_SYMBOL(mv_mac_learn_static_entry_del);

/***********************************************************
* mv_mac_learn_enable_set
* API for set MAC leaning enable state
* input:
*	enable: enable state, true: enable, false: disable
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
************************************************************/
int32_t mv_mac_learn_enable_set(bool enable)
{
	int32_t ret;

	ret = mv_mac_learn_op_enable_set(enable);
	if (ret) {
		MVMACLEARN_ERR_PRINT("MAC learn enable operation failed\n");
		return MAC_LEARN_FAIL;
	}

	return MAC_LEARN_OK;
}
EXPORT_SYMBOL(mv_mac_learn_enable_set);

/***********************************************************
* mv_mac_learn_enable_get
* API for get MAC leaning enable state
* output:
*	enable: enable state, true: enable, false: disable
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
************************************************************/
int32_t mv_mac_learn_enable_get(bool *enable)
{
	int32_t ret;

	ret = mv_mac_learn_op_enable_get(enable);
	if (ret) {
		MVMACLEARN_ERR_PRINT("MAC learn enable operation failed\n");
		return MAC_LEARN_FAIL;
	}

	return MAC_LEARN_OK;
}
EXPORT_SYMBOL(mv_mac_learn_enable_get);

/***********************************************************
* mv_mac_learn_overwrite_enable_set
* API for set MAC leaning overwrite enable state if FDB is full
* input:
*	enable: enable state, true: enable, false: disable
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
************************************************************/
int32_t mv_mac_learn_overwrite_enable_set(bool enable)
{
	int32_t ret;

	ret = mv_mac_learn_op_overwrite_set(enable);
	if (ret) {
		MVMACLEARN_ERR_PRINT("MAC learn overwrite enable operation failed\n");
		return MAC_LEARN_FAIL;
	}

	return MAC_LEARN_OK;
}
EXPORT_SYMBOL(mv_mac_learn_overwrite_enable_set);

/***********************************************************
* mv_mac_learn_overwrite_enable_get
* API for get MAC leaning overwrite enable state
* output:
*	enable: enable state, true: enable, false: disable
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
************************************************************/
int32_t mv_mac_learn_overwrite_enable_get(bool *enable)
{
	int32_t ret;

	ret = mv_mac_learn_op_overwrite_get(enable);
	if (ret) {
		MVMACLEARN_ERR_PRINT("MAC learn overwrite enable operation failed\n");
		return MAC_LEARN_FAIL;
	}

	return MAC_LEARN_OK;
}
EXPORT_SYMBOL(mv_mac_learn_overwrite_enable_get);

/***********************************************************
* mv_mac_learn_max_count_set
* API for set MAX count of MAC learning entry supported
* input:
*	max_count: max count
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
************************************************************/
int32_t mv_mac_learn_max_count_set(uint32_t max_count)
{
	int32_t ret;
	tpm_db_pnc_range_conf_t range_conf;

	if (tpm_db_pnc_rng_conf_get(TPM_PNC_MAC_LEARN, &range_conf)) {
		MVMACLEARN_ERR_PRINT("TPM PnC range info get failed\n");
		return MAC_LEARN_FAIL;
	}
	if (max_count > (range_conf.api_end - range_conf.api_start + 1)) {
		MVMACLEARN_ERR_PRINT("MAC learn max count larger than PNC range size\n");
		return MAC_LEARN_FAIL;
	}

	if (max_count > MAC_LEARN_FDB_MAX_COUNT)
		max_count = MAC_LEARN_FDB_MAX_COUNT;

	ret = mv_mac_learn_op_max_count_set(max_count);
	if (ret) {
		MVMACLEARN_ERR_PRINT("MAC learn max count set failed\n");
		return MAC_LEARN_FAIL;
	}

	return MAC_LEARN_OK;
}
EXPORT_SYMBOL(mv_mac_learn_max_count_set);

/***********************************************************
* mv_mac_learn_max_count_get
* API for get MAX count of MAC leaning entry supported
* output:
*	max_count: max count
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
************************************************************/
int32_t mv_mac_learn_max_count_get(uint32_t *max_count)
{
	int32_t ret;

	ret = mv_mac_learn_op_max_count_get(max_count);
	if (ret) {
		MVMACLEARN_ERR_PRINT("MAC learn max count get failed\n");
		return MAC_LEARN_FAIL;
	}

	return MAC_LEARN_OK;
}
EXPORT_SYMBOL(mv_mac_learn_max_count_get);

/***********************************************************
* mv_mac_learn_expire_time_set
* API for set expired time of MAC leaning non-static entry
* input:
*	expire_time: expire time, unit: second
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
************************************************************/
int32_t mv_mac_learn_expire_time_set(uint32_t expire_time)
{
	int32_t ret;

	/*input check*/
	if (expire_time == 0) {
		MVMACLEARN_ERR_PRINT("MAC learn expire time invalid(%d)\n", expire_time);
		return MAC_LEARN_FAIL;
	}

	ret = mv_mac_learn_op_expire_set(expire_time);
	if (ret) {
		MVMACLEARN_ERR_PRINT("MAC learn expire time set failed\n");
		return MAC_LEARN_FAIL;
	}

	return MAC_LEARN_OK;
}
EXPORT_SYMBOL(mv_mac_learn_expire_time_set);

/***********************************************************
* mv_mac_learn_expire_time_get
* API for get expired time of MAC leaning non-static entry
* output:
*	expire_time: expire time, unit: second
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
************************************************************/
int32_t mv_mac_learn_expire_time_get(uint32_t *expire_time)
{
	int32_t ret;

	ret = mv_mac_learn_op_expire_get(expire_time);
	if (ret) {
		MVMACLEARN_ERR_PRINT("MAC learn expire time get failed\n");
		return MAC_LEARN_FAIL;
	}

	return MAC_LEARN_OK;
}
EXPORT_SYMBOL(mv_mac_learn_expire_time_get);
