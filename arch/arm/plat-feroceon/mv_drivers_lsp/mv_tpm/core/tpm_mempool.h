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
**  FILE        : tpm_mempool.h                                              **
**                                                                           **
**  DESCRIPTION : This file tpm memory pool manager                          **
******************************************************************************/
#ifndef _TPM_MEMPOOL_H
#define _TPM_MEMPOOL_H

#include <linux/spinlock.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct tpm_common_mempool tpm_common_mempool_t;
typedef struct tpm_common_mpool_hdr {
	struct tpm_common_mpool_hdr *next;
	tpm_common_mempool_t *pool;
	uint32_t flags;
	uint32_t magic;
} tpm_common_mpool_hdr_t;

typedef struct {
	uint32_t magic;
	tpm_common_mempool_t *pool;
} tpm_common_mpool_ftr_t;

struct tpm_common_mempool {
	tpm_common_mpool_hdr_t *head;
	tpm_common_mpool_hdr_t *tail;
	uint32_t number_of_free_entries;
	spinlock_t lock;
	uint32_t data_size;	/* size of data section in pool entry */
	uint32_t pool_entry_size;	/* size of pool entry */
	/* parameters passed on init */
	uint32_t number_of_entries;
	uint32_t entry_size;
	uint8_t *mem;
};

void *tpm_common_mempool_alloc(tpm_common_mempool_t *pool);
void tpm_common_mempool_free(tpm_common_mempool_t *pool, void *mem);
tpm_common_mempool_t *tpm_common_mempool_get(void *mem);
tpm_common_mempool_t *tpm_common_mempool_create(uint32_t number_of_entries, uint32_t entry_size,
						int32_t priority);
void tpm_common_mempool_destroy(tpm_common_mempool_t *pool);
int32_t tpm_common_mempool_get_number_of_free_entries(tpm_common_mempool_t *pool);
int32_t tpm_common_mempool_get_number_of_entries(tpm_common_mempool_t *pool);
int32_t tpm_common_mempool_get_entry_size(tpm_common_mempool_t *pool);

#define TPM_COMMON_MPOOL_HDR_FLAGS_ALLOCATED	0x00000001
#define TPM_COMMON_MPOOL_HDR_MAGIC		0xa5a5a508
#define TPM_COMMON_MPOOL_FTR_MAGIC		0xa5a5a509
#define TPM_COMMON_MPOOL_ALIGN4(size)		((size)+4) & 0xFFFFFFFC;
#define TPM_COMMON_MPOOL_CHECK_ALIGNED4(ptr)	((((uint32_t)(ptr)) & 0x00000003) == 0)

#ifdef __cplusplus
}
#endif
#endif				/* _TPM_MEMPOOL_H */
