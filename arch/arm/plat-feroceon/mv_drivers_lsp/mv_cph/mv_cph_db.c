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
* mv_cph_db.c
*
* DESCRIPTION: Marvell CPH(CPH Packet Handler) data base implementation
*
* DEPENDENCIES:
*               None
*
* CREATED BY:   VictorGu
*
* DATE CREATED: 22Jan2013
*
* FILE REVISION NUMBER:
*               Revision: 1.0
*
*
*******************************************************************************/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/poll.h>
#include <linux/clk.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/miscdevice.h>

#include "mv_cph_header.h"
#include "ezxml.h"


/******************************************************************************
 * Variable Definition
 ******************************************************************************/
static CPH_APP_DB_T g_cph_app_db;
CHAR *g_cph_xml_cfg_file         = US_CPH_XML_CFG_FILE;
CHAR *g_onu_profile_xml_cfg_file = US_ONU_PROFILE_XML_CFG_FILE;


/******************************************************************************
 * Function Definition
 ******************************************************************************/
/******************************************************************************
* cph_db_set_param()
* _____________________________________________________________________________
*
* DESCRIPTION: Set CPH DB parameter
*
* INPUTS:
*       param   - The parameter type
*       value   - Parameter value
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK. 
*       On error returns error code accordingly.
*******************************************************************************/
MV_STATUS cph_db_set_param (CPH_DB_PARAM_E param, VOID *value)
{
    MV_STATUS rc = MV_OK;

    switch (param)
    {
        case CPH_DB_PARAM_PROFILE_ID:
            g_cph_app_db.profile_id   = *(tpm_eth_complex_profile_t *)value;
            break;
        case CPH_DB_PARAM_ACTIVE_PORT:
            g_cph_app_db.active_port  = *(MV_APP_GMAC_PORT_E *)value;
            break;
        case CPH_DB_PARAM_APP_SUPPORT:
            g_cph_app_db.app_support  = *(BOOL *)value;
            break;            
        case CPH_DB_PARAM_IGMP_SUPPORT:
            g_cph_app_db.igmp_support = *(BOOL *)value;
            break;
        case CPH_DB_PARAM_BC_SUPPORT:
            g_cph_app_db.bc_support   = *(BOOL *)value;
            break;
        case CPH_DB_PARAM_FLOW_SUPPORT:
            g_cph_app_db.flow_support = *(BOOL *)value;
            break;
        case CPH_DB_PARAM_UDP_SUPPORT:
            g_cph_app_db.udp_support  = *(BOOL *)value;            
            break;
        case CPH_DB_PARAM_BC_COUNTER:
            g_cph_app_db.bc_count     = *(UINT32 *)value;            
            break;
        default:
            break;
    }

    return rc;
}

/******************************************************************************
* cph_db_get_param()
* _____________________________________________________________________________
*
* DESCRIPTION: Get CPH DB parameter
*
* INPUTS:
*       param   - The parameter type
*
* OUTPUTS:
*       value   - Parameter value
*
* RETURNS:
*       On success, the function returns MV_OK. 
*       On error returns error code accordingly.
*******************************************************************************/
MV_STATUS cph_db_get_param (CPH_DB_PARAM_E param, VOID *value)
{
    MV_STATUS rc = MV_OK;

    switch (param)
    {
        case CPH_DB_PARAM_PROFILE_ID:
            *(tpm_eth_complex_profile_t *)value = g_cph_app_db.profile_id;
            break;
        case CPH_DB_PARAM_ACTIVE_PORT:
            *(MV_APP_GMAC_PORT_E *)value = g_cph_app_db.active_port;
            break;
        case CPH_DB_PARAM_APP_SUPPORT:
            *(BOOL *)value = g_cph_app_db.app_support;
            break;            
        case CPH_DB_PARAM_IGMP_SUPPORT:
            *(BOOL *)value = g_cph_app_db.igmp_support;
            break;
        case CPH_DB_PARAM_BC_SUPPORT:
            *(BOOL *)value = g_cph_app_db.bc_support;
            break;
        case CPH_DB_PARAM_FLOW_SUPPORT:
            *(BOOL *)value = g_cph_app_db.flow_support;
            break;
        case CPH_DB_PARAM_UDP_SUPPORT:
            *(BOOL *)value = g_cph_app_db.udp_support;
            break;
        case CPH_DB_PARAM_BC_COUNTER:
            *(UINT32 *)value = g_cph_app_db.bc_count;
            break;            
        default:
            break;
    }
    return rc;
}

/******************************************************************************
* cph_db_compare_rules()
* _____________________________________________________________________________
*
* DESCRIPTION: Compare the parse_bm and parse_key of two rules
*
* INPUTS:
*       parse_bm_1   - Parsing bitmap of first CPH rule
*       parse_key_1  - Parsing key of first CPH rule
*       parse_bm_2   - Parsing bitmap of second CPH rule
*       parse_key_2  - Parsing key of second CPH rule
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       In case same, return TRUE, 
*       In case different, return FALSE.
*******************************************************************************/
BOOL cph_db_compare_rules (
    CPH_APP_PARSE_FIELD_E parse_bm_1, 
    CPH_APP_PARSE_T      *parse_key_1, 
    CPH_APP_PARSE_FIELD_E parse_bm_2,
    CPH_APP_PARSE_T      *parse_key_2)
{
    if (parse_bm_1 == parse_bm_2)
    {
        /* Compare direction */
        if (parse_bm_1 & CPH_APP_PARSE_FIELD_DIR)
        {
            if (parse_key_1->dir != parse_key_2->dir)
            {
                MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Old dir[%d], new dir[%d] \n", parse_key_1->dir, parse_key_2->dir);
                return FALSE;
            }
        }

        /* Compare RX direction */
        if (parse_bm_1 & CPH_APP_PARSE_FIELD_RX_TX)
        {
            if (parse_key_1->rx_tx != parse_key_2->rx_tx)
            {
                MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Old rx_tx[%d], new rx_tx[%d] \n", parse_key_1->rx_tx, parse_key_2->rx_tx);
                return FALSE;
            }
        }
        
        /* Compare Marvell header */
        if (parse_bm_1 & CPH_APP_PARSE_FIELD_MH)
        {
            if (parse_key_1->mh != parse_key_2->mh)
            {
                MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Old mh[%d], new mh[%d] \n", parse_key_1->mh, parse_key_2->mh);
                return FALSE;
            }
        }

        /* Compare Eth type */
        if (parse_bm_1 & CPH_APP_PARSE_FIELD_ETH_TYPE)
        {
            if (parse_key_1->eth_type!= parse_key_2->eth_type)
            {
                MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Old eth_type[%d], new eth_type[%d] \n", parse_key_1->eth_type, parse_key_2->eth_type);
                return FALSE;
            }
        }
        
        /* Compare Eth subtype */
        if (parse_bm_1 & CPH_APP_PARSE_FIELD_ETH_SUBTYPE)
        {
            if (parse_key_1->eth_subtype != parse_key_2->eth_subtype)
            {
                MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Old eth_subtype[%d], new eth_subtype[%d] \n", parse_key_1->eth_subtype, parse_key_2->eth_subtype);
                return FALSE;
            }
        }

        /* Compare IPV4 type */
        if (parse_bm_1 & CPH_APP_PARSE_FIELD_IPV4_TYPE)
        {
            if (parse_key_1->ipv4_type != parse_key_2->ipv4_type)
            {
                MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Old ipv4_type[%d], new ipv4_type[%d] \n", parse_key_1->ipv4_type, parse_key_2->ipv4_type);
                return FALSE;
            }
        }

        /* Compare IPV6 type */
        if (parse_bm_1 & CPH_APP_PARSE_FIELD_IPV6_NH1)
        {
            if (parse_key_1->ipv6_nh1 != parse_key_2->ipv6_nh1)
            {
                MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Old ipv6_nh1[%d], new ipv6_nh1[%d] \n", parse_key_1->ipv6_nh1, parse_key_2->ipv6_nh1);
                return FALSE;
            }
        }

        /* Compare IPv6 NH */
        if (parse_bm_1 & CPH_APP_PARSE_FIELD_IPV6_NH2)
        {
            if (parse_key_1->ipv6_nh2!= parse_key_2->ipv6_nh2)
            {
                MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Old ipv6_nh2[%d], new ipv6_nh2[%d] \n", parse_key_1->ipv6_nh2, parse_key_2->ipv6_nh2);
                return FALSE;
            }
        }

        /* Compare ICMPv6 type */
        if (parse_bm_1 & CPH_APP_PARSE_FIELD_ICMPV6_TYPE)
        {
            if (parse_key_1->icmpv6_type != parse_key_2->icmpv6_type)
            {
                MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Old icmpv6_type[%d], new icmpv6_type[%d] \n", parse_key_1->icmpv6_type, parse_key_2->icmpv6_type);
                return FALSE;
            }
        }

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/******************************************************************************
* cph_db_compare_rule_and_packet()
* _____________________________________________________________________________
*
* DESCRIPTION: Compare the parse_bm and parse_key of CPH and the packet
*
* INPUTS:
*       parse_bm_rule     - Parsing bitmap of CPH rule
*       parse_key_rule    - Parsing key of CPH rule
*       parse_bm_packet   - Parsing bitmap of packet
*       parse_key_packet  - Parsing key of packet
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       In case same, return TRUE, 
*       In case different, return FALSE.
*******************************************************************************/
BOOL cph_db_compare_rule_and_packet (
    CPH_APP_PARSE_FIELD_E parse_bm_rule,
    CPH_APP_PARSE_T      *parse_key_rule,
    CPH_APP_PARSE_FIELD_E parse_bm_packet,
    CPH_APP_PARSE_T      *parse_key_packet)
{  
    /* Check direction */
    if (parse_bm_rule & CPH_APP_PARSE_FIELD_DIR)
    {
        if (parse_key_rule->dir != CPH_DIR_NOT_CARE)
        {
            if (! (parse_bm_packet & CPH_APP_PARSE_FIELD_DIR))
            {
                MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Packet has no dir field\n");
                return FALSE;
            }
            
            if (parse_key_rule->dir != parse_key_packet->dir)
            {
                MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Packet dir[%d] is different with rule dir[%d], mis-mathced!\n", parse_key_packet->dir, parse_key_rule->dir);
                return FALSE;
            }
        }
    }

    /* Check RX/TX direction */
    if (parse_bm_rule & CPH_APP_PARSE_FIELD_RX_TX)
    {
        if (parse_key_rule->rx_tx != CPH_RX_TX_NOT_CARE)
        {
            if (! (parse_bm_packet & CPH_APP_PARSE_FIELD_RX_TX))
            {
                MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Packet has no dir RX/TX field\n");
                return FALSE;
            }
            
            if (parse_key_rule->rx_tx != parse_key_packet->rx_tx)
            {
                MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Packet rx_tx[%d] is different with rule rx_tx[%d], mis-mathced!\n", parse_key_packet->rx_tx, parse_key_rule->rx_tx);
                return FALSE;
            }
        }
    }

    /* Check Marvell header */
    if (parse_bm_rule & CPH_APP_PARSE_FIELD_MH)
    {
        if (! (parse_bm_packet & CPH_APP_PARSE_FIELD_MH))
        {
            MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Packet has no MH field\n");
            return FALSE;
        }
        
        if (parse_key_rule->mh != parse_key_packet->mh)
        {
            MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Packet mh[0x%x] is different with rule mh[0x%x], mis-mathced!\n", parse_key_packet->mh, parse_key_rule->mh);
            return FALSE;
        }
    }

    /* Check Eth type */
    if (parse_bm_rule & CPH_APP_PARSE_FIELD_ETH_TYPE)
    {
        if (! (parse_bm_packet & CPH_APP_PARSE_FIELD_ETH_TYPE))
        {
            MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Packet has no eth_type field\n");
            return FALSE;
        }
        
        if (parse_key_rule->eth_type != parse_key_packet->eth_type)
        {
            MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Packet eth_type[0x%x] is different with rule eth_type[0x%x], mis-mathced!\n", parse_key_packet->eth_type, parse_key_rule->eth_type);
            return FALSE;
        }
    }

    /* Check Eth subtype */
    if (parse_bm_rule & CPH_APP_PARSE_FIELD_ETH_SUBTYPE)
    {
        if (! (parse_bm_packet & CPH_APP_PARSE_FIELD_ETH_SUBTYPE))
        {
            MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Packet has no eth_subtype field\n");
            return FALSE;
        }
        
        if (parse_key_rule->eth_subtype != parse_key_packet->eth_subtype)
        {
            MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Packet eth_subtype[%d] is different with rule eth_subtype[%d], mis-mathced!\n", parse_key_packet->eth_subtype, parse_key_rule->eth_subtype);
            return FALSE;
        }
    }

    /* Check IPV4 type */
    if (parse_bm_rule & CPH_APP_PARSE_FIELD_IPV4_TYPE)
    {
        if (! (parse_bm_packet & CPH_APP_PARSE_FIELD_IPV4_TYPE))
        {
            MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Packet has no ipv4_type field\n");
            return FALSE;
        }
        
        if (parse_key_rule->ipv4_type != parse_key_packet->ipv4_type)
        {
            MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Packet ipv4_type[%d] is different with rule ipv4_type[%d], mis-mathced!\n", parse_key_packet->ipv4_type, parse_key_rule->ipv4_type);
            return FALSE;
        }
    }

    /* Check IPV6 NH1 */
    if (parse_bm_rule & CPH_APP_PARSE_FIELD_IPV6_NH1)
    {
        if (! (parse_bm_packet & CPH_APP_PARSE_FIELD_IPV6_NH1))
        {
            MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Packet has no ipv6_nh1 field\n");
            return FALSE;
        }
        
        if (parse_key_rule->ipv6_nh1 != parse_key_packet->ipv6_nh1)
        {
            MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Packet ipv6_nh1[%d] is different with rule ipv6_nh1[%d], mis-mathced!\n", parse_key_packet->ipv6_nh1, parse_key_rule->ipv6_nh1);
            return FALSE;
        }
    }

    /* Check IPv6 NH2 */
    if (parse_bm_rule & CPH_APP_PARSE_FIELD_IPV6_NH2)
    {
        if (! (parse_bm_packet & CPH_APP_PARSE_FIELD_IPV6_NH2))
        {
            MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Packet has no ipv6_nh2 field\n");
            return FALSE;
        }
        
        if (parse_key_rule->ipv6_nh2 != parse_key_packet->ipv6_nh2)
        {
            MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Packet ipv6_nh2[%d] is different with rule ipv6_nh2[%d], mis-mathced!\n", parse_key_packet->ipv6_nh2, parse_key_rule->ipv6_nh2);
            return FALSE;
        }
    }

    /* Check ICMPv6 type */
    if (parse_bm_rule & CPH_APP_PARSE_FIELD_ICMPV6_TYPE)
    {
        if (! (parse_bm_packet & CPH_APP_PARSE_FIELD_ICMPV6_TYPE))
        {
            MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Packet has no icmpv6_type field\n");
            return FALSE;
        }
        
        if (parse_key_rule->icmpv6_type != parse_key_packet->icmpv6_type)
        {
            MV_CPH_PRINT(CPH_DEBUG_LEVEL, "Packet icmpv6_type[%d] is different with rule icmpv6_type[%d], mis-mathced!\n", parse_key_packet->icmpv6_type, parse_key_rule->icmpv6_type);
            return FALSE;
        }
    }

    return TRUE;
}

/******************************************************************************
* cph_db_check_duplicate_rule()
* _____________________________________________________________________________
*
* DESCRIPTION: Check whether there is duplicate CPH rule w/ same parse bitmap
*              value
*
* INPUTS:
*       parse_bm   - Parsing bitmap
*       parse_key  - Parsing key
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       In case has duplicated rule, return TRUE, 
*       In case has not duplicated rule, return FALSE.
*******************************************************************************/
BOOL cph_db_check_duplicate_rule (
    CPH_APP_PARSE_FIELD_E parse_bm, 
    CPH_APP_PARSE_T      *parse_key)
{
    UINT32           idx         = 0;
    UINT32           rule_idx    = 0;
    CPH_APP_RULE_T  *p_cph_rule  = NULL;
    BOOL             rc          = FALSE;
   
    for (idx = 0, rule_idx = 0; (idx < CPH_APP_MAX_RULE_NUM) && (rule_idx < g_cph_app_db.rule_num); idx++)
    {
        p_cph_rule = &g_cph_app_db.cph_rule[idx];
        
        /* Compare parse_bm and parse_key */
        if (p_cph_rule->valid == TRUE)
        {
            rule_idx++;

            rc = cph_db_compare_rules(p_cph_rule->parse_bm, &p_cph_rule->parse_key, parse_bm, parse_key);
            if (rc == TRUE)
            {
                return rc;
            }
        }
    }

    return rc;
}

/******************************************************************************
* cph_db_add_app_rule()
* _____________________________________________________________________________
*
* DESCRIPTION: Add application type CPH rule to data base
*
* INPUTS:
*       parse_bm   - Parsing bitmap
*       parse_key  - Parsing key
*       mod_bm     - Modification bitmap
*       mod_value  - Modification value
*       frwd_bm    - Forwarding bitmap
*       frwd_value - Forwarding value
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK. 
*       On error returns error code accordingly.
*******************************************************************************/
MV_STATUS cph_db_add_app_rule (
    CPH_APP_PARSE_FIELD_E parse_bm, 
    CPH_APP_PARSE_T      *parse_key,
    CPH_APP_MOD_FIELD_E   mod_bm,
    CPH_APP_MOD_T        *mod_value,
    CPH_APP_FRWD_FIELD_E  frwd_bm, 
    CPH_APP_FRWD_T       *frwd_value)
{
    UINT32           idx        = 0;
    CPH_APP_RULE_T  *p_cph_rule = NULL;
    BOOL             rc         = TRUE;
    unsigned long    flags;
    
    spin_lock_irqsave(&g_cph_app_db.app_lock, flags);
    /* Seach for an free entry */
    for (idx = 0; idx < CPH_APP_MAX_RULE_NUM; idx++)
    {
        if (g_cph_app_db.cph_rule[idx].valid == FALSE)
            break;
    }

    /* No free entry */
    if (idx == CPH_APP_MAX_RULE_NUM)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "No free CPH entry \n");
        spin_unlock_irqrestore(&g_cph_app_db.app_lock, flags);
        return MV_FULL;
    }

    /* Do not add new rule if there is already duplicated rule */
    rc = cph_db_check_duplicate_rule(parse_bm, parse_key);
    if (rc == TRUE)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "Already has duplicated rule, could not add new CPH rule \n");
        spin_unlock_irqrestore(&g_cph_app_db.app_lock, flags);
        return MV_ERROR;
    }        
    
    /* Save CPH rule for application packet */
    p_cph_rule = &g_cph_app_db.cph_rule[idx];
    p_cph_rule->parse_bm = parse_bm;
    memcpy(&p_cph_rule->parse_key,  parse_key,  sizeof(CPH_APP_PARSE_T));
    p_cph_rule->mod_bm   = mod_bm;
    memcpy(&p_cph_rule->mod_value,  mod_value,  sizeof(CPH_APP_MOD_T));
    p_cph_rule->frwd_bm  = frwd_bm;
    memcpy(&p_cph_rule->frwd_value, frwd_value, sizeof(CPH_APP_FRWD_T));
    p_cph_rule->valid    = TRUE;
    g_cph_app_db.rule_num ++;

    spin_unlock_irqrestore(&g_cph_app_db.app_lock, flags);
    return MV_OK;
}

/******************************************************************************
* cph_db_del_app_rule()
* _____________________________________________________________________________
*
* DESCRIPTION: Delete application type CPH rule from data base
*
* INPUTS:
*       parse_bm   - Parsing bitmap
*       parse_key  - Parsing key
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK. 
*       On error returns error code accordingly.
*******************************************************************************/
MV_STATUS cph_db_del_app_rule (
    CPH_APP_PARSE_FIELD_E parse_bm, 
    CPH_APP_PARSE_T      *parse_key)
{
    UINT32           idx         = 0;
    UINT32           rule_idx    = 0;
    CPH_APP_RULE_T  *p_cph_rule  = NULL;
    BOOL             rc          = FALSE;
    unsigned long    flags;

    spin_lock_irqsave(&g_cph_app_db.app_lock, flags);
    for (idx = 0, rule_idx = 0; (idx < CPH_APP_MAX_RULE_NUM) && (rule_idx < g_cph_app_db.rule_num); idx++)
    {
        p_cph_rule = &g_cph_app_db.cph_rule[idx];
        
        /* Compare parse_bm and parse_key */
        if (p_cph_rule->valid == TRUE)
        {
            rule_idx++;

            rc = cph_db_compare_rules(p_cph_rule->parse_bm, &p_cph_rule->parse_key, parse_bm, parse_key);
            if (rc == TRUE)
            {
                memset(p_cph_rule, 0, sizeof(CPH_APP_RULE_T));
                p_cph_rule->valid = FALSE;
                g_cph_app_db.rule_num --;
                
                spin_unlock_irqrestore(&g_cph_app_db.app_lock, flags);
                return MV_OK;
            }
        }
    }
    spin_unlock_irqrestore(&g_cph_app_db.app_lock, flags);

    return MV_OK;
}

/******************************************************************************
* cph_db_update_app_rule()
* _____________________________________________________________________________
*
* DESCRIPTION: Update application type CPH rule from data base 
*
* INPUTS:
*       parse_bm   - Parsing bitmap
*       parse_key  - Parsing key
*       mod_bm     - Modification bitmap
*       mod_value  - Modification value
*       frwd_bm    - Forwarding bitmap
*       frwd_value - Forwarding value
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK. 
*       On error returns error code accordingly.
*******************************************************************************/
MV_STATUS cph_db_update_app_rule (
    CPH_APP_PARSE_FIELD_E parse_bm, 
    CPH_APP_PARSE_T      *parse_key,  
    CPH_APP_MOD_FIELD_E   mod_bm, 
    CPH_APP_MOD_T        *mod_value, 
    CPH_APP_FRWD_FIELD_E  frwd_bm, 
    CPH_APP_FRWD_T       *frwd_value)
{
    UINT32           idx         = 0;
    UINT32           rule_idx    = 0;
    CPH_APP_RULE_T  *p_cph_rule  = NULL;
    BOOL             rc          = FALSE;
    unsigned long    flags;

    spin_lock_irqsave(&g_cph_app_db.app_lock, flags);   
    for (idx = 0, rule_idx = 0; (idx < CPH_APP_MAX_RULE_NUM) && (rule_idx < g_cph_app_db.rule_num); idx++)
    {
        p_cph_rule = &g_cph_app_db.cph_rule[idx];
        
        /* Compare parse_bm and parse_key */
        if (p_cph_rule->valid == TRUE)
        {
            rule_idx++;

            rc = cph_db_compare_rules(p_cph_rule->parse_bm, &p_cph_rule->parse_key, parse_bm, parse_key);
            if (rc == TRUE)
            {
                p_cph_rule->mod_bm   = mod_bm;
                memcpy(&p_cph_rule->mod_value,  mod_value,  sizeof(CPH_APP_MOD_T));
                p_cph_rule->frwd_bm  = frwd_bm;
                memcpy(&p_cph_rule->frwd_value, frwd_value, sizeof(CPH_APP_FRWD_T));

                spin_unlock_irqrestore(&g_cph_app_db.app_lock, flags);
                return MV_OK;
            }
        }
    }
    spin_unlock_irqrestore(&g_cph_app_db.app_lock, flags);

    return MV_OK;
}

/******************************************************************************
* cph_db_get_app_rule()
* _____________________________________________________________________________
*
* DESCRIPTION: Get application type CPH rule from data base 
*
* INPUTS:
*       parse_bm   - Parsing bitmap
*       parse_key  - Parsing key
*
* OUTPUTS:
*       mod_bm     - Modification bitmap
*       mod_value  - Modification value
*       frwd_bm    - Forwarding bitmap
*       frwd_value - Forwarding value
*
* RETURNS:
*       On success, the function returns MV_OK. 
*       On error returns error code accordingly.
*******************************************************************************/
MV_STATUS cph_db_get_app_rule (
    CPH_APP_PARSE_FIELD_E parse_bm, 
    CPH_APP_PARSE_T      *parse_key,  
    CPH_APP_MOD_FIELD_E  *mod_bm, 
    CPH_APP_MOD_T        *mod_value, 
    CPH_APP_FRWD_FIELD_E *frwd_bm, 
    CPH_APP_FRWD_T       *frwd_value)
{
    UINT32           idx         = 0;
    UINT32           rule_idx    = 0;
    CPH_APP_RULE_T  *p_cph_rule  = NULL;
    BOOL             rc          = FALSE;
    unsigned long    flags;

    spin_lock_irqsave(&g_cph_app_db.app_lock, flags);   
    for (idx = 0, rule_idx = 0; (idx < CPH_APP_MAX_RULE_NUM) && (rule_idx < g_cph_app_db.rule_num); idx++)
    {
        p_cph_rule = &g_cph_app_db.cph_rule[idx];
        
        /* Compare parse_bm and parse_key */
        if (p_cph_rule->valid == TRUE)
        {
            rule_idx++;

            rc = cph_db_compare_rule_and_packet(p_cph_rule->parse_bm, &p_cph_rule->parse_key, parse_bm, parse_key);
            if (rc == TRUE)
            {   
                if (p_cph_rule->mod_value.state == TRUE) 
                {
                    *mod_bm  = p_cph_rule->mod_bm;
                    memcpy(mod_value, &p_cph_rule->mod_value, sizeof(CPH_APP_MOD_T));
                    *frwd_bm = p_cph_rule->frwd_bm;
                    memcpy(frwd_value, &p_cph_rule->frwd_value, sizeof(CPH_APP_FRWD_T));

                    spin_unlock_irqrestore(&g_cph_app_db.app_lock, flags);
                    return MV_OK;
               }
            }
        }
    }
    spin_unlock_irqrestore(&g_cph_app_db.app_lock, flags);

    return MV_FAIL;
}

/******************************************************************************
* cph_db_get_app_rule_by_dir_proto()
* _____________________________________________________________________________
*
* DESCRIPTION: Get application type CPH rule from data base by protocol type
*
* INPUTS:
*       dir        - Direction
*       proto_type - SKB protocol type
*
* OUTPUTS:
*       parse_bm   - Parsing bitmap
*       parse_key  - Parsing key
*       mod_bm     - Modification bitmap
*       mod_value  - Modification value
*       frwd_bm    - Forwarding bitmap
*       frwd_value - Forwarding value
*
* RETURNS:
*       On success, the function returns MV_OK. 
*       On error returns error code accordingly.
*******************************************************************************/
MV_STATUS cph_db_get_app_rule_by_dir_proto (
    CPH_DIR_E              dir,
    UINT16                 proto_type,
    CPH_APP_PARSE_FIELD_E *parse_bm,
    CPH_APP_PARSE_T       *parse_key,    
    CPH_APP_MOD_FIELD_E   *mod_bm, 
    CPH_APP_MOD_T         *mod_value, 
    CPH_APP_FRWD_FIELD_E  *frwd_bm, 
    CPH_APP_FRWD_T        *frwd_value)
{
    UINT32           idx         = 0;
    UINT32           rule_idx    = 0;
    CPH_APP_RULE_T  *p_cph_rule  = NULL;
    unsigned long    flags;

    spin_lock_irqsave(&g_cph_app_db.app_lock, flags);   
    for (idx = 0, rule_idx = 0; (idx < CPH_APP_MAX_RULE_NUM) && (rule_idx < g_cph_app_db.rule_num); idx++)
    {
        p_cph_rule = &g_cph_app_db.cph_rule[idx];
        
        /* Compare parse_bm and parse_key */
        if (p_cph_rule->valid == TRUE)
        {
            rule_idx++;

            if ((p_cph_rule->mod_bm & CPH_APP_RX_MOD_REPLACE_PROTO_TYPE) &&
                (p_cph_rule->mod_value.proto_type == proto_type) &&
                (p_cph_rule->mod_value.state      == TRUE))
            {
                if ((p_cph_rule->parse_bm & CPH_APP_PARSE_FIELD_DIR) &&
                    ((p_cph_rule->parse_key.dir == CPH_DIR_NOT_CARE)||(p_cph_rule->parse_key.dir == dir)) &&
                     (p_cph_rule->parse_bm & CPH_APP_PARSE_FIELD_RX_TX) &&
                     (p_cph_rule->parse_key.rx_tx == CPH_DIR_TX))
                {
                    *parse_bm = p_cph_rule->parse_bm;
                    memcpy(parse_key, &p_cph_rule->parse_key, sizeof(CPH_APP_PARSE_T));
                    *mod_bm   = p_cph_rule->mod_bm;
                    memcpy(mod_value, &p_cph_rule->mod_value, sizeof(CPH_APP_MOD_T));
                    *frwd_bm  = p_cph_rule->frwd_bm;
                    memcpy(frwd_value, &p_cph_rule->frwd_value, sizeof(CPH_APP_FRWD_T));

                    spin_unlock_irqrestore(&g_cph_app_db.app_lock, flags);
                    return MV_OK;
                }
            }
        }
    }
    spin_unlock_irqrestore(&g_cph_app_db.app_lock, flags);

    return MV_FAIL;
}

/******************************************************************************
* cph_db_increase_counter()
* _____________________________________________________________________________
*
* DESCRIPTION: Increase RX counter
*
* INPUTS:
*       parse_bm   - Parsing bitmap
*       parse_key  - Parsing key
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK. 
*       On error returns error code accordingly.
*******************************************************************************/
MV_STATUS cph_db_increase_counter (
    CPH_APP_PARSE_FIELD_E parse_bm, 
    CPH_APP_PARSE_T      *parse_key)
{
    UINT32           idx         = 0;
    UINT32           rule_idx    = 0;
    CPH_APP_RULE_T  *p_cph_rule  = NULL;
    BOOL             rc          = MV_FAIL;
    unsigned long    flags;

    spin_lock_irqsave(&g_cph_app_db.app_lock, flags);   
    for (idx = 0, rule_idx = 0; (idx < CPH_APP_MAX_RULE_NUM) && (rule_idx < g_cph_app_db.rule_num); idx++)
    {
        p_cph_rule = &g_cph_app_db.cph_rule[idx];
        
        /* Compare parse_bm and parse_key */
        if (p_cph_rule->valid == TRUE)
        {
            rule_idx++;

            rc = cph_db_compare_rule_and_packet(p_cph_rule->parse_bm, &p_cph_rule->parse_key, parse_bm, parse_key);
            if (rc == TRUE)
            {   
                if (p_cph_rule->mod_value.state == TRUE) 
                {
                    p_cph_rule->count++;

                    spin_unlock_irqrestore(&g_cph_app_db.app_lock, flags);
                    return MV_OK;
               }
            }
        }
    }
    spin_unlock_irqrestore(&g_cph_app_db.app_lock, flags);

    return rc;
}

/******************************************************************************
* cph_db_increase_counter_by_dir_proto()
* _____________________________________________________________________________
*
* DESCRIPTION:  Increase RX counter according to protocol type
*
* INPUTS:
*       dir        - Direction
*       proto_type - SKB protocol type
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success, the function returns MV_OK. 
*       On error returns error code accordingly.
*******************************************************************************/
MV_STATUS cph_db_increase_counter_by_dir_proto (CPH_DIR_E dir,
    UINT16    proto_type)
{
    UINT32           idx         = 0;
    UINT32           rule_idx    = 0;
    CPH_APP_RULE_T  *p_cph_rule  = NULL;
    unsigned long    flags;

    spin_lock_irqsave(&g_cph_app_db.app_lock, flags);   
    for (idx = 0, rule_idx = 0; (idx < CPH_APP_MAX_RULE_NUM) && (rule_idx < g_cph_app_db.rule_num); idx++)
    {
        p_cph_rule = &g_cph_app_db.cph_rule[idx];
        
        /* Compare dir and protocol type */
        if (p_cph_rule->valid == TRUE)
        {
            rule_idx++;

            if ((p_cph_rule->mod_bm & CPH_APP_RX_MOD_REPLACE_PROTO_TYPE) &&
                (p_cph_rule->mod_value.proto_type == proto_type) &&
                (p_cph_rule->mod_value.state      == TRUE))
            {
               if ((p_cph_rule->parse_bm & CPH_APP_PARSE_FIELD_DIR) &&
                    ((p_cph_rule->parse_key.dir == CPH_DIR_NOT_CARE) || 
                     (p_cph_rule->parse_key.dir == dir)) &&
                     (p_cph_rule->parse_bm & CPH_APP_PARSE_FIELD_RX_TX) &&
                     (p_cph_rule->parse_key.rx_tx == CPH_DIR_TX))
                {
                    p_cph_rule->count++;

                    spin_unlock_irqrestore(&g_cph_app_db.app_lock, flags);
                    return MV_OK;
                }            
            }
        }
    }
    spin_unlock_irqrestore(&g_cph_app_db.app_lock, flags);

    return MV_OK;

}

/******************************************************************************
* cph_db_get_xml_param()
* _____________________________________________________________________________
*
* DESCRIPTION: Get the XML parameter
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
MV_STATUS cph_db_get_xml_param(VOID)
{
    ezxml_t     xml_head;
    ezxml_t     xml_element;
    ezxml_t     xml_attr;
    UINT32      attr_value = 0;
    BOOL        bool_value = TRUE;
    const CHAR *name       = NULL;
    const CHAR *value      = NULL;  
    INT32       rc         = MV_OK;

    /* Read ONU profile XML file */
    if ((xml_head = ezxml_parse_file(g_onu_profile_xml_cfg_file)) == 0)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s: ezxml_parse_file() of %s failed", __FUNCTION__, g_onu_profile_xml_cfg_file);
        rc = MV_NOT_FOUND;        
        goto free_exit;
    }

    if ((xml_element = ezxml_get(xml_head, XML_PROFILE_ELM_CAPABILITY, -1)) == 0)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s: ezxml_get() of %s failed\n", __FUNCTION__, XML_PROFILE_ELM_CAPABILITY);
        rc = MV_FAIL;        
        goto free_exit;

    }
    
    for (xml_attr = ezxml_child(xml_element, XML_PROFILE_ELM_ATTRIB); xml_attr; xml_attr = xml_attr->next)
    { 
        /* Parse attribute type and value */
        name = ezxml_attr(xml_attr, XML_PROFILE_ATTR_NAME);
        if (name == NULL)
        {
            MV_CPH_PRINT(CPH_ERR_LEVEL, "%s: ezxml_attr() of %s failed\n", __FUNCTION__, XML_PROFILE_ATTR_NAME);
            rc = MV_NOT_FOUND;        
            goto free_exit;
        }

        if (strcmp(name, XML_PROFILE_NAME_PROFILE) == 0)
        {
            value = ezxml_attr(xml_attr, XML_PROFILE_ATTR_VALUE);
            if (value == NULL)
            {
                MV_CPH_PRINT(CPH_ERR_LEVEL, "%s: ezxml_attr() of %s failed\n", __FUNCTION__, XML_PROFILE_ATTR_VALUE);
                rc = MV_NOT_FOUND;        
                goto free_exit;
            }
            attr_value = mtype_get_digit_num(value);

            cph_db_set_param(CPH_DB_PARAM_PROFILE_ID, &attr_value);
        } 
        else if (strcmp(name, XML_PROFILE_NAME_ACTIVE_PORT) == 0)
        {
            value = ezxml_attr(xml_attr, XML_PROFILE_ATTR_VALUE);
            if (value == NULL)
            {
                MV_CPH_PRINT(CPH_ERR_LEVEL, "%s: ezxml_attr() of %s failed\n", __FUNCTION__, XML_PROFILE_ATTR_VALUE);
                rc = MV_NOT_FOUND;        
                goto free_exit;
            }
            attr_value = mtype_get_digit_num(value);

            cph_db_set_param(CPH_DB_PARAM_ACTIVE_PORT, &attr_value);
            break;
        }
    }
    ezxml_free(xml_head);

    /* Read CPH XML file */
    if ((xml_head = ezxml_parse_file(g_cph_xml_cfg_file)) == 0)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s: ezxml_parse_file() of %s failed", __FUNCTION__, g_cph_xml_cfg_file);
        rc = MV_NOT_FOUND;        
        goto free_exit;
    }

    /* Read application support flag*/
    if ((xml_element = ezxml_get(xml_head, XML_CPH_ELM_APP_SUPPORT, -1)) == 0)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s: ezxml_get() of %s failed\n", __FUNCTION__, XML_CPH_ELM_APP_SUPPORT);
        rc = MV_FAIL;        
        goto free_exit;
    }
    if (xml_element->txt == NULL)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s: xml_element->txt == NULL \n", __FUNCTION__);
        rc = MV_FAIL;        
        goto free_exit;
    }
    attr_value = mtype_get_digit_num(xml_element->txt);
    bool_value = (attr_value != 0)? TRUE:FALSE;
    cph_db_set_param(CPH_DB_PARAM_APP_SUPPORT, &bool_value);

    /* Read IGMP support flag*/
    if ((xml_element = ezxml_get(xml_head, XML_CPH_ELM_IGMP_SUPPORT, -1)) == 0)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s: ezxml_get() of %s failed\n", __FUNCTION__, XML_CPH_ELM_IGMP_SUPPORT);
        rc = MV_FAIL;        
        goto free_exit;
    }
    if (xml_element->txt == NULL)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s: xml_element->txt == NULL \n", __FUNCTION__);
        rc = MV_FAIL;        
        goto free_exit;
    }
    attr_value = mtype_get_digit_num(xml_element->txt);
    bool_value = (attr_value != 0)? TRUE:FALSE;
    cph_db_set_param(CPH_DB_PARAM_IGMP_SUPPORT, &bool_value);

    /* Read BC support flag*/
    if ((xml_element = ezxml_get(xml_head, XML_CPH_ELM_BC_SUPPORT, -1)) == 0)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s: ezxml_get() of %s failed\n", __FUNCTION__, XML_CPH_ELM_BC_SUPPORT);
        rc = MV_FAIL;        
        goto free_exit;
    }
    if (xml_element->txt == NULL)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s: xml_element->txt == NULL \n", __FUNCTION__);
        rc = MV_FAIL;        
        goto free_exit;
    }
    attr_value = mtype_get_digit_num(xml_element->txt);
    bool_value = (attr_value != 0)? TRUE:FALSE;
    cph_db_set_param(CPH_DB_PARAM_BC_SUPPORT, &bool_value);

    /* Read Data flow mapping support flag*/
    if ((xml_element = ezxml_get(xml_head, XML_CPH_ELM_FLOW_SUPPORT, -1)) == 0)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s: ezxml_get() of %s failed\n", __FUNCTION__, XML_CPH_ELM_FLOW_SUPPORT);
        rc = MV_FAIL;        
        goto free_exit;
    }
    if (xml_element->txt == NULL)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s: xml_element->txt == NULL \n", __FUNCTION__);
        rc = MV_FAIL;        
        goto free_exit;
    }
    attr_value = mtype_get_digit_num(xml_element->txt);
    bool_value = (attr_value != 0)? TRUE:FALSE;
    cph_db_set_param(CPH_DB_PARAM_FLOW_SUPPORT, &bool_value);

    /* Read UDP port mapping support flag*/
    if ((xml_element = ezxml_get(xml_head, XML_CPH_ELM_UDP_SUPPORT, -1)) == 0)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s: ezxml_get() of %s failed\n", __FUNCTION__, XML_CPH_ELM_UDP_SUPPORT);
        rc = MV_FAIL;        
        goto free_exit;
    }
    if (xml_element->txt == NULL)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "%s: xml_element->txt == NULL \n", __FUNCTION__);
        rc = MV_FAIL;        
        goto free_exit;
    }
    attr_value = mtype_get_digit_num(xml_element->txt);
    bool_value = (attr_value != 0)? TRUE:FALSE;
    cph_db_set_param(CPH_DB_PARAM_UDP_SUPPORT, &bool_value);

free_exit:
    ezxml_free(xml_head);
    return rc;        
}

/*******************************************************************************
**
** cph_db_get_tcont_state
** ___________________________________________________________________________
**
** DESCRIPTION: The function get T-CONT state
**
** INPUTS:
**   tcont - T-CONT
**
** OUTPUTS:
**   None. 
**
** RETURNS:     
**   state - State of T-CONT, enabled or disabled.  
**
*******************************************************************************/
BOOL cph_db_get_tcont_state(UINT32 tcont)
{
    /* Check tcont */
    if (tcont >= MV_TCONT_LLID_NUM)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "tcont[%d] is illegal, should be less than [%d]\n", tcont, MV_TCONT_LLID_NUM); 
        return FALSE;
    }

    return g_cph_app_db.tcont_state[tcont];
}

/*******************************************************************************
**
** cph_db_set_tcont_state
** ___________________________________________________________________________
**
** DESCRIPTION: The function sets T-CONT state in mv_cph
**
** INPUTS:
**   tcont - T-CONT
**   state - State of T-CONT, enabled or disabled.
**
** OUTPUTS:
**   None. 
**
** RETURNS:     
**  On success, the function returns (MV_OK). On error different types are 
**  returned according to the case.  
**
*******************************************************************************/
MV_STATUS cph_db_set_tcont_state(UINT32 tcont, BOOL state)
{
    /* Check tcont */
    if (tcont >= MV_TCONT_LLID_NUM)
    {
        MV_CPH_PRINT(CPH_ERR_LEVEL, "tcont[%d] is illegal, should be less than [%d]\n", tcont, MV_TCONT_LLID_NUM); 
        return MV_FAIL;
    }

    /* Apply t-cont state to mv_cph */
    g_cph_app_db.tcont_state[tcont] = state;

    return MV_OK;
}

/******************************************************************************
* cph_db_display_parse_field()
* _____________________________________________________________________________
*
* DESCRIPTION: Display CPH rule parsing field
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
MV_STATUS cph_db_display_parse_field(
    CPH_APP_PARSE_FIELD_E parse_bm, 
    CPH_APP_PARSE_T      *parse_key)
{
    printk(KERN_INFO "Parse field(0x%x): ", parse_bm);    

    /* Print direction */
    if (parse_bm & CPH_APP_PARSE_FIELD_DIR)
    {
        printk(KERN_INFO "Dir(%s), ", cph_app_lookup_dir(parse_key->dir));
    }

    /* Print TX/RX direction */
    if (parse_bm & CPH_APP_PARSE_FIELD_RX_TX)
    {
        printk(KERN_INFO "RX/TX(%s), ", cph_app_lookup_rx_tx(parse_key->dir));
    }

    /* Print Marvell header */
    if (parse_bm & CPH_APP_PARSE_FIELD_MH)
    {
        printk(KERN_INFO "MH(0x%x), ", parse_key->mh);
    }

    /* Print Eth type */
    if (parse_bm & CPH_APP_PARSE_FIELD_ETH_TYPE)
    {
        printk(KERN_INFO "Eth type(0x%04x), ", parse_key->eth_type);
    }

    /* Print Eth subtype */
    if (parse_bm & CPH_APP_PARSE_FIELD_ETH_SUBTYPE)
    {
        printk(KERN_INFO "Eth subtype(%d), ", parse_key->eth_subtype);
    }

    /* Print IPV4 type */
    if (parse_bm & CPH_APP_PARSE_FIELD_IPV4_TYPE)
    {
        printk(KERN_INFO "IPv4 type(%d), ", parse_key->ipv4_type);
    }

    /* Print IPv6 NH1 */
    if (parse_bm & CPH_APP_PARSE_FIELD_IPV6_NH1)
    {
        printk(KERN_INFO "IPv6 NH1(%d), ", parse_key->ipv6_nh1);
    }

    /* Print IPv6 NH2 */
    if (parse_bm & CPH_APP_PARSE_FIELD_IPV6_NH2)
    {
        printk(KERN_INFO "IPv6 NH2(%d), ", parse_key->ipv6_nh2);
    }

    /* Print ICMPv6 type */
    if (parse_bm & CPH_APP_PARSE_FIELD_ICMPV6_TYPE)
    {
        printk(KERN_INFO "ICMPv6 type(%d)", parse_key->icmpv6_type);
    }

    printk(KERN_INFO "\n");

    return MV_OK;
}

/******************************************************************************
* cph_db_display_mod_field()
* _____________________________________________________________________________
*
* DESCRIPTION: Display CPH rule modification field
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
MV_STATUS cph_db_display_mod_field(
    CPH_APP_MOD_FIELD_E   mod_bm,
    CPH_APP_MOD_T        *mod_value)
{
    printk(KERN_INFO "Mod field(0x%x): ", mod_bm);    

    /* Print protocol type */
    if (mod_bm & CPH_APP_RX_MOD_REPLACE_PROTO_TYPE)
    {
        printk(KERN_INFO "Proto type(0x%x), ", mod_value->proto_type);
    }

    printk(KERN_INFO "state(%s)", (mod_value->state==TRUE)?"Enabled":"Disabled");

    printk(KERN_INFO "\n");  

    return MV_OK;
}

/******************************************************************************
* cph_db_display_frwd_field()
* _____________________________________________________________________________
*
* DESCRIPTION: Display CPH rule forwarding field
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
MV_STATUS cph_db_display_frwd_field(
    CPH_APP_FRWD_FIELD_E  frwd_bm, 
    CPH_APP_FRWD_T       *frwd_value)
{
    printk(KERN_INFO "Forward field(0x%x): ", frwd_bm);    

    /* Print target port */
    if (frwd_bm & CPH_APP_FRWD_SET_TRG_PORT)
    {
        printk(KERN_INFO "Target port(%d), ", frwd_value->trg_port);
    }

    /* Print target queue */
    if (frwd_bm & CPH_APP_FRWD_SET_TRG_QUEUE)
    {
        printk(KERN_INFO "Target queue(%d), ", frwd_value->trg_queue);
    }

    /* Print GEM port */
    if (frwd_bm & CPH_APP_FRWD_SET_GEM_PORT)
    {
        printk(KERN_INFO "Gem port(%d)", frwd_value->gem_port);
    }    


    printk(KERN_INFO "\n");  

    return MV_OK;
}


/******************************************************************************
* cph_db_display_all()
* _____________________________________________________________________________
*
* DESCRIPTION: Display all CPH rules in data base
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
MV_STATUS cph_db_display_all(VOID)
{
    CPH_APP_RULE_T *p_cph_rule = NULL;
    UINT32          idx        = 0;
    UINT32          rule_idx   = 0;

    printk(KERN_INFO "CPH Application Data Base, total rule number[%u]\n", g_cph_app_db.rule_num);
    printk(KERN_INFO "-------------------------------------------------------\n");

    printk(KERN_INFO "TPM complex profile: %s, active WAN: %s\n", 
           cph_app_lookup_profile_id(g_cph_app_db.profile_id), 
           cph_app_lookup_gmac(g_cph_app_db.active_port));

    printk(KERN_INFO "Generic application handling suppport: %s\n", 
       (g_cph_app_db.app_support == TRUE)?"Enabled":"Disabled");
    
#ifdef CONFIG_MV_CPH_IGMP_HANDLE
    printk(KERN_INFO "IGMP/MLD handling suppport: %s\n", 
           (g_cph_app_db.igmp_support == TRUE)?"Enabled":"Disabled");
#endif

#ifdef CONFIG_MV_CPH_BC_HANDLE  
    printk(KERN_INFO "Broadcast handling suppport: %s\n", 
           (g_cph_app_db.bc_support == TRUE)?"Enabled":"Disabled");
#endif

#ifdef CONFIG_MV_CPH_FLOW_MAP_HANDLE   
    printk(KERN_INFO "Data flow mapping/modification suppport: %s\n",     
           (g_cph_app_db.flow_support == TRUE)?"Enabled":"Disabled");
#endif

#ifdef CONFIG_MV_CPH_UDP_SAMPLE_HANDLE
    printk(KERN_INFO "UDP port mapping suppport: %s\n",
           (g_cph_app_db.udp_support == TRUE)?"Enabled":"Disabled");
#endif

    printk(KERN_INFO "-------------------------------------------------------\n");
    
    printk(KERN_INFO "CPH total rule number: %d\n", g_cph_app_db.rule_num);    

    for (idx = 0, rule_idx = 0; (idx < CPH_APP_MAX_RULE_NUM) && (rule_idx < g_cph_app_db.rule_num); idx++)
    {
        p_cph_rule = &g_cph_app_db.cph_rule[idx];
        if (p_cph_rule->valid == TRUE)
        {
            rule_idx++;
            
            printk(KERN_INFO "CPH rule: #%d\n", rule_idx);
            printk(KERN_INFO "-----------------------\n");
            cph_db_display_parse_field(p_cph_rule->parse_bm, &p_cph_rule->parse_key);
            cph_db_display_mod_field(p_cph_rule->mod_bm,     &p_cph_rule->mod_value);
            cph_db_display_frwd_field(p_cph_rule->frwd_bm,   &p_cph_rule->frwd_value);
            printk(KERN_INFO "Counter: %d\n\n", p_cph_rule->count);
        }
    }

    printk(KERN_INFO "Mis-matched or broadcast counter: %d\n", g_cph_app_db.bc_count);    

    printk(KERN_INFO "T-CONT State\n");
    for (idx = 0; idx < MV_TCONT_LLID_NUM; idx++)
    {
        printk(KERN_INFO "T-CONT[%d]: %s\n", idx, (g_cph_app_db.tcont_state[idx] == TRUE)?"TRUE":"FALSE");
    }

    return MV_OK;
}


/******************************************************************************
* cph_db_init()
* _____________________________________________________________________________
*
* DESCRIPTION: Initialize CPH data base 
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
MV_STATUS cph_db_init(VOID)
{
    UINT32    idx = 0;
    MV_STATUS rc  = MV_OK;

    memset(&g_cph_app_db, 0, sizeof(g_cph_app_db));
    for (idx = 0; idx < CPH_APP_MAX_RULE_NUM; idx++)
        g_cph_app_db.cph_rule[idx].valid = FALSE;

    /* Set the default value */
    g_cph_app_db.profile_id   = TPM_PON_WAN_DUAL_MAC_INT_SWITCH;
    g_cph_app_db.active_port  = MV_APP_PON_MAC_PORT;
    g_cph_app_db.app_support  = TRUE;
    g_cph_app_db.igmp_support = FALSE;
    g_cph_app_db.bc_support   = FALSE;
    g_cph_app_db.flow_support = FALSE;
    g_cph_app_db.udp_support  = FALSE;

    for (idx = 0; idx < MV_TCONT_LLID_NUM; idx++)
        g_cph_app_db.tcont_state[idx] = FALSE;

    /* Init spin lock */
    spin_lock_init(&g_cph_app_db.app_lock);

    return rc;
}
