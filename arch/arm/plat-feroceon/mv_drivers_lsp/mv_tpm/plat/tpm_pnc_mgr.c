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

#include "tpm_common.h"
#include "tpm_header.h"

#define TPM_PNC_ERR_ON_OOR(cond) if (cond){TPM_OS_ERROR(TPM_PNC_HM_MOD, "%s(%d): out of range\n",__FUNCTION__,__LINE__); return TPM_FAIL;}
#define TPM_PNC_ERR_ON_OOM(cond) if (cond){TPM_OS_ERROR(TPM_PNC_HM_MOD, "%s(%d): out of memory\n",__FUNCTION__,__LINE__);return TPM_FAIL;}
#define DWORD_LEN       32


/* Global  Variables */
spinlock_t tpmPncLock;



//static    unsigned long     flags;
//static    unsigned long     r_jiffies, t_jiffies;


/*******************************************************************************
* tpm_pnc_fill_in_entry()
*
* DESCRIPTION:  This function moves HW PnC entry in the internal format
*
* INPUTS:
*           tcam_entry  - HW PnC entry
*
* OUTPUTS:
*           pnc_entry   - PnC entry
*
* RETURNS:
*           None
*
* COMMENTS:
*           None
*
*******************************************************************************/
void tpm_pnc_print_sw_entry (tpm_pnc_all_t   *pnc_entry)
{
    int     i;

    printk("*****************************************************************************************\n");
    printk("TCAM:  LU  Port Add info    Packet\n");
    printk("=========================================================================================\n");
    printk("       %2.2d   %2.2d   %2.2x         ",
           pnc_entry->tcam_entry.lu_id,
           pnc_entry->tcam_entry.port_ids,
           pnc_entry->tcam_entry.add_info_data);

    for (i=0; i < TPM_TCAM_PKT_WIDTH; i++) {
        printk("%2.2x", pnc_entry->tcam_entry.pkt_data.pkt_byte[i]);
    }
    printk("\n                 %2.2x         ", pnc_entry->tcam_entry.add_info_mask);
    for (i=0; i < TPM_TCAM_PKT_WIDTH; i++) {
        printk("%2.2x", pnc_entry->tcam_entry.pkt_mask.pkt_byte[i]);
    }

    printk("\n");
    printk("================================================================================\n");
    printk("SRAM:  NLU  Add info  LU Done  NLU idx   Q      Shift(val)   RI         FL      \n");
    printk("================================================================================\n");
    printk("       %2.2d      %2.2x        %1.1x        %2.2x     %4.4x    %3.3x(%3.3x)    %2.2x%4.4x     %8.8x\n",
           pnc_entry->sram_entry.next_lu_id,
           pnc_entry->sram_entry.add_info_data,
           pnc_entry->sram_entry.lookup_done,
           pnc_entry->sram_entry.next_lu_off_reg,
           pnc_entry->sram_entry.pnc_queue,
           pnc_entry->sram_entry.shift_updt_reg,
           pnc_entry->sram_entry.shift_updt_val,
           pnc_entry->sram_entry.res_info_23_16_data,
           pnc_entry->sram_entry.res_info_15_0_data,
           pnc_entry->sram_entry.flowid_val);
    printk("               %2.2x                                            %2.2x%4.4x     %8.8x\n",
           pnc_entry->sram_entry.add_info_mask,
           pnc_entry->sram_entry.res_info_23_16_mask,
           pnc_entry->sram_entry.res_info_15_0_mask,
           pnc_entry->sram_entry.flowid_updt_mask);
    printk("*****************************************************************************************\n");
}

/*******************************************************************************
* tpm_pnc_fill_in_entry()
*
* DESCRIPTION:  This function moves HW PnC entry in the internal format
*
* INPUTS:
*           tcam_entry  - HW PnC entry
*
* OUTPUTS:
*           pnc_entry   - PnC entry
*
* RETURNS:
*           None
*
* COMMENTS:
*           None
*
*******************************************************************************/
void tpm_pnc_fill_in_entry(struct tcam_entry  *tcamEntry, tpm_pnc_all_t *pnc_entry)
{
    int         i, word, shift;
    uint32_t    val, mask;

    /* Fill in TCAM data */
    pnc_entry->tcam_entry.lu_id         = (tcamEntry->data.u.word[LU_WORD]   >> LU_OFFS) & LU_MASK;
    pnc_entry->tcam_entry.port_ids      = (tcamEntry->data.u.word[PORT_WORD] >> PORT_OFFS) & PORT_MASK;
    pnc_entry->tcam_entry.add_info_data = (tcamEntry->data.u.word[AI_WORD]   >> AI_OFFS) & AI_MASK;
    pnc_entry->tcam_entry.add_info_mask = (tcamEntry->mask.u.word[AI_WORD]   >> AI_OFFS) & AI_MASK;

    for (i = 0; i < TPM_TCAM_PKT_WIDTH; i++)
    {
        pnc_entry->tcam_entry.pkt_data.pkt_byte[i] = tcamEntry->data.u.byte[i];
        pnc_entry->tcam_entry.pkt_mask.pkt_byte[i] = tcamEntry->mask.u.byte[i];
    }

    /* Fill in SRAM data */
    word                                   = LU_ID_OFFS / DWORD_LEN;
    shift                                  = LU_ID_OFFS % DWORD_LEN;
    pnc_entry->sram_entry.next_lu_id       = (tcamEntry->sram.word[word] >> shift) & LU_MASK;

    word                                   = LU_DONE_OFFS / DWORD_LEN;
    shift                                  = LU_DONE_OFFS % DWORD_LEN;
    pnc_entry->sram_entry.lookup_done      = (tcamEntry->sram.word[word] >> shift) & 0x1;

    word                                   = RXQ_QUEUE_OFFS / DWORD_LEN;
    shift                                  = RXQ_QUEUE_OFFS % DWORD_LEN;
    pnc_entry->sram_entry.pnc_queue        = (tcamEntry->sram.word[word] >> shift) & RXQ_MASK;

    word                                   = NEXT_LU_SHIFT_OFFS / DWORD_LEN;
    shift                                  = NEXT_LU_SHIFT_OFFS % DWORD_LEN;
    pnc_entry->sram_entry.next_lu_off_reg  = (tcamEntry->sram.word[word] >> shift) & SHIFT_IDX_MASK;

    word                                   = SHIFT_IDX_OFFS / DWORD_LEN;
    shift                                  = SHIFT_IDX_OFFS % DWORD_LEN;
    pnc_entry->sram_entry.shift_updt_reg   = (tcamEntry->sram.word[word] >> shift) & SHIFT_IDX_MASK;

    word                                   = SHIFT_VAL_OFFS/DWORD_LEN;
    shift                                  = SHIFT_VAL_OFFS%DWORD_LEN;
    pnc_entry->sram_entry.shift_updt_val   = (tcamEntry->sram.word[word] >> shift) & SHIFT_VAL_MASK;

    word                                   = AI_VALUE_OFFS/DWORD_LEN;
    shift                                  = AI_VALUE_OFFS%DWORD_LEN;
    pnc_entry->sram_entry.add_info_data    = (tcamEntry->sram.word[word] >> shift) & AI_MASK;

    word                                   = AI_MASK_OFFS/DWORD_LEN;
    shift                                  = AI_MASK_OFFS%DWORD_LEN;
    pnc_entry->sram_entry.add_info_mask    = (tcamEntry->sram.word[word] >> shift) & AI_MASK;

    pnc_entry->sram_entry.flowid_updt_mask = 0;

    if (tcamEntry->sram.word[1] & FLOW_ID_LSW_MASK)
        pnc_entry->sram_entry.flowid_updt_mask |= FLOW_ID_LSW_MASK;

    if (tcamEntry->sram.word[1] & FLOW_ID_MSW_MASK)
        pnc_entry->sram_entry.flowid_updt_mask |= FLOW_ID_MSW_MASK;

    pnc_entry->sram_entry.flowid_val = tcamEntry->sram.word[0];

    word  = RI_VALUE_OFFS/DWORD_LEN;
    shift = RI_VALUE_OFFS%DWORD_LEN;
    val   = (tcamEntry->sram.word[word] >> shift) & RI_MASK;

    /* RI mask : word 1 - 6 bits (25 - 31)
                 word 2 - 18 bits (0 - 17) */
    word  = RI_MASK_OFFS/DWORD_LEN;
    shift = RI_MASK_OFFS%DWORD_LEN;
    mask  = (tcamEntry->sram.word[word] >> shift) & RI_MASK;
    word++;
    mask |= (tcamEntry->sram.word[word] & 0x3FFFF) << 6;

    pnc_entry->sram_entry.res_info_15_0_data  =  val  & RES_INFO_BITS_16_MASK;
    pnc_entry->sram_entry.res_info_15_0_mask  =  mask & RES_INFO_BITS_16_MASK;
    pnc_entry->sram_entry.res_info_23_16_data = (val  & RES_INFO_BITS_8_MASK) >> MAX_RES_INFO_BITS_16;
    pnc_entry->sram_entry.res_info_23_16_mask = (mask & RES_INFO_BITS_8_MASK) >> MAX_RES_INFO_BITS_16;

}


/*******************************************************************************
* tpm_pnc_hw_entry_inv()
*
* DESCRIPTION: This API invalidates a TCAM entry in a range.
*              It does not change any other entry in the TCAM.
*
* INPUTS:   entry   - entry to be invalidated
*
* OUTPUTS:
*           None
*
* RETURNS:
*           TPM_OK or TPK_FAIL
*
* COMMENTS:
*           None
*
*******************************************************************************/
static inline void tpm_pnc_hw_entry_inv(uint32_t entry)
{
    unsigned long     flags;

    /* Lock the PnC */
    spin_lock_irqsave(&tpmPncLock, flags);

    /* Invalidate entry */
    tcam_hw_inv(entry);

    /* Unlock PnC */
    spin_unlock_irqrestore(&tpmPncLock, flags);

    return;
}

static inline void tpm_pnc_hw_entry_read(struct tcam_entry *te, uint32_t entry)
{
    unsigned long     flags;

    /* Lock the PnC */
    spin_lock_irqsave(&tpmPncLock, flags);

    /* Read entry */
    tcam_hw_read(te, entry);

    /* Unlock PnC */
    spin_unlock_irqrestore(&tpmPncLock, flags);

    return;
}


static inline void tpm_pnc_hw_entry_write(struct tcam_entry *te, uint32_t entry)
{
    unsigned long     flags;

    /* Lock the PnC */
    spin_lock_irqsave(&tpmPncLock, flags);

    /* Write entry */
    tcam_hw_write(te, entry);

    /* Unlock PnC */
    spin_unlock_irqrestore(&tpmPncLock, flags);

    return;
}

/*******************************************************************************
* tpm_pnc_set()
*
* DESCRIPTION:  This API provides a generic interface to set a complete PnC
*               entry (TCAM + SRAM), or just update the SRAM.
*               If (TCAM and SRAM) are updated, PnC entry is first invalidated
*               and only then configured.
*
* INPUTS:
*           entry       - entry to be set/updated
*           sram_update - 0 - set a complete PnC entry
*                         1 - just SRAM update
*           pnc_entry   - pointer to the set/updated PnC entry
*
* OUTPUTS:
*           None
*
* RETURNS:
*           TPM_OK or TPM_FAIL
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t    tpm_pnc_set  (uint32_t      entry,
                         uint32_t      sram_update,
                         tpm_pnc_all_t *pnc_entry)
{
    struct tcam_entry tcamEntry;
    uint32_t          sram_res_info_data = 0;
    int               indx;
    uint32_t          cfg_pnc_parse_flag = 0;


	cfg_pnc_parse_flag = mv_eth_ctrl_pnc_get();

    if (cfg_pnc_parse_flag == 1) {
        TPM_OS_DEBUG(TPM_PNC_HM_MOD, " TPM is NOT configuring the PNC - MV driver is responsible \n");
        return (TPM_OK);
    }
    TPM_OS_DEBUG( TPM_PNC_HM_MOD, "entry(%d) sram_update(%d)\n",entry, sram_update);
    TPM_PNC_ERR_ON_OOR(entry >= TPM_PNC_SIZE);
    TPM_PNC_ERR_ON_OOM(pnc_entry == NULL);

    /* sram_update is equal to 0, therefore the PnC entry is set */
    if (!sram_update)
    {
        /* Update TCAM */

        /* Invalidate PnC entry */
        tpm_pnc_hw_entry_inv(entry);

        /* Clear SW TCAM entry */
        tcam_sw_clear(&tcamEntry);

        /* Set lookup */
        if (pnc_entry->tcam_entry.lu_id != TPM_PNC_ANY_LUID)
        {
            tcam_sw_set_lookup(&tcamEntry, pnc_entry->tcam_entry.lu_id);
        }

        /* Set port */
        tcam_sw_set_port(&tcamEntry, 0, PORT_MASK &(~(pnc_entry->tcam_entry.port_ids)));

        /* Set TCAM add info */
        tcam_sw_set_ainfo(&tcamEntry, MASK_AI(pnc_entry->tcam_entry.add_info_data),
                                      MASK_AI(pnc_entry->tcam_entry.add_info_mask));

        /* Set packet data */
        for (indx = 0; indx < TPM_TCAM_PKT_WIDTH; indx++)
        {
            tcam_sw_set_byte(&tcamEntry, indx, pnc_entry->tcam_entry.pkt_data.pkt_byte[indx]);
            tcam_sw_set_mask(&tcamEntry, indx, pnc_entry->tcam_entry.pkt_mask.pkt_byte[indx]);
        }
    }

    /* sram_update is not equal to 0, therefore SRAM of the PnC entry is updated, read PnC entry */
    else
    {
        /* Clear SW TCAM entry */
        tcam_sw_clear(&tcamEntry);

        /* Read the entry from HW */
        tpm_pnc_hw_entry_read(&tcamEntry, entry);

		/* Clear old sram entry */
		sram_sw_clear(&tcamEntry.sram);
    }

    /* Update SRAM */

    /* Set next lookup ID */
    sram_sw_set_next_lookup(&tcamEntry, pnc_entry->sram_entry.next_lu_id);

    /* Set add info update */
    sram_sw_set_ainfo(&tcamEntry, MASK_AI(pnc_entry->sram_entry.add_info_data), MASK_AI(pnc_entry->sram_entry.add_info_mask));

    /* Set lookup done */
    sram_sw_set_lookup_done(&tcamEntry, pnc_entry->sram_entry.lookup_done);

    /* Set Next lookup offset index */
    sram_sw_set_next_lookup_shift(&tcamEntry, pnc_entry->sram_entry.next_lu_off_reg);

    /* Set queue */
    if (pnc_entry->sram_entry.pnc_queue != 0xFFFF)
    {
        sram_sw_set_rxq(&tcamEntry, pnc_entry->sram_entry.pnc_queue, 1);
    }

    /* Set shift update */
    sram_sw_set_shift_update(&tcamEntry, pnc_entry->sram_entry.shift_updt_reg,
                                         pnc_entry->sram_entry.shift_updt_val);

    /* Set results info (RI) */
    sram_res_info_data = ((pnc_entry->sram_entry.res_info_23_16_data << 16) |
                           pnc_entry->sram_entry.res_info_15_0_data);
    sram_sw_set_rinfo(&tcamEntry, sram_res_info_data, sram_res_info_data);
    /* if need to set marvell header, update them all */
    if (sram_res_info_data & TPM_MH_RI_BIT14 ||
        sram_res_info_data & TPM_MH_RI_BIT15 ||
        sram_res_info_data & TPM_MH_RI_BIT16 ||
        sram_res_info_data & TPM_MH_RI_BIT17)
        tcamEntry.sram.word[RI_MASK_OFFS / DWORD_LEN] |= (1 << ((TPM_PNC_RI_MH_BIT_14 + RI_MASK_OFFS) % DWORD_LEN)) |
                                                         (1 << ((TPM_PNC_RI_MH_BIT_15 + RI_MASK_OFFS) % DWORD_LEN)) |
                                                         (1 << ((TPM_PNC_RI_MH_BIT_16 + RI_MASK_OFFS) % DWORD_LEN)) |
                                                         (1 << ((TPM_PNC_RI_MH_BIT_17 + RI_MASK_OFFS) % DWORD_LEN));

    /* Set flow id */
    sram_sw_set_flowid(&tcamEntry, pnc_entry->sram_entry.flowid_val, pnc_entry->sram_entry.flowid_updt_mask);

    /* Write the current entry to HW */
    tpm_pnc_hw_entry_write(&tcamEntry, entry);

    return TPM_OK;
}




/*******************************************************************************
* tpm_pnc_entry_inv()
*
* DESCRIPTION: This API invalidates a TCAM entry in a range.
*              It does not change any other entry in the TCAM.
*
* INPUTS:   entry   - entry to be invalidated
*
* OUTPUTS:
*           None
*
* RETURNS:
*           TPM_OK or TPK_FAIL
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t tpm_pnc_entry_inv(uint32_t entry)
{
    uint32_t          cfg_pnc_parse_flag = 0;

	cfg_pnc_parse_flag = mv_eth_ctrl_pnc_get();

    if (cfg_pnc_parse_flag == 1) {
        TPM_OS_DEBUG(TPM_PNC_HM_MOD, " TPM is NOT invalidating the PNC - MV driver is responsible \n");
        return (TPM_OK);
    }

    TPM_OS_DEBUG( TPM_PNC_HM_MOD, "entry(%d)\n",entry);
    TPM_PNC_ERR_ON_OOR(entry >= TPM_PNC_SIZE);

    /* Invalidate PnC entry */
    tpm_pnc_hw_entry_inv(entry);

    return TPM_OK;
}


void tpm_pnc_entry_copy(uint32_t read_entry, uint32_t write_entry)
{
    struct tcam_entry tcamEntry;
    uint32_t pnc_cgrp, lu_mask;

    /* Clear SW TCAM entry */
    tcam_sw_clear(&tcamEntry);

    /* Read entry from HW */
    tpm_pnc_hw_entry_read(&tcamEntry, read_entry);

    /* Read PNC counter to get group and LU kask*/
    pnc_cgrp = tpm_tcam_get_cntr_group(read_entry);
    lu_mask = tpm_tcam_get_lu_mask(read_entry);

    /* Write entry to HW */
    tpm_pnc_hw_entry_write(&tcamEntry, write_entry);

    /* Write PNC reg for counter group and LU mask */
    tpm_tcam_set_lu_mask(write_entry, (int32_t) lu_mask);
    tpm_tcam_set_cntr_group(write_entry, (int32_t) pnc_cgrp);
}



/*******************************************************************************
* tpm_pnc_entry_insert()
*
* DESCRIPTION: This API inserts an invalid empty PnC entry.
*              It 'pushes down' a block of TCAM entries following the inserted entry.
*
* INPUTS:      entry        - entry to be inserted
*              last_entry   - last entry to be moved
*
* OUTPUTS:
*           None
*
* RETURNS:
*           TPM_OK or TPM_FAIL
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t tpm_pnc_entry_insert(uint32_t entry,
                             uint32_t last_entry)
{
    int indx;
    int iterNum;
    uint32_t   cfg_pnc_parse_flag = 0;

	cfg_pnc_parse_flag = mv_eth_ctrl_pnc_get();

    if (cfg_pnc_parse_flag == 1) {
        TPM_OS_DEBUG(TPM_PNC_HM_MOD, " TPM is NOT configuring the PNC - MV driver is responsible \n");
        return (TPM_OK);
    }

    TPM_PNC_ERR_ON_OOR(entry >= TPM_PNC_SIZE);
    TPM_PNC_ERR_ON_OOR(last_entry+1 >= TPM_PNC_SIZE);
    TPM_PNC_ERR_ON_OOR(entry > last_entry);

    iterNum = last_entry - entry + 1;

    for (indx = 0; indx < iterNum; indx++)
    {
        tpm_pnc_entry_copy(last_entry-indx, last_entry-indx+1);
    }

    /* Invalidate inserted entry */
    tpm_pnc_entry_inv(entry);

    return TPM_OK;
}

/*******************************************************************************
* tpm_pnc_entry_delete()
*
* DESCRIPTION: This API deletes a TCAM entry in a range.
*              It closes the gap, and pushes up subsequent TCAM entries in the range.
*
* INPUTS:      entry        - entry to be deleted
*              last_entry   - last entry to be moved
*
* OUTPUTS:
*           None
*
* RETURNS:
*           TPM_OK or TPM_FAIL
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t tpm_pnc_entry_delete(uint32_t entry,
                             uint32_t last_entry)
{
    int indx;
    int iterNum;
    uint32_t  cfg_pnc_parse_flag = 0;

	cfg_pnc_parse_flag = mv_eth_ctrl_pnc_get();

    if (cfg_pnc_parse_flag == 1) {
        TPM_OS_DEBUG(TPM_PNC_HM_MOD, " TPM is NOT configuring the PNC - MV driver is responsible \n");
        return (TPM_OK);
    }
    TPM_PNC_ERR_ON_OOR(entry >= TPM_PNC_SIZE);
    TPM_PNC_ERR_ON_OOR(last_entry >= TPM_PNC_SIZE);
    TPM_PNC_ERR_ON_OOR(entry > last_entry);

    iterNum = last_entry - entry;

    /* Invalidate deleted entry */
    tpm_pnc_entry_inv(entry);

    for (indx = 0; indx < iterNum; indx++)
    {
        tpm_pnc_entry_copy(entry+indx+1, entry+indx);
    }

    /* Invalidate last moved entry */
    tpm_pnc_entry_inv(last_entry);

    return TPM_OK;
}

/*******************************************************************************
* tpm_pnc_range_inv()
*
* DESCRIPTION: This API resets a range
*
* INPUTS:      entry        - start entry to be deleted
*              last_entry   - last entry to be deleted
*
* OUTPUTS:
*           None
*
* RETURNS:
*           TPM_OK or TPM_FAIL
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t tpm_pnc_range_inv(uint32_t entry,
                          uint32_t last_entry)
{
    int indx;
    uint32_t          cfg_pnc_parse_flag = 0;

	cfg_pnc_parse_flag = mv_eth_ctrl_pnc_get();

    if (cfg_pnc_parse_flag == 1) {
        TPM_OS_DEBUG(TPM_PNC_HM_MOD, " TPM is NOT configuring the PNC - MV driver is responsible \n");
        return (TPM_OK);
    }

    TPM_OS_DEBUG( TPM_PNC_HM_MOD, "entry(%d) last_entry(%d)\n",entry, last_entry);
    TPM_PNC_ERR_ON_OOR(entry >= TPM_PNC_SIZE);
    TPM_PNC_ERR_ON_OOR(last_entry >= TPM_PNC_SIZE);
    TPM_PNC_ERR_ON_OOR(entry > last_entry);

    for (indx = entry; indx <= last_entry; indx++)
    {
        /* Invalidate entry */
        tpm_pnc_entry_inv(indx);
    }

    return TPM_OK;
}

/*******************************************************************************
* tpm_pnc_get_next_valid()
*
* DESCRIPTION: The API returns the next valid PnC entry
*
* INPUTS:   cur_entry   - start valid entry index
*
* OUTPUTS:  next_entry  - next valid entry index
*           pnc_entry   - next valid entry
*
* RETURNS:
*           TPM_OK, TPM_FAIL or TPM_NOT_FOUND
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t tpm_pnc_get_next_valid(uint32_t       cur_entry,
                               int32_t       *next_entry,
                               tpm_pnc_all_t *pnc_entry)
{
    int               indx;
    struct tcam_entry tcamEntry;
    int32_t           rc = TPM_NOT_FOUND;

    TPM_PNC_ERR_ON_OOR(cur_entry >= TPM_PNC_SIZE);
    TPM_PNC_ERR_ON_OOM(next_entry == NULL);
    TPM_PNC_ERR_ON_OOM(pnc_entry == NULL);

    *next_entry = -1;

    for (indx = (cur_entry + 1); indx < TPM_PNC_SIZE; indx++)
    {
        /* Clear SW TCAM entry */
        tcam_sw_clear(&tcamEntry);

        /* Read entry from HW */
        tpm_pnc_hw_entry_read(&tcamEntry, indx);

        if (tcamEntry.ctrl.flags & TCAM_F_INV)
            continue;

        rc = TPM_OK;
        *next_entry = indx;
        tpm_pnc_fill_in_entry(&tcamEntry, pnc_entry);
        break;
    }

    return rc;
}


