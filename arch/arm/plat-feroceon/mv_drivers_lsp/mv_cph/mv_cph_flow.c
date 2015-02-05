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
********************************************************************************
* mv_cph_flow.c
*
* DESCRIPTION: Marvell CPH(CPH Packet Handler) flow module to handle the 
*              flow mapping, VLAN modification of data traffic
*
* DEPENDENCIES:
*               None
*
* CREATED BY:   VictorGu
*
* DATE CREATED: 12Dec2011
*
* FILE REVISION NUMBER:
*               Revision: 1.1
*
*
*******************************************************************************/
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <net/ip.h>
#include <net/ipv6.h>

#include "mv_cph_header.h"

/****************************************************************************** 
*                           Global Data Definitions                                                                                                                                  
******************************************************************************/
static CPH_FLOW_TABLE_T gs_flow_table;
static CPH_DSCP_PBITS_T gs_dscp_map_table;

static MV_ENUM_ENTRY_T g_enum_map_op_type[] =
{
    { CPH_VLAN_OP_ASIS,                              "ASIS"},
    { CPH_VLAN_OP_DISCARD,                           "DISCARD"},
    { CPH_VLAN_OP_ADD,                               "ADD"},
    { CPH_VLAN_OP_ADD_COPY_DSCP,                     "ADD_COPY_DSCP"},
    { CPH_VLAN_OP_ADD_COPY_OUTER_PBIT,               "ADD_COPY_OUTER_PBIT"},
    { CPH_VLAN_OP_ADD_COPY_INNER_PBIT,               "ADD_COPY_INNER_PBIT"},
    { CPH_VLAN_OP_ADD_2_TAGS,                        "ADD_2_TAGS"},
    { CPH_VLAN_OP_ADD_2_TAGS_COPY_DSCP,              "ADD_2_TAGS_COPY_DSCP"},
    { CPH_VLAN_OP_ADD_2_TAGS_COPY_PBIT,              "ADD_2_TAGS_COPY_PBIT"},
    { CPH_VLAN_OP_REM,                               "REM"},
    { CPH_VLAN_OP_REM_2_TAGS,                        "REM_2_TAGS"},
    { CPH_VLAN_OP_REPLACE,                           "REPLACE"},    
    { CPH_VLAN_OP_REPLACE_VID,                       "REPLACE_VID"},
    { CPH_VLAN_OP_REPLACE_PBIT,                      "REPLACE_PBIT"},
    { CPH_VLAN_OP_REPLACE_INNER_ADD_OUTER,           "REPLACE_INNER_ADD_OUTER"},
    { CPH_VLAN_OP_REPLACE_INNER_ADD_OUTER_COPY_PBIT, "REPLACE_INNER_ADD_OUTER_COPY_PBIT"},
    { CPH_VLAN_OP_REPLACE_INNER_REM_OUTER,           "REPLACE_INNER_REM_OUTER"},
    { CPH_VLAN_OP_REPLACE_2TAGS,                     "REPLACE_2TAGS"},    
    { CPH_VLAN_OP_REPLACE_2TAGS_VID,                 "REPLACE_2TAGS_VID"},
    { CPH_VLAN_OP_SWAP,                              "SWAP"}    
};

static MV_ENUM_ARRAY_T g_enum_array_op_type =
{
    sizeof(g_enum_map_op_type)/sizeof(g_enum_map_op_type[0]),
    g_enum_map_op_type
};

/****************************************************************************** 
*                           External Declarations                                                                                                                                 
******************************************************************************/



/****************************************************************************** 
*                           Function Definitions                                                                                                                                  
******************************************************************************/
/******************************************************************************
* cph_flow_db_get_rule()
* _____________________________________________________________________________
*
* DESCRIPTION: Get CPH flow mapping rule.
*
* INPUTS:
*       flow       - Flow parsing field values
*       for_packet - Whether get rule for packet or for new CPH rule
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
MV_STATUS cph_flow_db_get_rule (CPH_FLOW_ENTRY_T *flow, BOOL for_packet)
{
    UINT32            idx         = 0;
    UINT32            rule_idx    = 0;
    CPH_FLOW_ENTRY_T *p_flow_rule = NULL;
    BOOL              rc          = FALSE;
    unsigned long     flags;

    spin_lock_irqsave(&gs_flow_table.flow_lock, flags);
    /* Traverse CPH flow rule table */
    for (idx = 0, rule_idx = 0; (idx < CPH_FLOW_ENTRY_NUM) && (rule_idx < gs_flow_table.rule_num); idx++)
    {
        p_flow_rule = &gs_flow_table.flow_rule[idx];
        
        /* Compare packet or new rule rule data base rule */
        if (p_flow_rule->valid == TRUE)
        {
            rule_idx++;

            if (for_packet == TRUE)                
                rc = cph_flow_compare_packet_and_rule(flow, p_flow_rule);
            else
                rc = cph_flow_compare_rules(flow, p_flow_rule);

            if (rc == TRUE)
            {   
                flow->op_type = p_flow_rule->op_type;
                memcpy(&flow->mod_outer_tci, &p_flow_rule->mod_outer_tci, sizeof(CPH_FLOW_TCI_T));
                memcpy(&flow->mod_inner_tci, &p_flow_rule->mod_inner_tci, sizeof(CPH_FLOW_TCI_T));
                memcpy(&flow->pkt_frwd,      &p_flow_rule->pkt_frwd,      sizeof(CPH_FLOW_FRWD_T));

                /* Increase count */
                if (for_packet == TRUE)
                {
                    if (p_flow_rule->count == 0xFFFFFFFF)
                        p_flow_rule->count = 0;
                    else
                        p_flow_rule->count++;
                }

                spin_unlock_irqrestore(&gs_flow_table.flow_lock, flags);
                return MV_OK;
            }
        }
    }
    spin_unlock_irqrestore(&gs_flow_table.flow_lock, flags);

    return MV_FAIL;
}

/******************************************************************************
* cph_flow_get_rule_by_vid()
* _____________________________________________________________________________
*
* DESCRIPTION: Get CPH flow mapping rule by VID, only used to compare packet and db rule.
*
* INPUTS:
*       flow       - Flow parsing field values
*       for_packet - Whether get rule for packet or for new CPH rule
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
MV_STATUS cph_flow_get_rule_by_vid (CPH_FLOW_ENTRY_T *flow)
{
    UINT32            idx         = 0;
    UINT32            rule_idx    = 0;
    CPH_FLOW_ENTRY_T *p_flow_rule = NULL;
    BOOL              rc          = FALSE;
    unsigned long     flags;

    spin_lock_irqsave(&gs_flow_table.flow_lock, flags);
    /* Traverse CPH flow rule table */
    for (idx = 0, rule_idx = 0; (idx < CPH_FLOW_ENTRY_NUM) && (rule_idx < gs_flow_table.rule_num); idx++)
    {
        p_flow_rule = &gs_flow_table.flow_rule[idx];

        /* Compare packet or new rule rule data base rule */
        if (p_flow_rule->valid == TRUE)
        {
            rule_idx++;

            rc = cph_flow_compare_packet_and_rule_vid(flow, p_flow_rule);
            if (rc == TRUE)
            {
                flow->op_type = p_flow_rule->op_type;
                memcpy(&flow->mod_outer_tci, &p_flow_rule->mod_outer_tci, sizeof(CPH_FLOW_TCI_T));
                memcpy(&flow->mod_inner_tci, &p_flow_rule->mod_inner_tci, sizeof(CPH_FLOW_TCI_T));
                memcpy(&flow->pkt_frwd,      &p_flow_rule->pkt_frwd,      sizeof(CPH_FLOW_FRWD_T));

                /* Increase count */
                if (p_flow_rule->count == 0xFFFFFFFF)
                    p_flow_rule->count = 0;
                else
                    p_flow_rule->count++;

                spin_unlock_irqrestore(&gs_flow_table.flow_lock, flags);
                return MV_OK;
            }
        }
    }
    spin_unlock_irqrestore(&gs_flow_table.flow_lock, flags);

    return MV_FAIL;
}

/******************************************************************************
* cph_flow_db_add_rule()
* _____________________________________________________________________________
*
* DESCRIPTION: Add flow rule to database 
*
* INPUTS:
*       cph_flow - VLAN ID, 802.1p value, pkt_fwd information.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
INT32  cph_flow_db_add_rule(CPH_FLOW_ENTRY_T *cph_flow)
{
    UINT32            idx         = 0;
    CPH_FLOW_ENTRY_T *p_flow_rule = NULL;
    BOOL              rc          = MV_OK;
    unsigned long     flags;

    /* If the flow table is full */
    if (gs_flow_table.rule_num >= CPH_FLOW_ENTRY_NUM)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s(), flow rule table is full<%d> \n", __FUNCTION__, gs_flow_table.rule_num);
        return MV_FULL;
    }

    /* Check if there is already conflicted rules */
    if (cph_flow_db_get_rule(cph_flow, FALSE) == MV_OK)
    {
        MV_CPH_PRINT(CPH_DEBUG_LEVEL, "%s(), already has conflict flow rule \n", __FUNCTION__);
        return MV_OK;
    }

    spin_lock_irqsave(&gs_flow_table.flow_lock, flags);    
    /* Traverse CPH flow rule tale */
    for (idx = 0; idx < CPH_FLOW_ENTRY_NUM; idx++)
    {
        p_flow_rule = &gs_flow_table.flow_rule[idx];
        
        /* Compare parse_bm and parse_key */
        if (p_flow_rule->valid == FALSE)
           break;
    }

    if (idx == CPH_FLOW_ENTRY_NUM)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s(), flow rule table is full<%d> \n", __FUNCTION__, gs_flow_table.rule_num);
        spin_unlock_irqrestore(&gs_flow_table.flow_lock, flags);        
        return MV_FULL;    
    }

    /* Save to db */
    memcpy(p_flow_rule, cph_flow, sizeof(CPH_FLOW_ENTRY_T));
    p_flow_rule->valid = TRUE;
    p_flow_rule->count = 0;
    gs_flow_table.rule_num ++;
    spin_unlock_irqrestore(&gs_flow_table.flow_lock, flags);

    return rc;   
}

/******************************************************************************
* cph_flow_db_del_rule()
* _____________________________________________________________________________
*
* DESCRIPTION: Delete CPH flow mapping rule.
*
* INPUTS:
*       flow - Flow parsing field values
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
MV_STATUS cph_flow_db_del_rule (CPH_FLOW_ENTRY_T *flow)
{
    UINT32            idx         = 0;
    UINT32            rule_idx    = 0;
    CPH_FLOW_ENTRY_T *p_flow_rule = NULL;
    BOOL              rc          = MV_OK;
    unsigned long     flags;
 
    spin_lock_irqsave(&gs_flow_table.flow_lock, flags);    
    /* Traverse CPH flow rule tale */
    for (idx = 0, rule_idx = 0; (idx < CPH_FLOW_ENTRY_NUM) && (rule_idx < gs_flow_table.rule_num); idx++)
    {
        p_flow_rule = &gs_flow_table.flow_rule[idx];
        
        /* Compare parse_bm and parse_key */
        if (p_flow_rule->valid == TRUE)
        {
            rule_idx++;

            rc = cph_flow_compare_rules(flow, p_flow_rule);
            if (rc == TRUE)
            {   
                memset(p_flow_rule, 0, sizeof(CPH_FLOW_ENTRY_T));
                p_flow_rule->valid = FALSE;
                gs_flow_table.rule_num--;

                spin_unlock_irqrestore(&gs_flow_table.flow_lock, flags);                
                return MV_OK;
            }
        }
    }
    spin_unlock_irqrestore(&gs_flow_table.flow_lock, flags);

    return rc;
}

/******************************************************************************
* cph_flow_db_clear_rule()
* _____________________________________________________________________________
*
* DESCRIPTION: Clear CPH flow mapping rules.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
MV_STATUS cph_flow_db_clear_rule (VOID)
{
    UINT32 idx = 0;
    unsigned long flags;

    spin_lock_irqsave(&gs_flow_table.flow_lock, flags);    
    /* Initializes flow rule */
    memset((UINT8 *)&gs_flow_table, 0, sizeof(CPH_FLOW_TABLE_T));
    for (idx = 0; idx < CPH_FLOW_ENTRY_NUM; idx++)
        gs_flow_table.flow_rule[idx].valid = FALSE;
    
    spin_unlock_irqrestore(&gs_flow_table.flow_lock, flags);

    return MV_OK;
}

/******************************************************************************
* cph_flow_db_clear_rule_by_mh()
* _____________________________________________________________________________
*
* DESCRIPTION: Clear CPH flow mapping rules by MH.
*
* INPUTS:
*       mh  -  Marvell header.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
MV_STATUS cph_flow_db_clear_rule_by_mh (UINT16 mh)
{
    CPH_FLOW_ENTRY_T   *p_flow_rule = NULL;
    UINT32              idx = 0;
    unsigned long       flags;
    
    spin_lock_irqsave(&gs_flow_table.flow_lock, flags);    
    /* Initializes flow rule */
    for (idx = 0; idx < CPH_FLOW_ENTRY_NUM; idx++)
    {
        p_flow_rule = &gs_flow_table.flow_rule[idx];

        if ((p_flow_rule->valid == TRUE) &&
            (p_flow_rule->mh == mh))
        {
            memset(p_flow_rule, 0, sizeof(CPH_FLOW_ENTRY_T));
            p_flow_rule->valid = FALSE;
            gs_flow_table.rule_num--;
        }
    }
    spin_unlock_irqrestore(&gs_flow_table.flow_lock, flags);

    return MV_OK;
}

/******************************************************************************
* cph_flow_db_init()
* _____________________________________________________________________________
*
* DESCRIPTION: Initialize CPH flow mapping database.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
MV_STATUS cph_flow_db_init (VOID)
{
    UINT32 idx = 0;
    
    /* Initializes flow rule */
    memset((UINT8 *)&gs_flow_table, 0, sizeof(CPH_FLOW_TABLE_T));
    for (idx = 0; idx < CPH_FLOW_ENTRY_NUM; idx++)
        gs_flow_table.flow_rule[idx].valid = FALSE;

    spin_lock_init(&gs_flow_table.flow_lock);

    /* Initializes DSCP to P-bits mapping table */
    memset((UINT8 *)&gs_dscp_map_table, 0, sizeof(CPH_DSCP_PBITS_T));
    gs_dscp_map_table.in_use = 0;

    return MV_OK;
}

/******************************************************************************
* cph_flow_verify_rule()
* _____________________________________________________________________________
*
* DESCRIPTION: Verify flow mapping rule
*
* INPUTS:
*       tci         - TPID, VLAN ID, P-bits information.
*       parse_field - Whether the TCI is from parsing field.
*       tci_field   - the TCI field need to be checked.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns TRUE. 
*       On error returns FALSE.
*******************************************************************************/
BOOL  cph_flow_verify_tci(CPH_FLOW_TCI_T *tci, BOOL parse_field, CPH_TCI_FIELD_E tci_field)
{
    UINT16 max_vid   = 0;
    UINT16 max_pbits = 0;
    
    if (TRUE == parse_field) {
        max_vid   = MV_CPH_VID_NOT_CARE_VALUE;
        max_pbits = MV_CPH_PBITS_NOT_CARE_VALUE;
    }
    else {
        max_vid   = MV_VLAN_ID_MAX;
        max_pbits = MV_PBITS_MAX;        
    }
    
    /* Check TPID */
    if ((tci->tpid != MV_TPID_8100) &&
        (tci->tpid != MV_TPID_9100) &&
        (tci->tpid != MV_TPID_88A8) &&
        ((tci->tpid != MV_CPH_TPID_NOT_CARE_VALUE) && (parse_field == TRUE))) {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "tpid[0x%x] is invalid \n", tci->tpid);
        return FALSE;
    }

    /* Check VID */
    if ((tci_field == CPH_TCI_FIELD_VID) ||
        (tci_field == CPH_TCI_FIELD_VID_PBIT) ||
        (tci_field == CPH_TCI_FIELD_ALL)) {
    if (tci->vid > max_vid) {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "vid[%d] exceeds maximum value[%d] \n", tci->vid, max_vid);
        return FALSE;
    }
    }
    
    /* Check P-bits */
    if ((tci_field == CPH_TCI_FIELD_PBIT) ||
        (tci_field == CPH_TCI_FIELD_VID_PBIT) ||
        (tci_field == CPH_TCI_FIELD_ALL)) {
    if (tci->pbits > max_pbits) {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "pbits[%d] exceeds maximum value[%d] \n", tci->pbits, max_pbits);
        return FALSE;
    }
    }
    
    return TRUE;
}

/******************************************************************************
* cph_flow_verify_rule()
* _____________________________________________________________________________
*
* DESCRIPTION: Verify flow mapping rule
*
* INPUTS:
*       cph_flow - VLAN ID, 802.1p value, pkt_fwd information.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns TRUE. 
*       On error returns FALSE.
*******************************************************************************/
BOOL  cph_flow_verify_rule(CPH_FLOW_ENTRY_T *cph_flow, BOOL full)
{
    CPH_DIR_E          dir         = CPH_DIR_US;
    CPH_FLOW_PARSE_E   parse_bm    = 0;
    CPH_VLAN_OP_TYPE_E op_type     = 0;    
    CPH_FLOW_FRWD_T   *p_pkt_fwd   = NULL;
    BOOL               rc          = TRUE;

    /* Get input information: VID, P-bits... */
    if (cph_flow == NULL) {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "cph_flow is NULL \n");
        return FALSE;
    } 

    dir       =  cph_flow->dir;    
    parse_bm  =  cph_flow->parse_bm;
    op_type   =  cph_flow->op_type;    
    p_pkt_fwd = &cph_flow->pkt_frwd;

    /* Check dir */
    if (dir >= CPH_DIR_NOT_CARE) {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "dir[%d] exceeds maximum value[%d] \n", dir, CPH_DIR_NOT_CARE);
        return FALSE;
    }

    /* Check parse_bm */
    if ((parse_bm & CPH_FLOW_PARSE_EXT_VLAN) && 
        (parse_bm & CPH_FLOW_PARSE_TWO_VLAN)) {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "Cann't set parse_bm CPH_FLOW_PARSE_EXT_VLAN and CPH_FLOW_PARSE_EXT_VLAN at the same time \n");
        return FALSE;
    }      

    /* Check op_type */
    if (op_type > CPH_VLAN_OP_SWAP) {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "op_type[%d] exceeds maximum value[%d] \n", op_type, CPH_VLAN_OP_SWAP);
        return FALSE;
    }
    
    /* Check TCI */
    if ((parse_bm & CPH_FLOW_PARSE_EXT_VLAN) ||
        (parse_bm & CPH_FLOW_PARSE_TWO_VLAN)) {
        rc = cph_flow_verify_tci(&cph_flow->parse_outer_tci, TRUE, CPH_TCI_FIELD_VID_PBIT);
    if (rc == FALSE) {
        return FALSE;
    }
    }

    if (parse_bm & CPH_FLOW_PARSE_TWO_VLAN) {
        rc = cph_flow_verify_tci(&cph_flow->parse_inner_tci, TRUE, CPH_TCI_FIELD_VID_PBIT);
        if (rc == FALSE) {
            return FALSE;
        }
    }

    switch (cph_flow->op_type) {
        case CPH_VLAN_OP_ASIS:
        case CPH_VLAN_OP_DISCARD:
        case CPH_VLAN_OP_REM:
        case CPH_VLAN_OP_REM_2_TAGS:
        case CPH_VLAN_OP_SWAP:
            break;
        case CPH_VLAN_OP_ADD:
        case CPH_VLAN_OP_REPLACE:
        case CPH_VLAN_OP_REPLACE_INNER_REM_OUTER:
            rc = cph_flow_verify_tci(&cph_flow->mod_outer_tci, FALSE, CPH_TCI_FIELD_VID_PBIT);
            if (rc == FALSE) {
                return FALSE;
            }
            break;
        case CPH_VLAN_OP_ADD_COPY_DSCP:
        case CPH_VLAN_OP_ADD_COPY_OUTER_PBIT:
        case CPH_VLAN_OP_ADD_COPY_INNER_PBIT:
        case CPH_VLAN_OP_REPLACE_VID:
            rc = cph_flow_verify_tci(&cph_flow->mod_outer_tci, FALSE, CPH_TCI_FIELD_VID);
    if (rc == FALSE) {
        return FALSE;
    }
            break;
        case CPH_VLAN_OP_ADD_2_TAGS:
        case CPH_VLAN_OP_REPLACE_2TAGS:
        case CPH_VLAN_OP_REPLACE_INNER_ADD_OUTER:
            rc = cph_flow_verify_tci(&cph_flow->mod_outer_tci, FALSE, CPH_TCI_FIELD_VID_PBIT);
            if (rc == FALSE) {
                return FALSE;
            }
            rc = cph_flow_verify_tci(&cph_flow->mod_inner_tci, FALSE, CPH_TCI_FIELD_VID_PBIT);
            if (rc == FALSE) {
                return FALSE;
            }
            break;
        case CPH_VLAN_OP_ADD_2_TAGS_COPY_DSCP:
        case CPH_VLAN_OP_ADD_2_TAGS_COPY_PBIT:
        case CPH_VLAN_OP_REPLACE_2TAGS_VID:
        case CPH_VLAN_OP_REPLACE_INNER_ADD_OUTER_COPY_PBIT:
            rc = cph_flow_verify_tci(&cph_flow->mod_outer_tci, FALSE, CPH_TCI_FIELD_VID);
            if (rc == FALSE) {
                return FALSE;
            }
            rc = cph_flow_verify_tci(&cph_flow->mod_inner_tci, FALSE, CPH_TCI_FIELD_VID);
    if (rc == FALSE) {
        return FALSE;
    }
            break;
        case CPH_VLAN_OP_REPLACE_PBIT:
            rc = cph_flow_verify_tci(&cph_flow->mod_outer_tci, FALSE, CPH_TCI_FIELD_PBIT);
    if (rc == FALSE) {
        return FALSE;
    }
            break;
        default:
            break;
    }
    
    /* Check target port/queue/GEM port */ 
    if (p_pkt_fwd->trg_port > MV_TCONT_LLID_MAX) {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "trg_port[%d] exceeds maximum value[%d] \n", p_pkt_fwd->trg_port, MV_TCONT_LLID_MAX);
        return FALSE;
    }

    if (p_pkt_fwd->trg_queue > MV_QUEUE_MAX) {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "SWF trg_queue[%d] exceeds maximum value[%d] \n", p_pkt_fwd->trg_queue, MV_QUEUE_MAX);
        return FALSE;
    }   

    if (p_pkt_fwd->trg_hwf_queue > MV_QUEUE_MAX) {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "HWF trg_queue[%d] exceeds maximum value[%d] \n", p_pkt_fwd->trg_hwf_queue, MV_QUEUE_MAX);
        return FALSE;
    }       

    if (p_pkt_fwd->gem_port > MV_GEM_PORT_MAX) {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "gem_port[%d] exceeds maximum value[%d] \n", p_pkt_fwd->gem_port, MV_GEM_PORT_MAX);
        return FALSE;
    }

    return TRUE;
}

/******************************************************************************
* cph_flow_display_tci()
* _____________________________________________________________________________
*
* DESCRIPTION: Display TCI value
*
* INPUTS:
*       tci         - TCI field
*       trace_level - Trace and debug level
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
VOID  cph_flow_display_tci(CPH_FLOW_TCI_T *tci, UINT32 trace_level)
{
    MV_CPH_PRINT(trace_level, 
                 "TPID[0x%x], vid[%d], p-bits[%d]\n",
                 ((tci != NULL)? tci->tpid:0),
                 ((tci != NULL)? tci->vid:0), 
                 ((tci != NULL)? tci->pbits:0));
    return;
}

/******************************************************************************
* cph_flow_add_rule()
* _____________________________________________________________________________
*
* DESCRIPTION: Sets flow mapping rule
*
* INPUTS:
*       cph_flow - VLAN ID, 802.1p value, pkt_fwd information.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
INT32  cph_flow_add_rule(CPH_FLOW_ENTRY_T *cph_flow)
{
    BOOL rc = TRUE;

    /* Display input CPH flow */
    MV_CPH_PRINT(CPH_DEBUG_LEVEL, 
                 "---->\n default rule[%s], dir[%d], parse bm[%x], mh[%d], eth type[0x%x], op type[%d], trg port[%d], SWF queue[%d], HWF queue[%d], GEM port[%d]\n",
                 (cph_flow->is_default == TRUE)? "Yes":"No",
                 cph_flow->dir, cph_flow->parse_bm, cph_flow->mh, cph_flow->eth_type, cph_flow->op_type,
                 cph_flow->pkt_frwd.trg_port, cph_flow->pkt_frwd.trg_queue, cph_flow->pkt_frwd.trg_hwf_queue, cph_flow->pkt_frwd.gem_port);
    cph_flow_display_tci(&cph_flow->parse_outer_tci, CPH_DEBUG_LEVEL);
    cph_flow_display_tci(&cph_flow->parse_inner_tci, CPH_DEBUG_LEVEL);
    cph_flow_display_tci(&cph_flow->mod_outer_tci, CPH_DEBUG_LEVEL);
    cph_flow_display_tci(&cph_flow->mod_inner_tci, CPH_DEBUG_LEVEL);
            
    /* Verify CPH flow rule */
    rc = cph_flow_verify_rule(cph_flow, TRUE);
    if (rc == FALSE) {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s(), verify rule failed \n", __FUNCTION__);
        return MV_FAIL;
    }

    /* Add flow rule to data base */
    if (cph_flow_db_add_rule(cph_flow) != MV_OK) {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s(), fail to call cph_flow_db_add_rule\n", __FUNCTION__);
        return MV_FAIL;
    }

    MV_CPH_PRINT(CPH_DEBUG_LEVEL, "<----\n");

    return MV_OK;
}

/******************************************************************************
* cph_flow_del_rule()
* _____________________________________________________________________________
*
* DESCRIPTION: Deletes flow mapping rule
*
* INPUTS:
*       cph_flow - VLAN ID, 802.1p value, pkt_fwd information.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
INT32  cph_flow_del_rule(CPH_FLOW_ENTRY_T *cph_flow)
{
    /* Display input CPH flow */
    MV_CPH_PRINT(CPH_DEBUG_LEVEL, 
                 "---->\n default rule[%s], dir[%d], parse bm[%x], mh[%d], eth type[0x%x], op type[%d]\n",
                 (cph_flow->is_default == TRUE)? "Yes":"No",
                 cph_flow->dir, cph_flow->parse_bm, cph_flow->mh, cph_flow->eth_type, cph_flow->op_type);
    cph_flow_display_tci(&cph_flow->parse_outer_tci, CPH_DEBUG_LEVEL);
    cph_flow_display_tci(&cph_flow->parse_inner_tci, CPH_DEBUG_LEVEL);

    /* Delete flow rule from data base */
    if (cph_flow_db_del_rule(cph_flow) != MV_OK) {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s(), fail to call cph_flow_db_del_rule\n", __FUNCTION__);
        return MV_FAIL;
    }

    MV_CPH_PRINT(CPH_DEBUG_LEVEL, "<----\n");

    return MV_OK; 
}

/******************************************************************************
* cph_flow_get_tag_rule()
* _____________________________________________________________________________
*
* DESCRIPTION: Gets flow mapping rule for tagged frames.
*
* INPUTS:
*       cph_flow - Input vid, pbits, dir
*
* OUTPUTS:
*       cph_flow - output packet forwarding information, including GEM port, 
*                  T-CONT, queue and packet modification for VID, P-bits.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
INT32  cph_flow_get_rule(CPH_FLOW_ENTRY_T *cph_flow)
{
    /* Display input CPH flow */
    MV_CPH_PRINT(CPH_DEBUG_LEVEL,
                 "---->\nPacket flow rule: default rule[%s], dir[%d], parse bm[%x], mh[%d], eth type[0x%x]\n",
                 (cph_flow->is_default == TRUE)? "Yes":"No",
                 cph_flow->dir, cph_flow->parse_bm, cph_flow->mh, cph_flow->eth_type);
    cph_flow_display_tci(&cph_flow->parse_outer_tci, CPH_DEBUG_LEVEL);
    cph_flow_display_tci(&cph_flow->parse_inner_tci, CPH_DEBUG_LEVEL);

    /* Get flow rule from data base */
    if (cph_flow_db_get_rule(cph_flow, TRUE) != MV_OK) {
        MV_CPH_PRINT(CPH_DEBUG_LEVEL, "%s(), fail to call cph_flow_db_get_rule\n", __FUNCTION__);
        return MV_FAIL;
    }

    MV_CPH_PRINT(CPH_DEBUG_LEVEL, "<----\n");

    return MV_OK;
}

/******************************************************************************
* cph_flow_clear_rule()
* _____________________________________________________________________________
*
* DESCRIPTION: Clears all flow mapping rules
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
INT32 cph_flow_clear_rule(VOID)
{
    /* Display input CPH flow */
    MV_CPH_PRINT(CPH_DEBUG_LEVEL, 
                 "---->\n");

    /* Clear flow rule from data base */
    if (cph_flow_db_clear_rule() != MV_OK) {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s(), fail to call cph_flow_db_clear_rule\n", __FUNCTION__);
        return MV_FAIL;
    }

    MV_CPH_PRINT(CPH_DEBUG_LEVEL, "<----\n"); 
    return MV_OK;
}

/******************************************************************************
* cph_flow_clear_rule_by_mh()
* _____________________________________________________________________________
*
* DESCRIPTION: Clears flow mapping rules by MH
*
* INPUTS:
*       mh   -  Marvell header.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
INT32 cph_flow_clear_rule_by_mh(UINT16 mh)
{
    /* Display input CPH flow */
    MV_CPH_PRINT(CPH_DEBUG_LEVEL, 
                 "---->  mh(%d)\n", mh);

    /* Clear flow rule from data base */
    if (cph_flow_db_clear_rule_by_mh(mh) != MV_OK) {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s(), fail to call cph_flow_db_clear_rule_by_mh\n", __FUNCTION__);
        return MV_FAIL;
    }

    MV_CPH_PRINT(CPH_DEBUG_LEVEL, "<----\n"); 
    return MV_OK;
}

/******************************************************************************
* cph_flow_set_dscp_map()
* _____________________________________________________________________________
*
* DESCRIPTION: Sets DSCP to P-bits mapping rules
*
* INPUTS:
*       dscp_map  - DSCP to P-bits mapping rules.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
INT32  cph_flow_set_dscp_map(CPH_DSCP_PBITS_T *dscp_map)
{
    MV_CPH_PRINT(CPH_DEBUG_LEVEL, 
                 "----> in_use[%d]\n",
                 ((dscp_map!= NULL)? dscp_map->in_use:0));

    if (dscp_map == NULL) {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "Input dscp_map is NULL\n");
        return MV_FAIL;
    }

    /* Case 1: to enable DSCP to P-bits mapping */
    if (dscp_map->in_use != 0) {
        memcpy(&gs_dscp_map_table.pbits[0], &dscp_map->pbits[0], sizeof(gs_dscp_map_table.pbits));
        gs_dscp_map_table.in_use = 1;
    
    }
    /* Case 2: to disable DSCP to P-bits mapping */
    else {
        memset((UINT8 *)&gs_dscp_map_table, 0, sizeof(gs_dscp_map_table));
        gs_dscp_map_table.in_use = 0;
    }
    
    MV_CPH_PRINT(CPH_DEBUG_LEVEL, "<----\n");

    return MV_OK;   
}

/******************************************************************************
* cph_flow_del_dscp_map()
* _____________________________________________________________________________
*
* DESCRIPTION: Deletes DSCP to P-bits mapping rules
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
INT32  cph_flow_del_dscp_map(VOID)
{
    MV_CPH_PRINT(CPH_DEBUG_LEVEL, "---->\n");

    /* Clear DSCP to P-bits mapping */
    memset((UINT8 *)&gs_dscp_map_table, 0, sizeof(gs_dscp_map_table));
    gs_dscp_map_table.in_use = 0;
    
    MV_CPH_PRINT(CPH_DEBUG_LEVEL, "<----\n");

    return MV_OK;
}

/******************************************************************************
* cph_flow_add_vlan()
* _____________________________________________________________________________
*
* DESCRIPTION: Add one VLAN tag behind of source MAC address.
*
* INPUTS:
*       mh     - Whether has MH or not
*       p_data - Pointer to packet
*       tpid   - Type of VLAN ID
*       vid    - VLAN to be added
*       pbits  - P-bits value
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       The shift of SKB data.
*******************************************************************************/
INLINE INT32 cph_flow_add_vlan(BOOL mh, UINT8 *p_data, UINT16 tpid, UINT16 vid, UINT8 pbits)
{
    UINT8  *p_new  = NULL;
    UINT16 *p_vlan = NULL;
    UINT32  len    = 0;

    p_new = p_data - MV_VLAN_HLEN;

    if (TRUE == mh)
        len = MV_ETH_MH_SIZE + MV_MAC_ADDR_SIZE + MV_MAC_ADDR_SIZE;
    else
        len = MV_MAC_ADDR_SIZE + MV_MAC_ADDR_SIZE;

    memmove(p_new, p_data, len);

    p_vlan = (UINT16 *)(p_new  + len);
    
    /* Set VLAN Type */
    *p_vlan = htons(tpid);
    p_vlan++;

    /* Set VID + priority */    
    *p_vlan = htons((vid & MV_VLAN_ID_MASK) | ((pbits & MV_PBITS_MASK) << MV_PBITS_SHIFT));

    return -MV_VLAN_HLEN;
}

/******************************************************************************
* cph_flow_del_vlan()
* _____________________________________________________________________________
*
* DESCRIPTION: Delete one VLAN tag behind of source MAC address.
*
* INPUTS:
*       mh     - Whether has MH or not
*       p_data - Pointer to packet.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       The shift of SKB data.
*******************************************************************************/
INLINE INT32 cph_flow_del_vlan(BOOL mh, UINT8 *p_data)
{
    UINT8  *p_new  = NULL;
    UINT32  len    = 0;

    p_new = p_data + MV_VLAN_HLEN;

    if (TRUE == mh)
        len = MV_ETH_MH_SIZE + MV_MAC_ADDR_SIZE + MV_MAC_ADDR_SIZE;
    else
        len = MV_MAC_ADDR_SIZE + MV_MAC_ADDR_SIZE;

    memmove(p_new, p_data, len);

    return MV_VLAN_HLEN;
}

/******************************************************************************
* cph_flow_strip_vlan()
* _____________________________________________________________________________
*
* DESCRIPTION: Delete all VLAN tags behind of source MAC address.
*
* INPUTS:
*       mh     - Whether has MH or not
*       p_data - Pointer to packet.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       The shift of SKB data.
*******************************************************************************/
INLINE INT32 cph_flow_strip_vlan(BOOL mh, UINT8 *p_data)
{
    INT32  offset       = 0;
    INT32  total_offset = 0;    
    UINT16 eth_type     = 0;
    UINT8 *p_field      = NULL;
    UINT32 len          = 0;

    if (TRUE == mh)
        len = MV_ETH_MH_SIZE + MV_MAC_ADDR_SIZE + MV_MAC_ADDR_SIZE;
    else
        len = MV_MAC_ADDR_SIZE + MV_MAC_ADDR_SIZE;

    p_field  = p_data + len;
    eth_type = ntohs(*(UINT16 *)p_field);

    while (eth_type == MV_TPID_8100 || eth_type == MV_TPID_88A8 || eth_type == MV_TPID_9100) 
    {
        offset   = cph_flow_del_vlan(mh, p_data+offset);
        
        p_field += offset;
        eth_type = ntohs(*(UINT16 *)p_field);
        total_offset += offset;
    }    

    return total_offset;
}

/******************************************************************************
* cph_flow_replace_vlan()
* _____________________________________________________________________________
*
* DESCRIPTION: Replace one VLAN tag behind of source MAC address.
*
* INPUTS:
*       mh     - Whether has MH or not
*       p_data - Pointer to packet
*       tpid   - Type of VLAN ID
*       vid    - VLAN to be added
*       pbits  - P-bits value
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       The shift of SKB data.
*******************************************************************************/
INLINE INT32 cph_flow_replace_vlan(BOOL mh, UINT8 *p_data, UINT16 tpid, UINT16 vid, UINT8 pbits)
{
    UINT16 *p_vlan = NULL;
    UINT32  len    = 0;

    if (TRUE == mh)
        len = MV_ETH_MH_SIZE + MV_MAC_ADDR_SIZE + MV_MAC_ADDR_SIZE;
    else
        len = MV_MAC_ADDR_SIZE + MV_MAC_ADDR_SIZE;

    p_vlan = (UINT16 *)(p_data + len);
    
    /* Set VLAN Type */
    *p_vlan = htons(tpid);
    p_vlan++;
    
    /* Set VID + priority */
    *p_vlan = htons((vid & MV_VLAN_ID_MASK) | ((pbits & MV_PBITS_MASK) << MV_PBITS_SHIFT));

    return 0;
}

/******************************************************************************
* cph_flow_swap_vlan()
* _____________________________________________________________________________
*
* DESCRIPTION: Swap between two VLAN tag.
*
* INPUTS:
*       mh     - Whether has MH or not
*       p_data - Pointer to packet
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       The shift of SKB data.
*******************************************************************************/
INLINE INT32 cph_flow_swap_vlan(BOOL mh, UINT8 *p_data)
{
    UINT32 *p_tci = NULL;
    UINT32  tci1  = 0;
    UINT32  tci2  = 0;
    UINT32  len    = 0;

    if (TRUE == mh)
        len = MV_ETH_MH_SIZE + MV_MAC_ADDR_SIZE + MV_MAC_ADDR_SIZE;
    else
        len = MV_MAC_ADDR_SIZE + MV_MAC_ADDR_SIZE;

    p_tci = (UINT32 *)(p_data + len);
    
    /* Save first TCI */
    tci1 = ntohl(*p_tci);
    p_tci++;

    /* Save second TCI and replace it w/ first TCI */
    tci2   = ntohl(*p_tci);
    *p_tci = htonl(tci1);

    /* Go back to replce first TCI */
    p_tci--;    
    *p_tci = htonl(tci2);
    
    return 0;
}

/******************************************************************************
* cph_flow_parse_packet()
* _____________________________________________________________________________
*
* DESCRIPTION: Parse packet and output flow information.
*
* INPUTS:
*       port - Source GMAC port
*       data - Pointer to packet
*       rx   - Whether in RX dir
*       mh   - Whether has Marvell header
*
* OUTPUTS:
*       flow - Flow parsing field values
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
MV_STATUS cph_flow_parse_packet (INT32 port, UINT8 *data, BOOL rx, BOOL mh, CPH_FLOW_ENTRY_T *flow)
{
    UINT16      eth_type = 0; 
    UINT8      *p_field  = NULL;
    UINT8       proto    = 0;
    MV_STATUS   rc       = MV_OK;
    struct ipv6hdr         *p_ipv6_hdr   = NULL;
    struct ipv6_hopopt_hdr *p_hopopt_hdr = NULL;
    struct icmp6hdr        *p_icmp_hdr   = NULL;     

    memset(flow, 0, sizeof(CPH_FLOW_ENTRY_T));
    
    /* Parse Direction */
    flow->dir = cph_app_parse_dir(port, rx);
    if(flow->dir == CPH_DIR_INVALID)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "dir[%d] is invalid \n", flow->dir);
        return MV_BAD_VALUE;
    }

    if (TRUE == mh)
    {
        /* Parse Marvell header */
        if (flow->dir == CPH_DIR_US)
            flow->mh = (ntohs(*(UINT16 *)data) & MV_VALID_MH_MASK);
        else
            flow->mh = (ntohs(*(UINT16 *)data) & MV_VALID_GH_MASK);
        
        flow->parse_bm |= CPH_FLOW_PARSE_MH;
        p_field  = data + MV_ETH_MH_SIZE + ETH_ALEN + ETH_ALEN;            
    }
    else
    {
        p_field  = data + ETH_ALEN + ETH_ALEN; 
    }

    /* Parse VLAN tag */
    eth_type = ntohs(*(UINT16 *)p_field);
    if (eth_type == MV_TPID_8100 || eth_type == MV_TPID_88A8 || eth_type == MV_TPID_9100) 
    {
        flow->parse_bm |= CPH_FLOW_PARSE_EXT_VLAN;
        
        flow->parse_outer_tci.tpid = ntohs(*(UINT16 *)p_field);
        p_field += MV_CPH_TPID_LEN;

        flow->parse_outer_tci.vid   = (ntohs(*(UINT16 *)p_field) & MV_VLAN_ID_MASK);
        flow->parse_outer_tci.pbits = ((ntohs(*(UINT16 *)p_field) >> MV_PBITS_SHIFT)& MV_PBITS_MASK);
        
        p_field += MV_CPH_VLAN_TAG_LEN;

        eth_type = ntohs(*(UINT16 *)p_field);
        if (eth_type == MV_TPID_8100 || eth_type == MV_TPID_88A8 || eth_type == MV_TPID_9100) 
        {
            flow->parse_bm &= ~CPH_FLOW_PARSE_EXT_VLAN;
            flow->parse_bm |= CPH_FLOW_PARSE_TWO_VLAN;
            
            flow->parse_inner_tci.tpid = ntohs(*(UINT16 *)p_field);
            p_field += MV_CPH_TPID_LEN;

            flow->parse_inner_tci.vid   = (ntohs(*(UINT16 *)p_field) & MV_VLAN_ID_MASK);
            flow->parse_inner_tci.pbits = ((ntohs(*(UINT16 *)p_field) >> MV_PBITS_SHIFT)& MV_PBITS_MASK);
            
            p_field += MV_CPH_VLAN_TAG_LEN;

            eth_type = ntohs(*(UINT16 *)p_field);
        }      
    }    
    while (eth_type == MV_TPID_8100 || eth_type == MV_TPID_88A8 || eth_type == MV_TPID_9100) {
        p_field += VLAN_HLEN;
        eth_type = ntohs(*(UINT16 *)p_field);
    }
    /* Parse Eth type */    
    flow->eth_type = eth_type;
    flow->parse_bm |= CPH_FLOW_PARSE_ETH_TYPE;

    /* Parse Multicast protocol */
    if (MV_CPH_ETH_TYPE_IPV4 == flow->eth_type) {
        p_field += MV_CPH_ETH_TYPE_LEN;
        p_field += MV_IPV4_PROTO_OFFSET;
        proto = *(UINT8 *)p_field;

        if (IPPROTO_IGMP == proto) {
            flow->parse_bm |= CPH_FLOW_PARSE_MC_PROTO;
            flow->ip_type   = IPPROTO_IGMP;
	}
    }
    else if (MV_CPH_ETH_TYPE_IPV6 == flow->eth_type) {
        p_ipv6_hdr = (struct ipv6hdr *)(p_field + MV_CPH_ETH_TYPE_LEN);

        if (NEXTHDR_HOP == p_ipv6_hdr->nexthdr) {
            p_hopopt_hdr = (struct ipv6_hopopt_hdr *)((UINT8 *)p_ipv6_hdr + sizeof(struct ipv6hdr));

            if (IPPROTO_ICMPV6 == p_hopopt_hdr->nexthdr) {
                p_icmp_hdr =  (struct icmp6hdr *)((UINT8 *)p_hopopt_hdr + ipv6_optlen(p_hopopt_hdr));
                
                switch (p_icmp_hdr->icmp6_type) {
                    case ICMPV6_MGM_QUERY:
                    case ICMPV6_MGM_REPORT:
                    case ICMPV6_MGM_REDUCTION:
                    case ICMPV6_MLD2_REPORT:
                        flow->parse_bm |= CPH_FLOW_PARSE_MC_PROTO;
			flow->ip_type = IPPROTO_ICMPV6;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    return rc;
}

/******************************************************************************
* cph_flow_compare_rules()
* _____________________________________________________________________________
*
* DESCRIPTION: Compare two flow rules.
*
* INPUTS:
*       parse_rule  - The parsing field values come from the packets
*       db_rule     - The flow rule stored in flow database
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       In case same, return TRUE, 
*       In case different, return FALSE.
*******************************************************************************/
BOOL cph_flow_compare_rules (CPH_FLOW_ENTRY_T *parse_rule, CPH_FLOW_ENTRY_T *db_rule)
{
    /* Check direction */
    if (parse_rule->dir != db_rule->dir)
        return FALSE;

    /* Check parse_bm */
    if (parse_rule->parse_bm != db_rule->parse_bm)
    {
        return FALSE;
    }  

    /* Check MH if needed */
    if (db_rule->parse_bm & CPH_FLOW_PARSE_MH)
    {
        if (parse_rule->mh != db_rule->mh)
            return FALSE;
    }

    /* Check if it is default rule */
    if (parse_rule->is_default != db_rule->is_default)
        return FALSE;
    
    /* Check VLAN ID */
    if (parse_rule->is_default == FALSE)
    {
        if ((db_rule->parse_bm & CPH_FLOW_PARSE_EXT_VLAN) ||  
            (db_rule->parse_bm & CPH_FLOW_PARSE_TWO_VLAN))
        {
            if (parse_rule->parse_outer_tci.tpid != db_rule->parse_outer_tci.tpid)
                return FALSE;

            if (parse_rule->parse_outer_tci.vid != db_rule->parse_outer_tci.vid)
                return FALSE;

            if (parse_rule->parse_outer_tci.pbits != db_rule->parse_outer_tci.pbits)
                return FALSE;
        }
        if (db_rule->parse_bm & CPH_FLOW_PARSE_TWO_VLAN)
        {
            if (parse_rule->parse_inner_tci.tpid != db_rule->parse_inner_tci.tpid)
                return FALSE;

            if (parse_rule->parse_inner_tci.vid != db_rule->parse_inner_tci.vid)
                return FALSE;

            if (parse_rule->parse_inner_tci.pbits != db_rule->parse_inner_tci.pbits)
                return FALSE;
        }
    }
    /* Check Ethernet type if needed */
    if (db_rule->parse_bm & CPH_FLOW_PARSE_ETH_TYPE)
    {
        if (parse_rule->eth_type != db_rule->eth_type)
            return FALSE;
    }

    if (db_rule->parse_bm & CPH_FLOW_PARSE_MC_PROTO)
    {
        if (parse_rule->ip_type != db_rule->ip_type)
            return FALSE;
    }

    return TRUE;
}

/******************************************************************************
* cph_flow_compare_packet_and_rule()
* _____________________________________________________________________________
*
* DESCRIPTION: Compare flow packet and rule.
*
* INPUTS:
*       packet_rule - The parsing field values come from the packets
*       db_rule     - The flow rule stored in flow database
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       In case same, return TRUE, 
*       In case different, return FALSE.
*******************************************************************************/
BOOL cph_flow_compare_packet_and_rule (CPH_FLOW_ENTRY_T *packet_rule, CPH_FLOW_ENTRY_T *db_rule)
{
    /* Check direction */
    if ((packet_rule->dir != db_rule->dir) &&
        (db_rule->dir != CPH_DIR_NOT_CARE))
        return FALSE;

    /* Check Multicast protocol */
     if ((db_rule->parse_bm & CPH_FLOW_PARSE_MC_PROTO) != (packet_rule->parse_bm & CPH_FLOW_PARSE_MC_PROTO))
    {
            return FALSE;
    }

	if (packet_rule->parse_bm & CPH_FLOW_PARSE_MC_PROTO) {
		if ((db_rule->ip_type != MV_CPH_IP_TYPE_ANY) && (db_rule->ip_type != packet_rule->ip_type))
			return false;
	}

    /* Check MH if needed */
    if ((db_rule->parse_bm & CPH_FLOW_PARSE_MH) &&
        (packet_rule->parse_bm & CPH_FLOW_PARSE_MH) )
    {
        if (packet_rule->mh != db_rule->mh)
            return FALSE;
    }

    /* Check if it is default rule */
    if (packet_rule->is_default != db_rule->is_default)
        return FALSE;
    
    /* Check VLAN ID */
    if ((packet_rule->parse_bm & (CPH_FLOW_PARSE_EXT_VLAN | CPH_FLOW_PARSE_TWO_VLAN))
         != (db_rule->parse_bm & (CPH_FLOW_PARSE_EXT_VLAN | CPH_FLOW_PARSE_TWO_VLAN)))
        return FALSE;
    if (packet_rule->is_default == FALSE)
    {
        if ((db_rule->parse_bm & CPH_FLOW_PARSE_EXT_VLAN) ||  
            (db_rule->parse_bm & CPH_FLOW_PARSE_TWO_VLAN))
        {
            if ((packet_rule->parse_outer_tci.tpid != db_rule->parse_outer_tci.tpid) &&
                (db_rule->parse_outer_tci.tpid != MV_CPH_TPID_NOT_CARE_VALUE))
                return FALSE;

            if ((packet_rule->parse_outer_tci.vid != db_rule->parse_outer_tci.vid) &&
                (db_rule->parse_outer_tci.vid != MV_CPH_VID_NOT_CARE_VALUE))
                return FALSE;

            if ((packet_rule->parse_outer_tci.pbits != db_rule->parse_outer_tci.pbits) &&
                (db_rule->parse_outer_tci.pbits != MV_CPH_PBITS_NOT_CARE_VALUE))
                return FALSE;
        }
        if (db_rule->parse_bm & CPH_FLOW_PARSE_TWO_VLAN)
        {
            if ((packet_rule->parse_inner_tci.tpid != db_rule->parse_inner_tci.tpid) &&
                (db_rule->parse_inner_tci.tpid != MV_CPH_TPID_NOT_CARE_VALUE))
                return FALSE;

            if ((packet_rule->parse_inner_tci.vid != db_rule->parse_inner_tci.vid) &&
                (db_rule->parse_inner_tci.vid != MV_CPH_VID_NOT_CARE_VALUE))
                return FALSE;

            if ((packet_rule->parse_inner_tci.pbits != db_rule->parse_inner_tci.pbits) &&
                (db_rule->parse_inner_tci.pbits != MV_CPH_PBITS_NOT_CARE_VALUE))
                return FALSE;
        }
    }
    /* Check Ethernet type if needed */
    if (db_rule->parse_bm & CPH_FLOW_PARSE_ETH_TYPE)
    {
        if (packet_rule->eth_type != db_rule->eth_type)
            return FALSE;
    }

    return TRUE;
}

/******************************************************************************
* cph_flow_compare_packet_and_rule_vid()
* _____________________________________________________________________________
*
* DESCRIPTION: Compare flow packet and rule w/ only VID.
*
* INPUTS:
*       packet_rule - The parsing field values come from the packets
*       db_rule     - The flow rule stored in flow database
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       In case same, return TRUE, 
*       In case different, return FALSE.
*******************************************************************************/
BOOL cph_flow_compare_packet_and_rule_vid (CPH_FLOW_ENTRY_T *packet_rule, CPH_FLOW_ENTRY_T *db_rule)
{
    /* Check direction */
    if ((packet_rule->dir != db_rule->dir) &&
        (db_rule->dir != CPH_DIR_NOT_CARE))
        return FALSE;

    /* Check Multicast protocol */
     if ((db_rule->parse_bm & CPH_FLOW_PARSE_MC_PROTO) != (packet_rule->parse_bm & CPH_FLOW_PARSE_MC_PROTO))
    {
            return FALSE;
    }

	if (packet_rule->parse_bm & CPH_FLOW_PARSE_MC_PROTO) {
		if ((db_rule->ip_type != MV_CPH_IP_TYPE_ANY) && (db_rule->ip_type != packet_rule->ip_type))
			return false;
	}

    /* Check MH if needed */
    if ((db_rule->parse_bm & CPH_FLOW_PARSE_MH) &&
        (packet_rule->parse_bm & CPH_FLOW_PARSE_MH) )
    {
        if (packet_rule->mh != db_rule->mh)
            return FALSE;
    }

    /* Check if it is default rule */
    if (packet_rule->is_default != db_rule->is_default)
        return FALSE;

    /* Check VLAN ID */
    if ((packet_rule->parse_bm & (CPH_FLOW_PARSE_EXT_VLAN | CPH_FLOW_PARSE_TWO_VLAN))
         != (db_rule->parse_bm & (CPH_FLOW_PARSE_EXT_VLAN | CPH_FLOW_PARSE_TWO_VLAN)))
        return FALSE;
    if (packet_rule->is_default == FALSE)
    {
        if ((db_rule->parse_bm & CPH_FLOW_PARSE_EXT_VLAN) ||
            (db_rule->parse_bm & CPH_FLOW_PARSE_TWO_VLAN))
        {
            if (db_rule->parse_outer_tci.tpid != MV_CPH_TPID_NOT_CARE_VALUE)
                return FALSE;

            if (db_rule->parse_outer_tci.pbits != MV_CPH_PBITS_NOT_CARE_VALUE)
                return FALSE;

            if ((packet_rule->parse_outer_tci.vid != db_rule->parse_outer_tci.vid) &&
                (db_rule->parse_outer_tci.vid != MV_CPH_VID_NOT_CARE_VALUE))
                return FALSE;
        }
        if (db_rule->parse_bm & CPH_FLOW_PARSE_TWO_VLAN)
        {
            if (db_rule->parse_inner_tci.tpid != MV_CPH_TPID_NOT_CARE_VALUE)
                return FALSE;

            if (db_rule->parse_inner_tci.pbits != MV_CPH_PBITS_NOT_CARE_VALUE)
                return FALSE;

            if ((packet_rule->parse_inner_tci.vid != db_rule->parse_inner_tci.vid) &&
                (db_rule->parse_inner_tci.vid != MV_CPH_VID_NOT_CARE_VALUE))
                return FALSE;
        }
    }
    /* Check Ethernet type if needed */
    if (db_rule->parse_bm & CPH_FLOW_PARSE_ETH_TYPE)
    {
        if (packet_rule->eth_type != db_rule->eth_type)
            return FALSE;
    }

    return TRUE;
}

/******************************************************************************
* cph_flow_mod_packet()
* _____________________________________________________________________________
*
* DESCRIPTION: Modify packet according to flow rule
*
* INPUTS:
*       skb        - Pointer to packet
*       mh         - Whether has MH or not
*       flow       - Flow parsing field values
*       out_offset - Offset of packet
*       rx         - Whether RX or TX
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
MV_STATUS cph_flow_mod_packet (struct sk_buff *skb,  BOOL mh, CPH_FLOW_ENTRY_T *flow, INT32 *out_offset)
{
    INT32  offset = 0;
    UINT16 tpid   = 0;    
    UINT16 vid    = 0;
    UINT8  pbits  = 0;
    BOOL   rc     = MV_OK;

    switch (flow->op_type)
    {
        case CPH_VLAN_OP_ASIS:
            break;            
        case CPH_VLAN_OP_DISCARD:
            break;            
        case CPH_VLAN_OP_ADD:
            tpid   = flow->mod_outer_tci.tpid ? flow->mod_outer_tci.tpid : MV_TPID_8100;
            offset = cph_flow_add_vlan(mh, skb->data, tpid, 
                                       flow->mod_outer_tci.vid, flow->mod_outer_tci.pbits);
            break;
        case CPH_VLAN_OP_ADD_COPY_DSCP:
            tpid   = flow->mod_outer_tci.tpid ? flow->mod_outer_tci.tpid : MV_TPID_8100;
            offset = cph_flow_add_vlan(mh, skb->data, tpid,
                                       flow->mod_outer_tci.vid, flow->mod_outer_tci.pbits);
            break;           
        case CPH_VLAN_OP_ADD_COPY_OUTER_PBIT:
            tpid   = flow->mod_outer_tci.tpid ? flow->mod_outer_tci.tpid : MV_TPID_8100;
            pbits  = (flow->parse_outer_tci.pbits == MV_CPH_PBITS_NOT_CARE_VALUE)? 0:flow->parse_outer_tci.pbits;
            offset = cph_flow_add_vlan(mh, skb->data, tpid,
                                       flow->mod_outer_tci.vid, pbits);
            break;
        case CPH_VLAN_OP_ADD_COPY_INNER_PBIT:
            tpid   = flow->mod_outer_tci.tpid ? flow->mod_outer_tci.tpid : MV_TPID_8100;
            pbits  = (flow->parse_inner_tci.pbits == MV_CPH_PBITS_NOT_CARE_VALUE)? 0:flow->parse_inner_tci.pbits;
            offset = cph_flow_add_vlan(mh, skb->data, tpid,
                                       flow->mod_outer_tci.vid, pbits);
            break;            
        case CPH_VLAN_OP_ADD_2_TAGS:
            tpid    = flow->mod_outer_tci.tpid ? flow->mod_outer_tci.tpid : MV_TPID_8100;
            offset  = cph_flow_add_vlan(mh, skb->data, tpid,
                                       flow->mod_outer_tci.vid, flow->mod_outer_tci.pbits);
            tpid    = flow->mod_inner_tci.tpid ? flow->mod_inner_tci.tpid : MV_TPID_8100;   
            offset += cph_flow_add_vlan(mh, skb->data, tpid,
                                       flow->mod_inner_tci.vid, flow->mod_inner_tci.pbits);
            break;            
        case CPH_VLAN_OP_ADD_2_TAGS_COPY_DSCP:
            tpid    = flow->mod_outer_tci.tpid ? flow->mod_outer_tci.tpid : MV_TPID_8100;
            offset  = cph_flow_add_vlan(mh, skb->data, tpid,
                                        flow->mod_outer_tci.vid, flow->mod_outer_tci.pbits);
            tpid    = flow->mod_inner_tci.tpid ? flow->mod_inner_tci.tpid : MV_TPID_8100;   
            offset += cph_flow_add_vlan(mh, skb->data, tpid,
                                        flow->mod_inner_tci.vid, flow->mod_inner_tci.pbits);            
            break;            
        case CPH_VLAN_OP_ADD_2_TAGS_COPY_PBIT:
            tpid   = flow->mod_outer_tci.tpid ? flow->mod_outer_tci.tpid : MV_TPID_8100;
            pbits  = (flow->parse_outer_tci.pbits == MV_CPH_PBITS_NOT_CARE_VALUE)? 0:flow->parse_outer_tci.pbits;
            offset = cph_flow_add_vlan(mh, skb->data, tpid,
                                       flow->mod_inner_tci.vid, pbits);
            tpid   = flow->mod_inner_tci.tpid ? flow->mod_inner_tci.tpid : MV_TPID_8100; 
            pbits  = (flow->parse_inner_tci.pbits == MV_CPH_PBITS_NOT_CARE_VALUE)? 0:flow->parse_inner_tci.pbits;
            offset += cph_flow_add_vlan(mh, skb->data, tpid,
                                        flow->mod_inner_tci.vid, pbits);            
            break;            
        case CPH_VLAN_OP_REM:
            offset = cph_flow_del_vlan(mh, skb->data);            
            break;            
        case CPH_VLAN_OP_REM_2_TAGS:
            offset  = cph_flow_del_vlan(mh, skb->data);
            offset += cph_flow_del_vlan(mh, skb->data);
            break;            
        case CPH_VLAN_OP_REPLACE:
            tpid   = flow->mod_outer_tci.tpid ? flow->mod_outer_tci.tpid : MV_TPID_8100;
            offset = cph_flow_replace_vlan(mh, skb->data, tpid, 
                                           flow->mod_outer_tci.vid, flow->mod_outer_tci.pbits);
            break;            
        case CPH_VLAN_OP_REPLACE_VID:
            tpid = flow->mod_outer_tci.tpid ? flow->mod_outer_tci.tpid : MV_TPID_8100;

            pbits  = (flow->parse_outer_tci.pbits == MV_CPH_PBITS_NOT_CARE_VALUE)? 0:flow->parse_outer_tci.pbits;
            offset = cph_flow_replace_vlan(mh, skb->data, tpid, 
                                           flow->mod_outer_tci.vid, pbits);
            break;            
        case CPH_VLAN_OP_REPLACE_PBIT:
            tpid = flow->mod_outer_tci.tpid ? flow->mod_outer_tci.tpid : MV_TPID_8100;

            vid    = (flow->parse_outer_tci.vid == MV_CPH_VID_NOT_CARE_VALUE)? 0:flow->parse_outer_tci.vid;
            offset = cph_flow_replace_vlan(mh, skb->data, tpid, 
                                           vid, flow->mod_outer_tci.pbits);
            break;            
        case CPH_VLAN_OP_REPLACE_INNER_ADD_OUTER:
            tpid = flow->mod_inner_tci.tpid ? flow->mod_inner_tci.tpid : MV_TPID_8100;
            offset = cph_flow_replace_vlan(mh, skb->data, tpid, 
                                           flow->mod_inner_tci.vid, flow->mod_inner_tci.pbits);
            tpid = flow->mod_outer_tci.tpid ? flow->mod_outer_tci.tpid : MV_TPID_8100;
            offset += cph_flow_add_vlan(mh, skb->data, tpid, 
                                       flow->mod_outer_tci.vid, flow->mod_outer_tci.pbits);
            break;            
        case CPH_VLAN_OP_REPLACE_INNER_ADD_OUTER_COPY_PBIT:
            tpid = flow->mod_inner_tci.tpid ? flow->mod_inner_tci.tpid : MV_TPID_8100;
            pbits  = (flow->parse_inner_tci.pbits == MV_CPH_PBITS_NOT_CARE_VALUE)? 0:flow->parse_inner_tci.pbits;
            offset = cph_flow_replace_vlan(mh, skb->data, tpid, 
                                           flow->mod_inner_tci.vid, pbits);
            tpid = flow->mod_outer_tci.tpid ? flow->mod_outer_tci.tpid : MV_TPID_8100;
            pbits   = (flow->parse_outer_tci.pbits == MV_CPH_PBITS_NOT_CARE_VALUE)? 0:flow->parse_outer_tci.pbits;            
            offset += cph_flow_add_vlan(mh, skb->data, tpid, 
                                        flow->mod_outer_tci.vid, pbits);
            break;            
        case CPH_VLAN_OP_REPLACE_INNER_REM_OUTER:
            offset  = cph_flow_del_vlan(mh, skb->data);
            tpid    = flow->mod_outer_tci.tpid ? flow->mod_outer_tci.tpid : MV_TPID_8100;
            offset += cph_flow_replace_vlan(mh, skb->data, tpid, 
                                            flow->mod_outer_tci.vid, flow->mod_outer_tci.pbits);
            break;            
        case CPH_VLAN_OP_REPLACE_2TAGS:
            tpid    = flow->mod_inner_tci.tpid ? flow->mod_inner_tci.tpid : MV_TPID_8100;
            offset  = cph_flow_replace_vlan(mh, skb->data, tpid,
                                           flow->mod_inner_tci.vid, flow->mod_inner_tci.pbits);
            offset += cph_flow_swap_vlan(mh, skb->data);
            tpid    = flow->mod_outer_tci.tpid ? flow->mod_outer_tci.tpid : MV_TPID_8100;
            offset += cph_flow_replace_vlan(mh, skb->data, tpid,
                                            flow->mod_outer_tci.vid, flow->mod_outer_tci.pbits);
            break;            
        case CPH_VLAN_OP_REPLACE_2TAGS_VID:
            tpid    = flow->mod_inner_tci.tpid ? flow->mod_inner_tci.tpid : MV_TPID_8100;
            pbits   = (flow->parse_inner_tci.pbits == MV_CPH_PBITS_NOT_CARE_VALUE)? 0:flow->parse_inner_tci.pbits;
            offset  = cph_flow_replace_vlan(mh, skb->data, tpid,
                                           flow->mod_inner_tci.vid, pbits);
            offset += cph_flow_swap_vlan(mh, skb->data);
            tpid    = flow->mod_outer_tci.tpid ? flow->mod_outer_tci.tpid : MV_TPID_8100;
            pbits   = (flow->parse_outer_tci.pbits == MV_CPH_PBITS_NOT_CARE_VALUE)? 0:flow->parse_outer_tci.pbits;
            offset += cph_flow_replace_vlan(mh, skb->data, tpid,
                                            flow->mod_outer_tci.vid, pbits);
            break;            
        case CPH_VLAN_OP_SWAP:
            offset  = cph_flow_swap_vlan(mh, skb->data);
            break;
        default:
            break;
    }

    /* Save SKB data offset */
    skb->data  += offset;
    skb->len   -= offset;
    *out_offset = offset;
    
    return rc;
}

/******************************************************************************
* cph_flow_mod_frwd()
* _____________________________________________________________________________
*
* DESCRIPTION: Modify forwarding parameter of transmiting packet according to flow rule
*
* INPUTS:
*       flow        - Flow parsing field values
*       tx_spec_out - TX descriptor
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
MV_STATUS cph_flow_mod_frwd (CPH_FLOW_ENTRY_T *flow, struct mv_eth_tx_spec *tx_spec_out)
{
    MV_STATUS rc = MV_OK;
    
    tx_spec_out->txp     = flow->pkt_frwd.trg_port;
    tx_spec_out->txq     = flow->pkt_frwd.trg_queue;
    if ((FALSE == cph_db_get_tcont_state(tx_spec_out->txp)) ||
        (flow->op_type == CPH_VLAN_OP_DISCARD))
        tx_spec_out->txq = MV_INVALID_QUEUE_NUM;
    tx_spec_out->hw_cmd  = ((flow->pkt_frwd.gem_port << 8)|0x0010);
    tx_spec_out->tx_func = NULL;
    tx_spec_out->flags   = MV_ETH_F_MH;

    return rc;
}

/******************************************************************************
* cph_flow_send_packet()
* _____________________________________________________________________________
*
* DESCRIPTION: CPH function to handle the received application packets 
*
* INPUTS:
*       dev_out     - Net device
*       pkt         - Marvell packet information
*       tx_spec_out - TX descriptor
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns 1. 
*       On error returns 0.
*******************************************************************************/
MV_STATUS cph_flow_send_packet(struct net_device *dev_out, struct eth_pbuf *pkt, 
    struct mv_eth_tx_spec *tx_spec_out)
{


#if 0
    struct eth_port *pp = MV_ETH_PRIV(dev_out);
    int frags = 0;
    bool tx_spec_ready = false;
    struct mv_eth_tx_spec tx_spec;
    u32 tx_cmd;
    struct tx_queue *txq_ctrl = NULL;
    struct neta_tx_desc *tx_desc;
    struct sk_buff  *skb;


    skb = (struct sk_buff *)(pkt->osInfo);

    read_lock(&pp->rwlock);

    if (!(netif_running(dev_out))) {
        printk(KERN_ERR "!netif_running() in %s\n", __func__);
        goto out;
    }

    /* Get TXQ (without BM) to send packet generated by Linux */
    if (tx_spec_ready == false) {
        tx_spec.txp    = tx_spec_out->txp;
        tx_spec.txq    = tx_spec_out->txq;
        tx_spec.hw_cmd = tx_spec_out->hw_cmd;
        tx_spec.flags  = tx_spec_out->flags;
    }

    txq_ctrl = &pp->txq_ctrl[tx_spec.txp * CONFIG_MV_ETH_TXQ + tx_spec.txq];
    if (txq_ctrl == NULL) {
        printk(KERN_ERR "%s: invalidate txp/txq (%d/%d)\n", __func__, tx_spec.txp, tx_spec.txq);
        goto out;
    }
    spin_lock_irqsave(&txq_ctrl->queue_lock);

#if 0
#ifdef CONFIG_MV_ETH_TSO
    /* GSO/TSO */
    if (skb_is_gso(skb)) {
        frags = mv_eth_tx_tso(skb, dev_out, &tx_spec, txq_ctrl);
        goto out;
    }
#endif /* CONFIG_MV_ETH_TSO */
#endif

    frags = 1;

#if 0
    if (tx_spec.flags & MV_ETH_F_MH) {
        if (tx_spec.flags & MV_ETH_F_SWITCH)
            mh = dev_priv->tx_vlan_mh;
        else
            mh = pp->tx_mh;

        if (mv_eth_skb_mh_add(skb, mh)) {
            frags = 0;
            goto out;
        }
    }
#endif
    tx_desc = mv_eth_tx_desc_get(txq_ctrl, frags);
    if (tx_desc == NULL) {
        frags = 0;
        goto out;
    }

    tx_cmd = NETA_TX_L4_CSUM_NOT;

#ifdef CONFIG_MV_PON
    tx_desc->hw_cmd = tx_spec.hw_cmd;
#endif

    /* FIXME: beware of nonlinear --BK */
    tx_desc->dataSize = skb_headlen(skb);

    tx_desc->bufPhysAddr = mvOsCacheFlush(NULL, skb->data, tx_desc->dataSize);

    if (frags == 1) {
        /*
         * First and Last descriptor
         */
        if (tx_spec.flags & MV_ETH_F_NO_PAD)
            tx_cmd |= NETA_TX_F_DESC_MASK | NETA_TX_L_DESC_MASK;
        else
            tx_cmd |= NETA_TX_FLZ_DESC_MASK;

        tx_desc->command = tx_cmd;
        mv_eth_tx_desc_flush(tx_desc);

        txq_ctrl->shadow_txq[txq_ctrl->shadow_txq_put_i] = ((MV_ULONG) skb | MV_ETH_SHADOW_SKB);
        mv_eth_shadow_inc_put(txq_ctrl);
    } 

    txq_ctrl->txq_count += frags;

#ifdef CONFIG_MV_ETH_DEBUG_CODE
    if (pp->flags & MV_ETH_F_DBG_TX) {
        printk(KERN_ERR "\n");
        printk(KERN_ERR "%s - eth_tx_%lu: port=%d, txp=%d, txq=%d, skb=%p, head=%p, data=%p, size=%d\n",
               dev_out->name, dev_out->stats.tx_packets, pp->port, tx_spec.txp, tx_spec.txq, skb,
               skb->head, skb->data, skb->len);
        mv_eth_tx_desc_print(tx_desc);
    }
#endif /* CONFIG_MV_ETH_DEBUG_CODE */

#ifdef CONFIG_MV_PON
    if (MV_PON_PORT(pp->port))
        mvNetaPonTxqBytesAdd(pp->port, tx_spec.txp, tx_spec.txq, skb->len);
#endif /* CONFIG_MV_PON */

    /* Enable transmit */
    mvNetaTxqPendDescAdd(pp->port, tx_spec.txp, tx_spec.txq, frags);

    STAT_DBG(txq_ctrl->stats.txq_tx += frags);

out:
    if (frags > 0) {
        dev_out->stats.tx_packets++;
        dev_out->stats.tx_bytes += skb->len;
    } else {
        dev_out->stats.tx_dropped++;
        dev_kfree_skb_any(skb);
    }

#ifndef CONFIG_MV_ETH_TXDONE_ISR
    if (txq_ctrl) {
        if (txq_ctrl->txq_count >= mv_ctrl_txdone) {
            u32 tx_done = mv_eth_txq_done(pp, txq_ctrl);

            STAT_DIST((tx_done < pp->dist_stats.tx_done_dist_size) ? pp->dist_stats.tx_done_dist[tx_done]++ : 0);

        }
        /* If after calling mv_eth_txq_done, txq_ctrl->txq_count equals frags, we need to set the timer */
        if ((txq_ctrl->txq_count == frags) && (frags > 0))
            mv_eth_add_tx_done_timer(pp);
    }
#endif /* CONFIG_MV_ETH_TXDONE_ISR */

    if (txq_ctrl)
        spin_unlock_irqrestore(&txq_ctrl->queue_lock);

    read_unlock(&pp->rwlock);

#endif    
    return MV_OK;
}

/******************************************************************************
* cph_flow_lookup_op_type()
* _____________________________________________________________________________
*
* DESCRIPTION:lookup operation type string according to value
*
* INPUTS:
*       enum_value - The enum value to be matched
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Enum string
*******************************************************************************/
CHAR *cph_flow_lookup_op_type(INT32 enum_value)
{
    return mtype_lookup_enum_str(&g_enum_array_op_type, enum_value);
}

/******************************************************************************
* cph_flow_display_all()
* _____________________________________________________________________________
*
* DESCRIPTION: The function displays valid flow mapping tables and DSCP 
*              to P-bits mapping tablefor untagged frames.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
INT32  cph_flow_display_all (VOID)
{
    UINT32            idx         = 0;
    UINT32            rule_idx    = 0;
    CPH_FLOW_ENTRY_T *p_flow_rule = NULL;
    INT32             offset      = 0;
    UINT8             buff[512];

    /* Print flow rule entries */
    printk(KERN_INFO "MV_CPH Flow Rule Table\n----------------------------------\n");
    printk(KERN_INFO "Total rule number:%d, Max rule number:%d \n", gs_flow_table.rule_num, CPH_FLOW_ENTRY_NUM);

    printk(KERN_INFO "-------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    printk(KERN_INFO "                                |Parse outer       |Parse inner       |Mod outer         |Mod Inner         |Forward\n");
    printk(KERN_INFO "dir default parse_bm mh   ety    tpid   vid  pbits  tpid   vid  pbits  tpid   vid  pbits  tpid   vid  pbits  port queue hwf_queue gem  count    ip_type op_type\n");
    /* Traverse CPH flow rule tale */
    for (idx = 0, rule_idx = 0; (idx < CPH_FLOW_ENTRY_NUM) && (rule_idx < gs_flow_table.rule_num); idx++)
    {
        p_flow_rule = &gs_flow_table.flow_rule[idx];
        
        /* Compare parse_bm and parse_key */
        if (p_flow_rule->valid == TRUE)
        {
            rule_idx++;

            printk(KERN_INFO  
                   "%2.2s  %3.3s     0x%04x   %-4d 0x%04x 0x%04x %-4d %1d      0x%04x %-4d %1d      0x%04x %-4d %1d      0x%04x %-4d %1d      %1d    %1d     %1d         %-4d %-8d %7d %s\n",
                   cph_app_lookup_dir(p_flow_rule->dir), (p_flow_rule->is_default == TRUE)? "Yes":"No ", 
                   p_flow_rule->parse_bm,             p_flow_rule->mh,                  p_flow_rule->eth_type,
                   p_flow_rule->parse_outer_tci.tpid, p_flow_rule->parse_outer_tci.vid, p_flow_rule->parse_outer_tci.pbits, 
                   p_flow_rule->parse_inner_tci.tpid, p_flow_rule->parse_inner_tci.vid, p_flow_rule->parse_inner_tci.pbits,
                   p_flow_rule->mod_outer_tci.tpid,   p_flow_rule->mod_outer_tci.vid,   p_flow_rule->mod_outer_tci.pbits,
                   p_flow_rule->mod_inner_tci.tpid,   p_flow_rule->mod_inner_tci.vid,   p_flow_rule->mod_inner_tci.pbits,
                   p_flow_rule->pkt_frwd.trg_port,    p_flow_rule->pkt_frwd.trg_queue,  p_flow_rule->pkt_frwd.trg_hwf_queue, p_flow_rule->pkt_frwd.gem_port, 
                   p_flow_rule->count,                p_flow_rule->ip_type,             cph_flow_lookup_op_type(p_flow_rule->op_type));
        }
    }   

    /* Print  DSCP to P-bits mapping table */
    printk(KERN_INFO "\nMV_CPH DSCP to P-bits Mapping Table\n----------------------------------\n");  
    if (gs_dscp_map_table.in_use == 0) {
        printk(KERN_INFO "No DSCP to P-bits mapping\n");
    }
    else {
        printk(KERN_INFO "DSCP[Pbits]\n");

        memset(buff, 0, sizeof(buff));
        for (idx = 0; idx < MV_CPH_DSCP_PBITS_TABLE_MAX_SIZE; idx++) {
            offset += sprintf(buff+offset, "%2.2d[%2.2d] ", idx, gs_dscp_map_table.pbits[idx]);
            if (((idx+1) % 16) == 0)
                offset += sprintf(buff+offset, "\n");
        }
        printk(KERN_INFO "%s\n", buff);
    }
    printk(KERN_INFO "\n");
 
    return MV_OK; 
}

/******************************************************************************
* cph_flow_init()
* _____________________________________________________________________________
*
* DESCRIPTION: Initializes CPH flow mapping data structure.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK.
*       On error returns error code accordingly.
*******************************************************************************/
INT32  cph_flow_init(VOID)
{
    MV_STATUS rc = MV_OK;

    rc = cph_flow_db_init();
    CHECK_API_RETURN_AND_LOG_ERROR(rc, "Fail to call cph_flow_db_init");
        
    return rc;
}
