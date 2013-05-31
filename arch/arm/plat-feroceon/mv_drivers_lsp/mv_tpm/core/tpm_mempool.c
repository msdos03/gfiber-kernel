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

/******************************************************************************
**  FILE        : tpm_mempool.c                                              **
**                                                                           **
**  DESCRIPTION : This file tpm memory pool manager                          **
******************************************************************************/
#include <linux/slab.h>
#include "tpm_mempool.h"

bool tpm_common_mempool_check_internal(tpm_common_mempool_t *pool,
				       void *ptr,
				       tpm_common_mpool_hdr_t *hdr,
				       tpm_common_mpool_ftr_t *ftr)
{
	if (!ptr) {
		printk(KERN_ERR "illegal ptr NULL");
		return false;
	}

	if (!TPM_COMMON_MPOOL_CHECK_ALIGNED4(ptr)) {
		printk(KERN_ERR "ptr not aligned %p", ptr);
		return false;
	}

	if (hdr->magic != TPM_COMMON_MPOOL_HDR_MAGIC) {
		printk(KERN_ERR "illegal hdr magic %x for ptr %p", hdr->magic, ptr);
		return false;
	}

	if (ftr->magic != TPM_COMMON_MPOOL_FTR_MAGIC) {
		printk(KERN_ERR "illegal ftr magic %x for ptr %p", ftr->magic, ptr);
		return false;
	}

	if (hdr->pool != pool || ftr->pool != pool) {
		printk(KERN_ERR "inconsistent size hdr->pool: %p ftr->pool: %p for ptr %p", hdr->pool, ftr->pool, ptr);
		return false;
	}

	if (!(hdr->flags & TPM_COMMON_MPOOL_HDR_FLAGS_ALLOCATED)) {
		printk(KERN_ERR "ptr %p was not allocated", ptr);
		return false;
	}
	return true;
}

void *tpm_common_mempool_alloc(tpm_common_mempool_t * pool)
{
	tpm_common_mpool_hdr_t *hdr;

	if (!pool || !pool->head || pool->number_of_free_entries == 0)
		return NULL;

	spin_lock_bh(&pool->lock);
	hdr = pool->head;
	pool->head = pool->head->next;

	if (!pool->head)
		pool->tail = NULL;

	hdr->flags = TPM_COMMON_MPOOL_HDR_FLAGS_ALLOCATED;
	pool->number_of_free_entries--;
	spin_unlock_bh(&pool->lock);
	return ((uint8_t *) hdr + sizeof(tpm_common_mpool_hdr_t));
}

void tpm_common_mempool_free(tpm_common_mempool_t *pool, void *ptr)
{
	tpm_common_mpool_hdr_t *hdr;
	tpm_common_mpool_ftr_t *ftr;

	if (!pool || !ptr)
		return;

	if (!TPM_COMMON_MPOOL_CHECK_ALIGNED4(ptr)) {
		printk(KERN_ERR "ptr not aligned %p", ptr);
		return;
	}
	spin_lock_bh(&pool->lock);
	hdr = (tpm_common_mpool_hdr_t *) ((uint8_t *) ptr - sizeof(tpm_common_mpool_hdr_t));
	ftr = (tpm_common_mpool_ftr_t *) ((uint8_t *) ptr + pool->data_size);

	if (!tpm_common_mempool_check_internal(pool, ptr, hdr, ftr)) {
		printk(KERN_ERR "invalid ptr %p", ptr);
		spin_unlock_bh(&pool->lock);
		return;
	}

	hdr->flags ^= TPM_COMMON_MPOOL_HDR_FLAGS_ALLOCATED;
	hdr->next = NULL;

	if (!pool->head)
		pool->head = pool->tail = hdr;
	else {
		pool->tail->next = hdr;
		pool->tail = hdr;
	}

	pool->number_of_free_entries++;
	spin_unlock_bh(&pool->lock);
}

tpm_common_mempool_t *tpm_common_mempool_create(uint32_t number_of_entries,
						uint32_t entry_size,
						int32_t priority)
{
	uint32_t i;
	uint32_t aligned_entry_size;
	uint32_t pool_entry_size;
	tpm_common_mpool_hdr_t *hdr;
	tpm_common_mpool_hdr_t *next_hdr;
	tpm_common_mpool_ftr_t *ftr;
	tpm_common_mempool_t *pool;

	aligned_entry_size = TPM_COMMON_MPOOL_ALIGN4(entry_size);
	pool_entry_size =
	    TPM_COMMON_MPOOL_ALIGN4(sizeof(tpm_common_mpool_hdr_t) + aligned_entry_size +
				    sizeof(tpm_common_mpool_ftr_t));
	pool = kmalloc((sizeof(tpm_common_mempool_t) + pool_entry_size * number_of_entries), priority);

	if (!pool)
		return NULL;

	pool->entry_size = entry_size;
	pool->number_of_entries = number_of_entries;
	pool->data_size = aligned_entry_size;
	pool->pool_entry_size = pool_entry_size;
	pool->number_of_free_entries = number_of_entries;
	pool->mem = (uint8_t *) (pool + 1);
	pool->head = (tpm_common_mpool_hdr_t *) pool->mem;
	spin_lock_init(&pool->lock);

	for (i = 0; i < number_of_entries; i++) {
		hdr = (tpm_common_mpool_hdr_t *) &pool->mem[pool_entry_size * i];
		ftr =
		    (tpm_common_mpool_ftr_t *) ((uint8_t *) hdr + sizeof(tpm_common_mpool_hdr_t) + aligned_entry_size);
		hdr->magic = TPM_COMMON_MPOOL_HDR_MAGIC;
		hdr->pool = pool;
		hdr->flags = 0;
		ftr->magic = TPM_COMMON_MPOOL_FTR_MAGIC;
		ftr->pool = pool;

		if (i < (number_of_entries - 1))
			next_hdr = (tpm_common_mpool_hdr_t *) &pool->mem[pool_entry_size * (i + 1)];
		else {
			pool->tail = hdr;
			next_hdr = NULL;
		}

		hdr->next = next_hdr;
	}
	return pool;
}

void tpm_common_mempool_destroy(tpm_common_mempool_t *pool)
{
	if (!pool)
		return;

	kfree(pool);
}

int32_t tpm_common_mempool_get_number_of_free_entries(tpm_common_mempool_t *pool)
{
	if (!pool)
		return -1;

	return (int32_t) pool->number_of_free_entries;
}

int32_t tpm_common_mempool_get_number_of_entries(tpm_common_mempool_t *pool)
{
	if (!pool)
		return -1;

	return (int32_t) pool->number_of_entries;
}

int32_t tpm_common_mempool_get_entry_size(tpm_common_mempool_t *pool)
{
	if (!pool)
		return -1;

	return (int32_t) pool->entry_size;
}

tpm_common_mempool_t *tpm_common_mempool_get(void *ptr)
{
	tpm_common_mpool_hdr_t *hdr;
	tpm_common_mpool_ftr_t *ftr;

	if (!ptr)
		return NULL;

	if (!TPM_COMMON_MPOOL_CHECK_ALIGNED4(ptr))
		return NULL;

	hdr = (tpm_common_mpool_hdr_t *) ((uint8_t *) ptr - sizeof(tpm_common_mpool_hdr_t));

	if (!hdr)
		return NULL;

	ftr = (tpm_common_mpool_ftr_t *) ((uint8_t *) ptr + hdr->pool->data_size);

	if (hdr->magic != TPM_COMMON_MPOOL_HDR_MAGIC) {
		printk(KERN_ERR "illegal hdr magic %x for ptr %p", hdr->magic, ptr);
		return NULL;
	}
	if (ftr->magic != TPM_COMMON_MPOOL_FTR_MAGIC) {
		printk(KERN_ERR "illegal ftr magic %x for ptr %p", ftr->magic, ptr);
		return NULL;
	}
	if ((!hdr->pool) || (hdr->pool != ftr->pool)) {
		printk(KERN_ERR "inconsistent size hdr->pool: %p ftr->pool: %p for ptr %p", hdr->pool, ftr->pool, ptr);
		return false;
	}
	return hdr->pool;
}
