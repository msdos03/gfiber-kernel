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

#ifndef __TPM_PNCM_H__
#define __TPM_PNCM_H__


#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************/
/*               TEMP. DEFINITIONS                           */
/*************************************************************/

/*************************************************************/
/*               DEFINITIONS                                 */
/*************************************************************/
#define MAX_PORT_NUMBER         0x1F
#define MAX_ADD_INFO_BITS       7  /* Add info field size = 7 bits */

/* Result info field size = 24 bits */
#define MAX_RES_INFO_BITS_16    16
#define MAX_RES_INFO_BITS_8     8
#define RES_INFO_BITS_16_MASK   0x0000FFFF
#define RES_INFO_BITS_8_MASK    0x00FF0000


/* Flow ID */
#define FLOW_ID_LSW_MASK        0x00000001
#define FLOW_ID_MSW_MASK        0x00000002
#define FLOW_ID_MSW_SHIFT       16

/*************************************************************/
/*               ENUMERATIONS                                */
/*************************************************************/

/*************************************************************/
/*               STRUCTURES                                  */
/*************************************************************/

/*************************************************************/
/*               PROTOTYPES for internal using               */
/*************************************************************/
void tpm_pnc_fill_in_entry(struct tcam_entry *tcamEntry, tpm_pnc_all_t *pnc_entry);
void tpm_pnc_print_sw_entry(tpm_pnc_all_t *pnc_entry);

/*************************************************************/
/*               PROTOTYPES                                  */
/*************************************************************/
int32_t tpm_pnc_set            (uint32_t entry, uint32_t sram_update, tpm_pnc_all_t *pnc_entry);
int32_t tpm_pnc_get_next_valid (uint32_t cur_entry, int32_t *next_entry, tpm_pnc_all_t  *pnc_entry);
int32_t tpm_pnc_entry_insert   (uint32_t entry, uint32_t last_entry);
int32_t tpm_pnc_entry_delete   (uint32_t entry, uint32_t last_entry);
int32_t tpm_pnc_entry_inv      (uint32_t entry);
int32_t tpm_pnc_range_inv      (uint32_t entry, uint32_t last_entry);

#ifdef __cplusplus
}
#endif

#endif /* __TPM_PNCM_H__*/

