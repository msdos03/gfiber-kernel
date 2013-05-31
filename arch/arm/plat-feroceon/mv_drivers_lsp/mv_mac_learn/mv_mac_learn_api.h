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
* mv_mac_learn_api.h
*
* DESCRIPTION:
*
*
*******************************************************************************/
#ifndef __mv_mac_learn_api_h__
#define __mv_mac_learn_api_h__

/* MACRO
------------------------------------------------------------------------------*/
#define MAC_LEARN_OK 0
#define MAC_LEARN_FAIL 1

/* CPU rate limit related*/
#define MAC_LEARN_RATE_LIMIT 128
#define MAC_LEARN_BUCKET_SIZE 2000

/* Function protype
------------------------------------------------------------------------------*/

/***********************************************************
* mv_mac_learn_static_entry_add
* API for adding static learn entry
* input:
*	static_mac_addr: source MAC address
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
***********************************************************/
int32_t mv_mac_learn_static_entry_add(char *static_mac_addr);


/***********************************************************
* mv_mac_learn_static_entry_del
* API for del static learn entry
* input:
*	static_mac_addr: source MAC address
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
************************************************************/
int32_t mv_mac_learn_static_entry_del(char *static_mac_addr);

/***********************************************************
* mv_mac_learn_enable_set
* API for set MAC leaning enable state
* input:
*	enable: enable state, true: enable, false: disable
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
************************************************************/
int32_t mv_mac_learn_enable_set(bool enable);

/***********************************************************
* mv_mac_learn_enable_get
* API for get MAC leaning enable state
* output:
*	enable: enable state, true: enable, false: disable
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
************************************************************/
int32_t mv_mac_learn_enable_get(bool *enable);

/***********************************************************
* mv_mac_learn_overwrite_enable_set
* API for set MAC leaning overwrite enable state if FDB is full
* input:
*	enable: enable state, true: enable, false: disable
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
************************************************************/
int32_t mv_mac_learn_overwrite_enable_set(bool enable);

/***********************************************************
* mv_mac_learn_overwrite_enable_get
* API for get MAC leaning overwrite enable state
* output:
*	enable: enable state, true: enable, false: disable
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
************************************************************/
int32_t mv_mac_learn_overwrite_enable_get(bool *enable);

/***********************************************************
* mv_mac_learn_max_count_set
* API for set MAX count of MAC learning entry supported
* input:
*	max_count: max count
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
************************************************************/
int32_t mv_mac_learn_max_count_set(uint32_t max_count);

/***********************************************************
* mv_mac_learn_max_count_get
* API for get MAX count of MAC leaning entry supported
* output:
*	max_count: max count
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
************************************************************/
int32_t mv_mac_learn_max_count_get(uint32_t *max_count);

/***********************************************************
* mv_mac_learn_expire_time_set
* API for set expired time of MAC leaning non-static entry
* input:
*	expire_time: expire time, unit: second
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
************************************************************/
int32_t mv_mac_learn_expire_time_set(uint32_t expire_time);

/***********************************************************
* mv_mac_learn_expire_time_get
* API for get expired time of MAC leaning non-static entry
* output:
*	expire_time: expire time, unit: second
* return:
*	success--MAC_LEARN_OK
*	failed --MAC_LEARN_FAIL
************************************************************/
int32_t mv_mac_learn_expire_time_get(uint32_t *expire_time);

#endif
