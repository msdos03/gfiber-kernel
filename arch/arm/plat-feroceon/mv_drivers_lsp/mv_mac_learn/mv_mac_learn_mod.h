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
* mv_mac_learn_mod.h
*
* DESCRIPTION:
*
*
*******************************************************************************/
#ifndef __mv_mac_learn_mod_h__
#define __mv_mac_learn_mod_h__

/* MACRO
------------------------------------------------------------------------------*/

#define MAC_LEARN_QUEUE_SIZE 128
#define MAC_LEARN_EX_TIME_DEFAULT 300
#define MAC_LEARN_NON_STATIC_DEFAULT 4

/*Debug info*/
extern uint32_t mac_learn_glob_trace;

#define MAC_LEARN_GLOB_TRACE mac_learn_glob_trace
#define MAC_LEARN_DEBUG_MASK  0x00000001
#define MAC_LEARN_WARN_MASK   0x00000002
#define MAC_LEARN_ERR_MASK    0x00000004

#define MVMACLEARN_DEBUG_PRINT(format , ...) \
	{ \
		if (MAC_LEARN_DEBUG_MASK & MAC_LEARN_GLOB_TRACE) \
			printk("%s(%d):  "format,__FUNCTION__,__LINE__, ##__VA_ARGS__); \
	}

#define MVMACLEARN_WARN_PRINT(format , ...) \
	{ \
		if (MAC_LEARN_WARN_MASK & MAC_LEARN_GLOB_TRACE) \
			printk("%s(%d) WARN:  "format,__FUNCTION__,__LINE__, ##__VA_ARGS__); \
	}

#define MVMACLEARN_ERR_PRINT(format, ...)     printk("%s(%d) ERROR:  "format,__FUNCTION__,__LINE__, ##__VA_ARGS__)

/* Typedefs
------------------------------------------------------------------------------*/
typedef enum
{
	MAC_LEARN_TPM_API_LOCK_ADD,
	MAC_LEARN_TPM_API_LOCK_DEL,
	MAC_LEARN_TPM_API_LOCK_MAX
} mac_learn_tpm_lock_t;


typedef struct {
	char mac_addr_entry[MV_MAC_ADDR_SIZE];
}src_mac_addr_t;

typedef struct {
	struct semaphore queue_sem;
	src_mac_addr_t mc_src_mac[MAC_LEARN_QUEUE_SIZE];
	uint32_t wr_idx;
	uint32_t rd_idx;
	bool queue_empty;
	bool queue_full;
}src_mac_queue_t;

typedef struct {
	volatile bool mac_learn_enable;
	bool mac_learn_overwrite_enable;/*Used to control the action after full */
	uint32_t mac_learn_max_count;/*Non-static max count*/
	uint32_t mac_learn_expire_time;
	uint32_t mac_learn_overwrite_time;
	bool queue_init;
	struct workqueue_struct *mac_learn_queue;
	struct work_struct mac_learn_work;
	src_mac_queue_t sc_mac_queue;/*Used to store MAC address from network*/
	struct timer_list mac_learn_timer;
	spinlock_t tpm_api_lock[MAC_LEARN_TPM_API_LOCK_MAX];
} mac_learn_t;

/* Function protype
------------------------------------------------------------------------------*/
int mac_learn_queue_work_insert(void);
void mac_learn_tpm_api_lock_get(mac_learn_tpm_lock_t lock_type);
void mac_learn_tpm_api_lock_release(mac_learn_tpm_lock_t lock_type);
int mv_mac_learn_queue_write(char *src_mac);
int mv_mac_learn_queue_read(char *src_mac);
int mv_mac_learn_op_enable_set(bool mac_learn_enable);
int mv_mac_learn_op_enable_get(bool *mac_learn_enable);
int mv_mac_learn_op_overwrite_set(bool overwrite_enable);
int mv_mac_learn_op_overwrite_get(bool *overwrite_enable);
int mv_mac_learn_op_max_count_set(uint32_t mac_learn_max_count);
int mv_mac_learn_op_max_count_get(uint32_t *mac_learn_max_count);
int mv_mac_learn_op_expire_set(uint32_t mac_learn_expire_time);
int mv_mac_learn_op_expire_get(uint32_t *mac_learn_expire_time);
int mv_mac_learn_op_trace_level_set(uint32_t trace_level);


#endif /* __mv_mac_learn_mod_h__ */
