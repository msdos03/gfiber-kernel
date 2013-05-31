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

#ifndef __TPM_MODZ2_H__
#define __TPM_MODZ2_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "tpm_common.h"
#include "tpm_db.h"
#include "tpm_self_check.h"

/*************************************************************/
/*               TEMP. DEFINITIONS                           */
/*************************************************************/


/*************************************************************/
/*               DEFINITIONS                                 */
/*************************************************************/
#define TPM_MOD2_OPCODE_NOOP                0x0
#define TPM_MOD2_OPCODE_ADD                 0x1
#define TPM_MOD2_OPCODE_CONF_VLAN           0x2
#define TPM_MOD2_OPCODE_ADD_VLAN            0x3
#define TPM_MOD2_OPCODE_CONF_DSA1           0x4
#define TPM_MOD2_OPCODE_CONF_DSA2           0x5
#define TPM_MOD2_OPCODE_ADD_DSA             0x6
#define TPM_MOD2_OPCODE_DEL                 0x7
#define TPM_MOD2_OPCODE_REP2                0x8
#define TPM_MOD2_OPCODE_REP_LSB             0x9
#define TPM_MOD2_OPCODE_REP_MSB             0xA
#define TPM_MOD2_OPCODE_REP_VLAN            0xB
#define TPM_MOD2_OPCODE_DEC_TTL_LSB         0xC
#define TPM_MOD2_OPCODE_DEC_TTL_MSB         0xD
#define TPM_MOD2_OPCODE_ADD_CALC_LEN        0xE
#define TPM_MOD2_OPCODE_REP_LEN             0xF
#define TPM_MOD2_OPCODE_REP_IP_CHKSUM       0x10
#define TPM_MOD2_OPCODE_ADD_IP_CHKSUM       0x18
#define TPM_MOD2_OPCODE_REP_L4_CHKSUM       0x11
#define TPM_MOD2_OPCODE_SKIP                0x12
#define TPM_MOD2_OPCODE_JUMP                0x13
#define TPM_MOD2_OPCODE_JUMP_SKIP2          0x14
#define TPM_MOD2_OPCODE_JUMP_SUBR           0x15
#define TPM_MOD2_OPCODE_PPPOE               0x16
#define TPM_MOD2_OPCODE_STORE               0x17


#define TPM_MOD2_DEFAULT_ETY                (0x8100)
#define TPM_MOD2_DEFAULT_PPPOE_ETY          (0x8864)
#define TPM_MOD2_DEFAULT_PPPOE_DATA         (0x1100)
#define TPM_MOD2_DEFAULT_PPPOE_LEN_MH_EN    (0x7000)
#define TPM_MOD2_DEFAULT_PPPOE_LEN_MH_DIS   (0x7202)
#define TPM_MOD2_DEFAULT_PPPOE_PROTO        (0x0021)

/* Self-defined Ethernet type for CPU loopback frames*/
#define TPM_MOD2_CPU_LOOPBACK_ETY           (0xAABB)
/* Self-defined Marvell header for MAC learning frames on Media Convert*/
#define TPM_MOD2_MAC_LEARN_MH               (0xAACC)

/* Max number of modification entries per command */
#define TPM_MOD2_MAX_CMD_ENTRIES            (5)

#define TPM_MOD2_MH_CMD_ENTRIES             (1)
#define TPM_MOD2_MAC_CMD_ENTRIES            (3)
#define TPM_MOD2_IP_CMD_ENTRIES             (2)
#define TPM_MOD2_PPPOE_DEL_CMD_ENTRIES      (2)
#define TPM_MOD2_PPPOE_ADD_CMD_ENTRIES      (1)
#define TPM_MOD2_IPV6_PPPOE_ADD_CMD_ENTRIES (5)

#define TPM_MOD2_MAX_VLAN_ENTRIES           (2)
#define TPM_MOD2_VLAN_ADD_ENTRIES           (2)
#define TPM_MOD2_VLAN_DEL_ENTRIES           (1)
#define TPM_MOD2_VLAN_MOD_ENTRIES           (2)
#define TPM_MOD2_VLAN_SKIP_ENTRIES          (1)

#define TPM_MOD2_NULL_PATTERN_IDX           (0xFF)

#define TPM_MOD2_NEW_TPID                   (0xFFFF)
#define TPM_MOD2_NEW_VID                    (0xFFFF)
#define TPM_MOD2_NEW_CFI                    (0xFF)
#define TPM_MOD2_NEW_PBIT                   (0xFF)

#define TPM_MOD2_ORIGINAL_VID               (0x0)
#define TPM_MOD2_ORIGINAL_CFI               (0x0)
#define TPM_MOD2_ORIGINAL_PBIT              (0x0)

#define TPM_MOD2_ENTRY_VID_OFFS             (0)
#define TPM_MOD2_ENTRY_VID_BITS             (12)
#define TPM_MOD2_ENTRY_VID_MASK             (((1 << TPM_MOD2_ENTRY_VID_BITS) - 1) << TPM_MOD2_ENTRY_VID_OFFS)

#define TPM_MOD2_ENTRY_CFI_OFFS             (12)
#define TPM_MOD2_ENTRY_CFI_BITS             (1)
#define TPM_MOD2_ENTRY_CFI_MASK             (1 << TPM_MOD2_ENTRY_CFI_OFFS)

#define TPM_MOD2_ENTRY_PBIT_OFFS            (13)
#define TPM_MOD2_ENTRY_PBIT_BITS            (3)
#define TPM_MOD2_ENTRY_PBIT_MASK            (((1 << TPM_MOD2_ENTRY_PBIT_BITS) - 1) << TPM_MOD2_ENTRY_PBIT_OFFS)

#define TPM_MOD2_ENTRY_MOD_DATA_MASK        (0x0000FFFF)

#define TPM_MOD2_TP_FROM_REG_0              (0x0000)
#define TPM_MOD2_TP_FROM_REG_1              (0x0001)
#define TPM_MOD2_TP_FROM_REG_2              (0x0002)
#define TPM_MOD2_TP_FROM_REG_3              (0x0003)
#define TPM_MOD2_TP_FROM_VLAN_1             (0x0004)
#define TPM_MOD2_TP_FROM_VLAN_2             (0x0005)

#define TPM_MOD2_VID_FROM_VLAN_1            (0x0000)
#define TPM_MOD2_VID_FROM_VLAN_2            (0x0010)
#define TPM_MOD2_VID_NEW                    (0x0020)

#define TPM_MOD2_CFI_FROM_VLAN_1            (0x0000)
#define TPM_MOD2_CFI_FROM_VLAN_2            (0x0040)
#define TPM_MOD2_CFI_NEW                    (0x0080)

#define TPM_MOD2_PBIT_FROM_VLAN_1           (0x0000)
#define TPM_MOD2_PBIT_FROM_VLAN_2           (0x0100)
#define TPM_MOD2_PBIT_NEW                   (0x0200)


#define TPM_MOD2_INVALID_CHAIN_ID           (0xFFFF)

#define TPM_MOD2_IPV4_MOD_MASK              (TPM_DSCP_SET | TPM_TTL_DEC | TPM_IPV4_UPDATE | TPM_IPV4_SRC_SET | TPM_IPV4_DST_SET)
#define TPM_MOD2_IPV6_MOD_MASK              (TPM_IPV6_UPDATE | TPM_HOPLIM_DEC | TPM_IPV6_SRC_SET | TPM_IPV6_DST_SET)
#define TPM_MOD2_L4_MOD_MASK                (TPM_L4_SRC_SET | TPM_L4_DST_SET)

#define TPM_MOD2_DONT_CARE_INT_BM           (0xFFFFFFFF)

/*************************************************************/
/*               ENUMERATIONS                                */
/*************************************************************/


/*************************************************************/
/*               STRUCTURES                                  */
/*************************************************************/

typedef struct
{
    tpm_area_type_t     area_type;
    tpm_chain_type_t    chain_type;
    uint16_t            index;
    uint8_t             is_valid;
    uint8_t             line_num;
    tpm_mod2_entry_t    line[TPM_MOD2_MAX_PATTERN_ENTRIES];
} tpm_pattern_entry_t;

typedef struct
{
    tpm_chain_type_t    main_chain_type;
    uint16_t            main_chain_id;
    tpm_pattern_entry_t pattern_set[TPM_MOD2_MAX_PATTERN_SETS];
} tpm_mod_pattern_data_t;

typedef int32_t  (*pattern_f)(tpm_gmacs_enum_t      gmac_port,
                              uint32_t                bm,
                              uint16_t                cmd_idx,
                              uint8_t                *skip,
                              tpm_pkt_mod_t          *mod_data,
                              tpm_mod_pattern_data_t *pattern_data,
                              tpm_self_check_enable_t is_tpm_check);

typedef struct
{
    uint8_t             skip_before;
    uint8_t             skip_no_parse;
    uint32_t            mask;
    pattern_f           process_pattern;
    tpm_mod2_entry_t    entry[TPM_MOD2_MAX_CMD_ENTRIES];

} tpm_mod_flag_bm_entry_t;

typedef struct
{
    tpm_mod2_entry_t    vlan1[TPM_MOD2_MAX_VLAN_ENTRIES];
    tpm_mod2_entry_t    vlan2[TPM_MOD2_MAX_VLAN_ENTRIES];
} tpm_mod_vlan_entry_t;

typedef struct
{
    tpm_chain_type_t chain_type;
    tpm_pkt_mod_bm_t mod_bm;
	tpm_pkt_mod_int_bm_t int_mod_bm;
} tpm_chain_map_t;

/*************************************************************/
/*               PROTOTYPES                                  */
/*************************************************************/
int32_t    tpm_mod2_entry_set   (tpm_mod_owner_t    owner,
                                 tpm_gmacs_enum_t   gmac_port,
                                 tpm_pkt_mod_bm_t   mod_bm,
                                 tpm_pkt_mod_int_bm_t int_mod_bm,
                                 tpm_pkt_mod_t      *mod_data,
                                 uint32_t           *mod_entry);

int32_t    tpm_mod2_entry_get   (tpm_gmacs_enum_t   gmac_port,
                                        uint32_t           mod_entry,
                                        uint16_t           *entries_num,
                                        tpm_mod_rule_t     *pnc_entries);

int32_t    tpm_mod2_entry_del   (tpm_mod_owner_t    owner,
                                 tpm_gmacs_enum_t   gmac_port,
                                 uint32_t           mod_entry);

int32_t    tpm_mod2_hw_inv_entry(tpm_gmacs_enum_t    gmac_port,
                                 uint16_t            mod_entry);

int32_t    tpm_mod2_mac_inv (tpm_gmacs_enum_t   gmac_port);

int32_t    tpm_mod2_registers_init (tpm_gmacs_enum_t   gmac_port, uint16_t txp);

int32_t    tpm_mod2_tpid_get(uint32_t tpid,
                             uint32_t *eth_type_sel);

int32_t    tpm_mod2_print_tmp_pattern (tpm_mod_pattern_data_t *pattern_data);

int32_t    tpm_mod2_print_rule (tpm_gmacs_enum_t   gmac_port,
                                uint32_t           mod_entry);

int32_t    tpm_mod2_print_jump_all (tpm_gmacs_enum_t   gmac_port);

int32_t    tpm_mod2_print_main_all (tpm_gmacs_enum_t   gmac_port);

int32_t    tpm_mod2_print_config (tpm_gmacs_enum_t   gmac_port);

int32_t    tpm_mod2_init(void);
int32_t tpm_mod2_split_mod_init(tpm_gmacs_enum_t port);
int32_t tpm_mod2_split_mod_create_l2_pmts(tpm_gmacs_enum_t port, tpm_pkt_mod_t *mod_data, bool init_flag);
int32_t tpm_mod2_split_mod_try_pmt_entry_del(tpm_api_sections_t api_section,
                                  tpm_gmacs_enum_t     gmac_port,
                                  uint32_t             mod_entry);

int32_t    tpm_mod2_fill_in_pattern   (tpm_gmacs_enum_t     gmac_port,
                                       tpm_pkt_mod_bm_t        mod_bm,
                                       tpm_pkt_mod_int_bm_t    int_mod_bm,
                                       tpm_pkt_mod_t          *mod_data,
                                       uint32_t               *mod_entry,
                                       tpm_mod_pattern_data_t *pattern_data,
                                       tpm_self_check_enable_t is_tpm_check);
tpm_chain_type_t tpm_mod2_parse_chain_type(tpm_pkt_mod_bm_t mod_bm, tpm_pkt_mod_int_bm_t int_mod_bm);

/*for TPM self check*/
int32_t tpm_mod2_main_chain_index_get(tpm_gmacs_enum_t gmac_port,
                                      uint16_t jump_idx,
                                      uint16_t *main_entry,
                                      tpm_self_check_enable_t from);
int32_t tpm_mod2_neta_pmt_get(tpm_gmacs_enum_t gmac_port, int idx, MV_NETA_PMT *pEntry);
int32_t tpm_mod2_db_jump_pattern_data_get(tpm_gmacs_enum_t gmac_port, uint16_t entry_id, tpm_mod2_entry_t *pattern);
int32_t tpm_mod2_db_chain_pattern_data_get(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type,
					   uint16_t chain_id,
					   uint16_t pattern_id,
					   tpm_mod2_entry_t *pattern);
#ifdef __cplusplus
}
#endif

#endif /* __TPM_MODZ2_H__ */
