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
* mv_mac_learn_db.h
*
* DESCRIPTION:
*
*
*******************************************************************************/
#ifndef __mv_mac_learn_db_h__
#define __mv_mac_learn_db_h__

/* MACRO
------------------------------------------------------------------------------*/
#define MAC_LEARN_FDB_MAX_COUNT 32

#define MAC_LEARN_FDB_OK 0
#define MAC_LEARN_FDB_FAIL 1

#define MAC_LEARN_FDB_INVALID 0xDE
#define MAC_LEARN_FDB_VALID 0x01
#define MAC_LEARN_FDB_RESERVED 0xCE

#define MAC_LEARN_FDB_STATIC 0x0
#define MAC_LEARN_FDB_NON_STATIC 0x1

#define MAC_LEARN_FDB_INVALID_PNC 0xFFFF

/* Typedefs
------------------------------------------------------------------------------*/
typedef struct {
	uint8_t valid;
	uint8_t state;
	uint8_t port;
	uint8_t src_mac_addr[6];
	uint32_t pnc_idx;
	uint32_t hit_count;
	uint32_t no_hit_time;/* unit:s */
} mac_learn_db_entry_t;

typedef struct {
	bool mac_learn_fdb_init;/* indicate whether db is init or not */
	atomic_t fdb_valid_count;
	atomic_t fdb_non_static_count;
	spinlock_t mac_learn_db_entry_lock[MAC_LEARN_FDB_MAX_COUNT];
	mac_learn_db_entry_t mac_learn_db_entry[MAC_LEARN_FDB_MAX_COUNT];
} mac_learn_fdb_t;

/* Function protype
------------------------------------------------------------------------------*/
int32_t mac_learn_db_nohit_time_clear(void);
int32_t mac_learn_db_reset(void);
int32_t mac_learn_db_nonstatic_clear(void);
int32_t mac_learn_db_no_hit_time_get(uint32_t fdb_idx, uint32_t *no_hit_time);
void mac_learn_db_no_time_clear_update_hit(uint32_t fdb_idx, uint32_t hit_count);
void mac_learn_db_no_hit_time_inc_read(uint32_t fdb_idx, mac_learn_db_entry_t *mac_learn_db_entry);
int32_t mac_learn_db_nonstatic_pnc_get_by_fdbidx(uint32_t fdb_idx, uint32_t *pnc_idx, uint32_t *hit_count);
int32_t mac_learn_db_find_mac_addr(tpm_l2_acl_key_t *src_mac_key, int32_t *addr_exist, mac_learn_db_entry_t *mac_learn_db_entry);
int32_t mac_learn_db_pncidx_update(uint8_t rule_add, uint32_t pnc_index);
int32_t mac_learn_db_free_inx_get(uint32_t *free_idx);
int32_t mac_learn_db_entry_info_by_pncidx(uint32_t pnc_idx, uint32_t *fdb_idx, mac_learn_db_entry_t *mac_learn_db_entry);
int32_t mac_learn_db_entry_add(mac_learn_db_entry_t *mac_learn_db_entry);
int32_t mac_learn_db_entry_del(uint32_t del_idx);
int32_t mac_learn_db_valid_count_get(uint32_t *valid_ount);
void mac_learn_db_nonstatic_count_inc(void);
void mac_learn_db_nonstatic_count_dec(void);
int32_t mac_learn_db_nonstatic_count_get(uint32_t *nonstatic_ount);
void mac_learn_db_valid_print(void);
int32_t mac_learn_fdb_init(void);

#endif