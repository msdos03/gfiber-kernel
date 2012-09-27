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
* mv_cust_flow_map.c
*
* DESCRIPTION:
*   Victor  - initial version created.   12/Dec/2011 
*
*******************************************************************************/
#include <mvCommon.h>

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <net/ip.h>
#include <net/ipv6.h>

#include <mvOs.h>
#include <ctrlEnv/mvCtrlEnvLib.h>

#include "mv_cust_dev.h"
#include "mv_cust_netdev.h"
#include "mv_cust_flow_map.h"
#include "mv_cust_mng_if.h"

/****************************************************************************** 
*                           Global Data Definitions                                                                                                                                  
******************************************************************************/
static mv_cust_vid_index_t  gs_vid_index_table[MV_CUST_FLOW_DIR_NUM];
static mv_cust_pbits_map_t  gs_pbits_map_table[MV_CUST_FLOW_DIR_NUM][MV_CUST_MAX_PBITS_MAP_TABLE_SIZE];
static mv_cust_dscp_pbits_t gs_dscp_map_table;

static uint32_t gs_mv_cust_trace_flag = 0;


/****************************************************************************** 
*                           External Declarations                                                                                                                                 
******************************************************************************/



/****************************************************************************** 
*                           Function Definitions                                                                                                                                  
******************************************************************************/
/*******************************************************************************
**
**    mv_cust_set_trace_flag
**    ___________________________________________________________________________
**
**    DESCRIPTION: The function sets mv_cust trace flag.
**
**    INPUTS:
**      enTrace     - Enable or disable mv_cust trace.
**
**    OUTPUTS:
**      None. 
**
**    RETURNS:     
**      On success, the function returns (MV_CUST_OK). On error different types are 
**    returned according to the case.  
**
*******************************************************************************/    
int mv_cust_set_trace_flag(uint32_t enTrace)
{
    gs_mv_cust_trace_flag = enTrace;
    
    return MV_CUST_OK;
}

/*******************************************************************************
**
**    mv_cust_valid_pbits_table_get
**    ___________________________________________________________________________
**
**    DESCRIPTION: The function gets available P-bits mapping table.
**
**    INPUTS:
**      None.
**
**    OUTPUTS:
**      None. 
**
**    RETURNS:     
**      Available P-bits mapping table index.  
**
*******************************************************************************/    
uint32_t mv_cust_valid_pbits_table_get(mv_cust_flow_dir_e dir)
{
    uint32_t table_idx = 0;

    /* Table index MV_CUST_MAX_PBITS_MAP_TABLE_SIZE is reserved for tagged default packets */
    for (table_idx=0; table_idx<MV_CUST_MAX_PBITS_MAP_TABLE_SIZE-1; table_idx++) {
    
       if (gs_pbits_map_table[dir][table_idx].in_use == 0)
           return table_idx;
    }
    
    return MV_CUST_INVALID_PBITS_TABLE_INDEX;
}

/*******************************************************************************
**
**    mv_cust_pbits_table_status_get
**    ___________________________________________________________________________
**
**    DESCRIPTION: The function gets P-bits mapping table status.
**
**    INPUTS:
**      pbits_map   - P-bits mapping table.
**
**    OUTPUTS:
**      None. 
**
**    RETURNS:     
**      0:No P-bits mapping rule exist in current table, 1: still exists P-bits mapping rule.  
**
*******************************************************************************/    
uint32_t mv_cust_pbits_table_status_get(mv_cust_pbits_map_t *pbits_map)
{
    uint32_t pbits_idx = 0;

    for (pbits_idx=0; pbits_idx<MV_CUST_PBITS_MAP_MAX_ENTRY_NUM; pbits_idx++) {
    
       if (pbits_map->pkt_fwd[pbits_idx].in_use != 0)
           return MV_CUST_OK;
    }
    
    return MV_CUST_FAIL;
}

/*******************************************************************************
**
**    mv_cust_map_rule_set
**    ___________________________________________________________________________
**
**    DESCRIPTION: The function sets GPON flow mapping rules
**
**    INPUTS:
**      cust_flow  - VLAN ID, 802.1p value, pkt_fwd information.
**
**    OUTPUTS:
**      None. 
**
**    RETURNS:     
**      On success, the function returns (MV_CUST_OK). On error different types are 
**    returned according to the case.  
**
*******************************************************************************/
int mv_cust_map_rule_set(mv_cust_ioctl_flow_map_t *cust_flow)
{
    uint8_t              *pVidEntry  = NULL;
    mv_cust_pbits_map_t  *pPbitsMap  = NULL;
    uint32_t              pbitsIndex = 0;
    uint32_t              index      = 0;
    uint32_t              vid        = 0;
    uint32_t              pbits      = 0;
    uint32_t              mod_vid    = 0;
    uint32_t              mod_pbits  = 0;   
    mv_cust_flow_dir_e    dir        = MV_CUST_FLOW_DIR_US;    
    mv_cust_pkt_frwd_t   *pkt_fwd    = NULL;

    /* Get input information: VID, P-bits... */
    if (cust_flow == NULL) {
        MVCUST_ERR_PRINT(KERN_ERR "cust_flow is NULL \n\r");
        return MV_CUST_FAIL;
    } 
    vid       =  cust_flow->vid;
    pbits     =  cust_flow->pbits;
    mod_vid   =  cust_flow->mod_vid;
    mod_pbits =  cust_flow->mod_pbits;    
    pkt_fwd   = &cust_flow->pkt_frwd;
    dir       =  cust_flow->dir;

    if (gs_mv_cust_trace_flag) {
        MVCUST_TRACE_PRINT(KERN_INFO 
                           "==ENTER==%s: vid[%d], pbits[%d], mod_vid[%d], mod_pbits[%d], T-CONT[%d], SWF queue[%d], HWF queue[%d], GEM port[%d], dir[%d]\n\r",
                            __FUNCTION__, vid, pbits, mod_vid, mod_pbits,
                           ((pkt_fwd!= NULL)? pkt_fwd->trg_port:0), 
                           ((pkt_fwd!= NULL)? pkt_fwd->trg_queue:0), 
                           ((pkt_fwd!= NULL)? pkt_fwd->trg_hwf_queue:0),                            
                           ((pkt_fwd!= NULL)? pkt_fwd->gem_port:0),
                           cust_flow->dir);
    }

    /* Check VID */
    if (vid > (MV_CUST_VID_INDEX_TABLE_MAX_SIZE - 1)) {
        MVCUST_ERR_PRINT(KERN_ERR "vid[%d] exceeds maximum value[%d] \n\r", vid, (MV_CUST_VID_INDEX_TABLE_MAX_SIZE - 1));
        return MV_CUST_FAIL;
    }
    
    /* Check P-bits */
    if (pbits > MV_CUST_PBITS_NOT_CARE_VALUE) {
        MVCUST_ERR_PRINT(KERN_ERR "P-bits[%d] exceeds maximum value[%d] \n\r", pbits, MV_CUST_PBITS_NOT_CARE_VALUE);
        return MV_CUST_FAIL;
    }

    /* Check mod VID */
    if (mod_vid > (MV_CUST_VID_INDEX_TABLE_MAX_SIZE - 1)) {
        MVCUST_ERR_PRINT(KERN_ERR "mod_vid[%d] exceeds maximum value[%d] \n\r", mod_vid, (MV_CUST_VID_INDEX_TABLE_MAX_SIZE - 1));
        return MV_CUST_FAIL;
    }
    
    /* Check mod P-bits */
    if (mod_pbits > MV_CUST_PBITS_NOT_CARE_VALUE) {
        MVCUST_ERR_PRINT(KERN_ERR "mod_pbits[%d] exceeds maximum value[%d] \n\r", mod_pbits, MV_CUST_PBITS_NOT_CARE_VALUE);
        return MV_CUST_FAIL;
    }

    /* Check dir */
    if (dir >= MV_CUST_FLOW_DIR_NUM) {
        MVCUST_ERR_PRINT(KERN_ERR "dir[%d] exceeds maximum value[%d] \n\r", dir, MV_CUST_FLOW_DIR_NUM-1);
        return MV_CUST_FAIL;
    }    

    /* Check target port/queue/GEM port */ 
    if (pkt_fwd->trg_port > MV_CUST_MAX_TRG_PORT_VALUE) {
        MVCUST_ERR_PRINT(KERN_ERR "trg_port[%d] exceeds maximum value[%d] \n\r", pkt_fwd->trg_port, MV_CUST_MAX_TRG_PORT_VALUE);
        return MV_CUST_FAIL;
    }

    if (pkt_fwd->trg_queue > MV_CUST_MAX_TRG_QUEUE_VALUE) {
        MVCUST_ERR_PRINT(KERN_ERR "SWF trg_queue[%d] exceeds maximum value[%d] \n\r", pkt_fwd->trg_queue, MV_CUST_MAX_TRG_QUEUE_VALUE);
        return MV_CUST_FAIL;
    }   

    if (pkt_fwd->trg_hwf_queue > MV_CUST_MAX_TRG_QUEUE_VALUE) {
        MVCUST_ERR_PRINT(KERN_ERR "HWF trg_queue[%d] exceeds maximum value[%d] \n\r", pkt_fwd->trg_queue, MV_CUST_MAX_TRG_QUEUE_VALUE);
        return MV_CUST_FAIL;
    }       

    if (pkt_fwd->gem_port > MV_CUST_MAX_GEM_PORT_VALUE) {
        MVCUST_ERR_PRINT(KERN_ERR "trg_queue[%d] exceeds maximum value[%d] \n\r", pkt_fwd->gem_port, MV_CUST_MAX_GEM_PORT_VALUE);
        return MV_CUST_FAIL;
    }          
 
    
    /* Find VID index entry by VID */    
    pVidEntry = &gs_vid_index_table[dir].pbits_map_index[vid];

    /* Get P-bits mapping table */
    /* If this VID index entry does not point to any P-bits mapping table,
       need to search for an available P-bits mapping table             */
    if (*pVidEntry >= MV_CUST_MAX_PBITS_MAP_TABLE_SIZE) {
        /* Reserved for default tagged rule */
        if (vid == MV_CUST_DEFAULT_SINGLE_TAG_RULE)
            pbitsIndex = MV_CUST_MAX_PBITS_MAP_TABLE_SIZE - 1;
        else
            pbitsIndex = mv_cust_valid_pbits_table_get(dir);
        
        if (pbitsIndex >= MV_CUST_MAX_PBITS_MAP_TABLE_SIZE) {
            MVCUST_ERR_PRINT(KERN_ERR " %d P-bits mapping table has used out\n\r", MV_CUST_INVALID_PBITS_TABLE_INDEX);
            return MV_CUST_FAIL;     
        }            
    }
    /* In case that the VID index already points to a P-bits mapping table,
       Need to replace the forwarding information of this P-bit mapping table */
    else {
        pbitsIndex = *pVidEntry;
    }
    pPbitsMap = &gs_pbits_map_table[dir][pbitsIndex];

    /* If legal P-bits is configured */
    if (pbits < MV_CUST_PBITS_NOT_CARE_VALUE) {
        /* In case to enable packet forwarding */
        if (pkt_fwd->in_use != 0) {
        
            /* Save forwarding information */        
            pPbitsMap->pkt_fwd[pbits].trg_port      = pkt_fwd->trg_port;
            pPbitsMap->pkt_fwd[pbits].trg_queue     = pkt_fwd->trg_queue;
            pPbitsMap->pkt_fwd[pbits].trg_hwf_queue = pkt_fwd->trg_hwf_queue;
            pPbitsMap->pkt_fwd[pbits].gem_port      = pkt_fwd->gem_port;

            /* Save mod_vid mod_pbits */
            pPbitsMap->mod_vid[pbits]           = mod_vid;
            pPbitsMap->mod_pbits[pbits]         = mod_pbits;

            /* Enable in_use flag */
            pPbitsMap->pkt_fwd[pbits].in_use    = 1;
            pPbitsMap->in_use                   = 1;

            /* Save P-bit mapping table index in VID index table */
            *pVidEntry = pbitsIndex;                
        }
        /* In case to disable packet forwarding */
        else {
            /* CLear forwarding information */        
            pPbitsMap->pkt_fwd[pbits].trg_port      = 0;
            pPbitsMap->pkt_fwd[pbits].trg_queue     = 0;
            pPbitsMap->pkt_fwd[pbits].trg_hwf_queue = 0;
            pPbitsMap->pkt_fwd[pbits].gem_port      = 0;

            /* Clear mod_vid mod_pbits */
            pPbitsMap->mod_vid[pbits]           = 0;
            pPbitsMap->mod_pbits[pbits]         = 0;          

            /* Disable in_use flag */
            pPbitsMap->pkt_fwd[pbits].in_use    = 0;
            if (mv_cust_pbits_table_status_get(pPbitsMap) != MV_CUST_OK) {
                pPbitsMap->in_use               = 0;
                *pVidEntry                      = MV_CUST_INVALID_PBITS_TABLE_INDEX;
            }
            else {
                pPbitsMap->in_use               = 1;
                *pVidEntry                      = pbitsIndex;
            }
        }
    }
    /* If does not care for P-bits, each P-bits mapping entry should be set */
    else if (pbits == MV_CUST_PBITS_NOT_CARE_VALUE) {

        index = MV_CUST_PBITS_NOT_CARE_VALUE;

        /* In case to enable packet forwarding */
        if (pkt_fwd->in_use != 0) {
            
            /* Save forwarding information */        
            pPbitsMap->pkt_fwd[index].trg_port      = pkt_fwd->trg_port;
            pPbitsMap->pkt_fwd[index].trg_queue     = pkt_fwd->trg_queue;
            pPbitsMap->pkt_fwd[pbits].trg_hwf_queue = pkt_fwd->trg_hwf_queue;
            pPbitsMap->pkt_fwd[index].gem_port      = pkt_fwd->gem_port;
            pPbitsMap->pkt_fwd[index].in_use        = 1;
            
            /* Save mod_vid mod_pbits */
            pPbitsMap->mod_vid[index]           = mod_vid;
            pPbitsMap->mod_pbits[index]         = mod_pbits;           

            /* Enable in_use flag */
            pPbitsMap->in_use = 1;

            /* Save P-bit mapping table index in VID index table */
            *pVidEntry = pbitsIndex;                
        }
        /* In case to disable packet forwarding */
        else { 
            
            /* Clear forwarding information */        
            pPbitsMap->pkt_fwd[index].trg_port      = 0;
            pPbitsMap->pkt_fwd[index].trg_queue     = 0;
            pPbitsMap->pkt_fwd[pbits].trg_hwf_queue = 0;
            pPbitsMap->pkt_fwd[index].gem_port      = 0;
            pPbitsMap->pkt_fwd[index].in_use        = 0;
            /* clear mod_vid mod_pbits */
            pPbitsMap->mod_vid[index]           = 0;
            pPbitsMap->mod_pbits[index]         = 0;

            /* Disable in_use flag */
            if (mv_cust_pbits_table_status_get(pPbitsMap) != MV_CUST_OK) {
                pPbitsMap->in_use               = 0;
                *pVidEntry                      = MV_CUST_INVALID_PBITS_TABLE_INDEX;
            }
            else {
                pPbitsMap->in_use               = 1;
                *pVidEntry                      = pbitsIndex;
            }
        }        
    }

    if (gs_mv_cust_trace_flag) {
        MVCUST_TRACE_PRINT(KERN_INFO
                           "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return MV_CUST_OK;   
}

/*******************************************************************************
**
**    mv_cust_dscp_map_set
**    ___________________________________________________________________________
**
**    DESCRIPTION: The function sets GPON DSCP to P-bits mapping rules
**
**    INPUTS:
**      dscp_map     - DSCP to P-bits mapping rules.
**
**    OUTPUTS:
**      None. 
**
**    RETURNS:     
**      On success, the function returns (MV_CUST_OK). On error different types are 
**    returned according to the case.  
**
*******************************************************************************/
int mv_cust_dscp_map_set(mv_cust_dscp_pbits_t *dscp_map)
{
    if (gs_mv_cust_trace_flag) {
        MVCUST_TRACE_PRINT(KERN_INFO 
                           "==ENTER==%s: in_use[%d]\n\r", __FUNCTION__,
                           ((dscp_map!= NULL)? dscp_map->in_use:0));
    }

    if (dscp_map == NULL) {
        MVCUST_ERR_PRINT(KERN_ERR "Input dscp_map is NULL\n\r");
        return MV_CUST_FAIL;
    }

    /* Case 1: to enable DSCP to P-bits mapping */
    if (dscp_map->in_use != 0) {
        memcpy(&gs_dscp_map_table.pbits[0], &dscp_map->pbits[0], sizeof(gs_dscp_map_table.pbits));
        gs_dscp_map_table.in_use = 1;
    
    }
    /* Case 2: to disable DSCP to P-bits mapping */
    else {
        memset((uint8_t *)&gs_dscp_map_table, 0, sizeof(gs_dscp_map_table));
        gs_dscp_map_table.in_use = 0;
    }
    
    if (gs_mv_cust_trace_flag) {
        MVCUST_TRACE_PRINT(KERN_INFO
                           "==EXIT== %s:\n\r",__FUNCTION__);
    }
    
    return MV_CUST_OK;   
}

/*******************************************************************************
**
**    mv_cust_map_rule_del
**    ___________________________________________________________________________
**
**    DESCRIPTION: The function deletes GPON flow mapping rules
**
**    INPUTS:
**      vid         - VLAN ID.
**      pbits      - 802.1p value.
**
**    OUTPUTS:
**      None 
**
**    RETURNS:     
**      On success, the function returns (MV_CUST_OK). On error different types are 
**    returned according to the case.  
**
*******************************************************************************/
int mv_cust_map_rule_del(uint16_t vid, uint8_t pbits, mv_cust_flow_dir_e dir)
{
    uint8_t              *pVidEntry  = NULL;
    mv_cust_pbits_map_t  *pPbitsMap  = NULL;

    if (gs_mv_cust_trace_flag) {
        MVCUST_TRACE_PRINT(KERN_INFO 
                           "==ENTER==: vid[%d],pbits[%d]\n\r", vid, pbits);
    }

    /* Check VID */
    if (vid > (MV_CUST_VID_INDEX_TABLE_MAX_SIZE - 1)) {
        MVCUST_ERR_PRINT(KERN_ERR "vid[%d] exceeds maximum value[%d] \n\r", vid, (MV_CUST_VID_INDEX_TABLE_MAX_SIZE - 1));
        return MV_CUST_FAIL;
    }
    
    /* Check P-bits */
    if (pbits > MV_CUST_PBITS_NOT_CARE_VALUE) {
        MVCUST_ERR_PRINT(KERN_ERR "P-bits[%d] exceeds maximum value[%d] \n\r", pbits, MV_CUST_PBITS_NOT_CARE_VALUE);
        return MV_CUST_FAIL;
    }

    /* Check dir */
    if (dir >= MV_CUST_FLOW_DIR_NUM) {
        MVCUST_ERR_PRINT(KERN_ERR "dir[%d] exceeds maximum value[%d] \n\r", dir, MV_CUST_FLOW_DIR_NUM-1);
        return MV_CUST_FAIL;
    }   

    /* Find VID index entry by VID */    
    pVidEntry = &gs_vid_index_table[dir].pbits_map_index[vid];

    if (*pVidEntry >= MV_CUST_MAX_PBITS_MAP_TABLE_SIZE) {
         MVCUST_TRACE_PRINT(KERN_INFO "%s, pVidEntry[%d], does not need to delete \n\r",__FUNCTION__, *pVidEntry);    
        return MV_CUST_OK;                      
    }

    /* Find P-bits mapping table */
    pPbitsMap = &gs_pbits_map_table[dir][*pVidEntry];

    /* Delete P-bits mapping rule */
    pPbitsMap->pkt_fwd[pbits].trg_port   = 0;
    pPbitsMap->pkt_fwd[pbits].trg_queue  = 0;
    pPbitsMap->pkt_fwd[pbits].gem_port   = 0;
    pPbitsMap->pkt_fwd[pbits].in_use     = 0;
    pPbitsMap->mod_vid[pbits]            = 0;
    pPbitsMap->mod_pbits[pbits]          = 0;          
    
    /* Disable in_use flag */
    if (mv_cust_pbits_table_status_get(pPbitsMap)!= MV_CUST_OK) {
        pPbitsMap->in_use                = 0;
        *pVidEntry                       = MV_CUST_INVALID_PBITS_TABLE_INDEX;
    }

    if (gs_mv_cust_trace_flag) {
        MVCUST_TRACE_PRINT(KERN_INFO
                           "==EXIT==:\n\r");
    }

    return MV_CUST_OK;  
}

/*******************************************************************************
**
**    mv_cust_dscp_map_del
**    ___________________________________________________________________________
**
**    DESCRIPTION: The function deletes DSCP to P-bits mapping rules
**
**    INPUTS:
**      None.
**
**    OUTPUTS:
**      None. 
**
**    RETURNS:     
**      On success, the function returns (MV_CUST_OK). On error different types are 
**    returned according to the case.  
**
*******************************************************************************/
int mv_cust_dscp_map_del(void)
{
    if (gs_mv_cust_trace_flag) {
        MVCUST_TRACE_PRINT(KERN_INFO 
                           "==ENTER==\n\r");
    }

    /* Clear DSCP to P-bits mapping */
    else {
        memset((uint8_t *)&gs_dscp_map_table, 0, sizeof(gs_dscp_map_table));
        gs_dscp_map_table.in_use = 0;
    }
    
    if (gs_mv_cust_trace_flag) {
        MVCUST_TRACE_PRINT(KERN_INFO
                           "==EXIT==\n\r");
    }
    
    return MV_CUST_OK;
}

/*******************************************************************************
**
**    mv_cust_map_rule_clear
**    ___________________________________________________________________________
**
**    DESCRIPTION: The function clears all GPON flow mapping rules
**
**    INPUTS:
**      None.
**
**    OUTPUTS:
**      None. 
**
**    RETURNS:     
**      On success, the function returns (MV_CUST_OK). On error different types are 
**    returned according to the case.  
**
*******************************************************************************/
int mv_cust_map_rule_clear(void)
{
    uint32_t pbits_index = 0;

    if (gs_mv_cust_trace_flag) {
        MVCUST_TRACE_PRINT(KERN_INFO 
                           "==ENTER==\n\r");
    }

    /* Clear VID index table              */
    memset((uint8_t *)&gs_vid_index_table, MV_CUST_INVALID_PBITS_TABLE_INDEX, sizeof(gs_vid_index_table));

    /* Clear P-bits mapping tables        */
    for (pbits_index=0; pbits_index<MV_CUST_MAX_PBITS_MAP_TABLE_SIZE; pbits_index++) {
        memset((uint8_t *)&gs_pbits_map_table[MV_CUST_FLOW_DIR_US][pbits_index], 0, sizeof(mv_cust_pbits_map_t));
        gs_pbits_map_table[MV_CUST_FLOW_DIR_US][pbits_index].in_use = 0;
        memset((uint8_t *)&gs_pbits_map_table[MV_CUST_FLOW_DIR_DS][pbits_index], 0, sizeof(mv_cust_pbits_map_t));
        gs_pbits_map_table[MV_CUST_FLOW_DIR_DS][pbits_index].in_use = 0;        
    }

    if (gs_mv_cust_trace_flag) {
        MVCUST_TRACE_PRINT(KERN_INFO
                           "==EXIT==\n\r");
    }

    return MV_CUST_OK;  
}

/*******************************************************************************
**
**    mv_cust_tag_map_rule_get
**    ___________________________________________________________________________
**
**    DESCRIPTION: The function gets GPON flow mapping rule for tagged frames.
**
**  INPUTS:
**    cust_flow  - parsing vid, pbits, dir
**
**  OUTPUTS:
**    cust_flow  - out packet forwarding information, including GEM port, T-CONT, queue.
**                 and packet modification for VID, P-bits
**
**    RETURNS:     
**      On success, the function returns (MV_CUST_OK). On error different types are 
**    returned according to the case.  
**
*******************************************************************************/
int mv_cust_tag_map_rule_get(mv_cust_ioctl_flow_map_t *cust_flow)
{
    uint8_t              *pVidEntry  = NULL;
    mv_cust_pbits_map_t  *pPbitsMap  = NULL;
    mv_cust_pkt_frwd_t   *pPktFrwd   = NULL;
    uint32_t              vid        = 0;
    uint32_t              pbits      = 0;  
    mv_cust_flow_dir_e    dir        = MV_CUST_FLOW_DIR_US;     
    mv_cust_pkt_frwd_t   *pkt_fwd    = NULL;
    uint32_t              index      = 0;    

    /* Get input parameters */
    vid     = cust_flow->vid;
    pbits   = cust_flow->pbits;
    dir     = cust_flow->dir;
    pkt_fwd = &cust_flow->pkt_frwd;
    
    if (gs_mv_cust_trace_flag) {
        MVCUST_TRACE_PRINT(KERN_INFO 
                           "==ENTER==: vid[%d], pbits[%d], dir[%d]\n\r", vid, pbits, dir);
    }

    /* Check VID */
    if (vid > (MV_CUST_VID_INDEX_TABLE_MAX_SIZE - 1)) {
        MVCUST_ERR_PRINT(KERN_ERR "vid[%d] exceeds maximum value[%d] \n\r", vid, (MV_CUST_VID_INDEX_TABLE_MAX_SIZE - 1));
        return MV_CUST_FAIL;
    }
    
    /* Check P-bits */
    if (pbits > MV_CUST_PBITS_NOT_CARE_VALUE) {
        MVCUST_ERR_PRINT(KERN_ERR "P-bits[%d] exceeds maximum value[%d] \n\r", pbits, MV_CUST_PBITS_NOT_CARE_VALUE);
        return MV_CUST_FAIL;
    }

    /* Check dir */
    if (dir >= MV_CUST_FLOW_DIR_NUM) {
        MVCUST_ERR_PRINT(KERN_ERR "dir[%d] exceeds maximum value[%d] \n\r", dir, MV_CUST_FLOW_DIR_NUM-1);
        return MV_CUST_FAIL;
    }    

    /* Set default values */
    cust_flow->mod_vid   = MV_CUST_VID_NOT_CARE_VALUE;
    cust_flow->mod_pbits = MV_CUST_PBITS_NOT_CARE_VALUE; 
    
    /* Find VID index entry by VID */    
    pVidEntry = &gs_vid_index_table[dir].pbits_map_index[vid];

    if (*pVidEntry >= MV_CUST_MAX_PBITS_MAP_TABLE_SIZE) {
         //MVCUST_ERR_PRINT(KERN_ERR "%s, pVidEntry[%d], No matched P-bits mapping table \n\r",__FUNCTION__, *pVidEntry);    

        pkt_fwd->in_use = 0;
        if (gs_mv_cust_trace_flag) {
            MVCUST_TRACE_PRINT(KERN_INFO
                               "==EXIT==\n\r");
        }

        return MV_CUST_FAIL;                      
    }

    /* Find P-bits mapping table */
    pPbitsMap = &gs_pbits_map_table[dir][*pVidEntry];    

    /* Get packet forwarding information */
    index = pbits;

    pPktFrwd = &pPbitsMap->pkt_fwd[index];

    /* If specific flow mapping rule exists */
    if (pPktFrwd->in_use != 0) { 
        pkt_fwd->trg_port      = pPktFrwd->trg_port;
        pkt_fwd->trg_queue     = pPktFrwd->trg_queue;
        pkt_fwd->trg_hwf_queue = pPktFrwd->trg_hwf_queue;
        pkt_fwd->gem_port      = pPktFrwd->gem_port;
        cust_flow->mod_vid     = pPbitsMap->mod_vid[index];
        cust_flow->mod_pbits   = pPbitsMap->mod_pbits[index];       
        pkt_fwd->in_use        = 1;
        
        if (gs_mv_cust_trace_flag) {
            MVCUST_TRACE_PRINT(KERN_INFO
                               "trg_port(%d), trg_queue(%d) trg_hwf_queue(%d) gem_port(%d), mod_vid(%d), mod_pbits(%d)\n\r",
                                cust_flow->pkt_frwd.trg_port, cust_flow->pkt_frwd.trg_queue, cust_flow->pkt_frwd.trg_hwf_queue, 
                                cust_flow->pkt_frwd.gem_port, cust_flow->mod_vid, cust_flow->mod_pbits);
        }
        
    }
    /* If specific flow mapping rule does not exist, look for default rule */
    else {
        index = MV_CUST_PBITS_NOT_CARE_VALUE;
        pPktFrwd = &pPbitsMap->pkt_fwd[index];

        /* If default flow mapping rule exists */
        if (pPktFrwd->in_use != 0) { 
            pkt_fwd->trg_port      = pPktFrwd->trg_port;
            pkt_fwd->trg_queue     = pPktFrwd->trg_queue;
            pkt_fwd->trg_hwf_queue = pPktFrwd->trg_hwf_queue;
            pkt_fwd->gem_port      = pPktFrwd->gem_port;
            cust_flow->mod_vid     = pPbitsMap->mod_vid[index];
            cust_flow->mod_pbits   = pPbitsMap->mod_pbits[index];       
            pkt_fwd->in_use        = 1;
            
            if (gs_mv_cust_trace_flag) {
                MVCUST_TRACE_PRINT(KERN_INFO
                                   "trg_port(%d), trg_queue(%d) trg_hwf_queue(%d) gem_port(%d), mod_vid(%d), mod_pbits(%d)\n\r",
                                    cust_flow->pkt_frwd.trg_port, cust_flow->pkt_frwd.trg_queue, cust_flow->pkt_frwd.trg_hwf_queue, 
                                    cust_flow->pkt_frwd.gem_port, cust_flow->mod_vid, cust_flow->mod_pbits);
            }
            
        }
        else {
        
            pkt_fwd->in_use      = 0;
            if (gs_mv_cust_trace_flag) {
                MVCUST_TRACE_PRINT(KERN_INFO
                                   "==EXIT==\n\r");
            }        
            return MV_CUST_FAIL;
        }
    }

    if (gs_mv_cust_trace_flag) {
        MVCUST_TRACE_PRINT(KERN_INFO
                           "==EXIT==\n\r");
    }

    return MV_CUST_OK;
}

/*******************************************************************************
**
**    mv_cust_untag_map_rule_get
**    ___________________________________________________________________________
**
**    DESCRIPTION: The function gets GPON flow mapping rule for untagged frames.
**
**    INPUTS:
**      dscp         - DSCP value.
**
**    OUTPUTS:
**      cust_flow    - packet forwarding information, including GEM port, T-CONT, queue.
**
**    RETURNS:     
**      On success, the function returns (MV_CUST_OK). On error different types are 
**    returned according to the case.  
**
*******************************************************************************/
int mv_cust_untag_map_rule_get(mv_cust_ioctl_flow_map_t *cust_flow)
{
    uint8_t              *pVidEntry  = NULL;
    mv_cust_pbits_map_t  *pPbitsMap  = NULL;
    mv_cust_pkt_frwd_t   *pPktFrwd   = NULL;
    uint32_t              pbitsIndex = MV_CUST_PBITS_NOT_CARE_VALUE;
    mv_cust_flow_dir_e    dir        = MV_CUST_FLOW_DIR_US;
    uint32_t              dscp       = 0;

    if (gs_mv_cust_trace_flag) {
        MVCUST_TRACE_PRINT(KERN_INFO 
                           "==ENTER==: dscp[%d] \n\r", dscp);
    }

    /* Check target port/queue/GEM port */
    if (cust_flow == NULL) {
        MVCUST_ERR_PRINT(KERN_ERR "cust_flow is NULL \n\r");
        return MV_CUST_FAIL;
    }

    /* Set forwarding flag to false at first */
    cust_flow->pkt_frwd.in_use = 0;
    cust_flow->mod_vid         = MV_CUST_VID_NOT_CARE_VALUE;
    cust_flow->mod_pbits       = MV_CUST_PBITS_NOT_CARE_VALUE; 

    dir  = cust_flow->dir;  
    dscp = cust_flow->dscp;  

    /* Check DSCP */
    if (dscp < MV_CUST_DSCP_PBITS_TABLE_MAX_SIZE) {
        if (gs_dscp_map_table.in_use != 0) {
            pbitsIndex = gs_dscp_map_table.pbits[dscp];
        }
    }

    if(pbitsIndex > MV_CUST_PBITS_NOT_CARE_VALUE){
        //MVCUST_ERR_PRINT(KERN_ERR "pbitsIndex[%d] is illegal \n\r", pbitsIndex);
        return MV_CUST_FAIL;
    }

    /* Find P-bits mapping table */
    pVidEntry = &gs_vid_index_table[dir].pbits_map_index[MV_CUST_DEFAULT_UNTAG_RULE];

    if (*pVidEntry >= MV_CUST_MAX_PBITS_MAP_TABLE_SIZE) {
        //MVCUST_TRACE_PRINT(KERN_INFO "%s, pVidEntry[%d], does not exist \n\r",__FUNCTION__, *pVidEntry);                 
    }
    else {
        
        pPbitsMap = &gs_pbits_map_table[dir][*pVidEntry];

        if (pPbitsMap->in_use != 0) {

            pPktFrwd  = &pPbitsMap->pkt_fwd[pbitsIndex];
            if (pPktFrwd->in_use != 0) {
                cust_flow->pkt_frwd.trg_port      = pPktFrwd->trg_port;
                cust_flow->pkt_frwd.trg_queue     = pPktFrwd->trg_queue;
                cust_flow->pkt_frwd.trg_hwf_queue = pPktFrwd->trg_hwf_queue;
                cust_flow->pkt_frwd.gem_port      = pPktFrwd->gem_port;
                cust_flow->mod_vid                = pPbitsMap->mod_vid[pbitsIndex];
                cust_flow->mod_pbits              = pPbitsMap->mod_pbits[pbitsIndex];
                cust_flow->pkt_frwd.in_use        = 1;
                
                if (gs_mv_cust_trace_flag) {
                    MVCUST_TRACE_PRINT(KERN_INFO
                                       "trg_port(%d), trg_queue(%d), trg_hwf_queue(%d), gem_port(%d), mod_vid(%d), mod_pbits(%d)\n\r",
                                        cust_flow->pkt_frwd.trg_port, cust_flow->pkt_frwd.trg_queue, cust_flow->pkt_frwd.trg_hwf_queue,
                                        cust_flow->pkt_frwd.gem_port, cust_flow->mod_vid, cust_flow->mod_pbits);
                    MVCUST_TRACE_PRINT(KERN_INFO
                                       "==EXIT==:\n\r");                                            
                }
                
                return MV_CUST_OK;
            }             
        }          
    }

    if (gs_mv_cust_trace_flag) {
        MVCUST_TRACE_PRINT(KERN_INFO
                           "==EXIT==:\n\r");
    }

    return MV_CUST_FAIL;
}

/*******************************************************************************
**
**    mv_cust_map_table_print
**    ___________________________________________________________________________
**
**    DESCRIPTION: The function displays valid GPON flow mapping tables and DSCP 
**                 to P-bits mapping tablefor untagged frames.
**
**    INPUTS:
**      None.
**
**    OUTPUTS:
**      None.
**
**    RETURNS:     
**      On success, the function returns (MV_CUST_OK). On error different types are 
**    returned according to the case.  
**
*******************************************************************************/
int mv_cust_map_table_print(void)
{
    uint32_t  index = 0;
    uint32_t  table_index = 0;

    //MVCUST_TRACE_PRINT(KERN_INFO "==ENTER==\n\r");

    /* Print Valid VID index entries */
    printk(KERN_INFO "In Upstream Direction      \n----------------------------------\n"); 
    printk(KERN_INFO "MV_CUST VLAN ID Index Table\n----------------------------------\n");         
    printk(KERN_INFO "VID   P-bits_table_index\n");
    for (index=0; index<MV_CUST_VID_INDEX_TABLE_MAX_SIZE; index++) {
        if(gs_vid_index_table[MV_CUST_FLOW_DIR_US].pbits_map_index[index] < MV_CUST_MAX_PBITS_MAP_TABLE_SIZE)
            printk(KERN_INFO "%4.4d   %d\n", index, gs_vid_index_table[MV_CUST_FLOW_DIR_US].pbits_map_index[index]);    
    }
    printk(KERN_INFO "\n\n");

    /* Print P-bits mapping tables */
    printk(KERN_INFO "MV_CUST P-bits Flow Mapping Tables\n----------------------------------\n\n");  
    for (table_index=0; table_index<MV_CUST_MAX_PBITS_MAP_TABLE_SIZE; table_index++) {
        if(gs_pbits_map_table[MV_CUST_FLOW_DIR_US][table_index].in_use != 0) {
                printk(KERN_INFO "P-bits Flow Mapping Table %d\n----------------------------\n", table_index); 
                printk(KERN_INFO "P-bits  in_use  mod_vid mod_pbits trg_port  trg_queue  trg_hwf_queue  gem_port\n"); 
                for (index=0; index<MV_CUST_PBITS_MAP_MAX_ENTRY_NUM; index++) 
                    printk(KERN_INFO "%1.1d       %3.3s     %4.4d    %1.1d          %2.2d        %2.2d         %2.2d            %4.4d\n", 
                                       index,
                                       (gs_pbits_map_table[MV_CUST_FLOW_DIR_US][table_index].pkt_fwd[index].in_use!=0)? "YES":"",
                                       gs_pbits_map_table[MV_CUST_FLOW_DIR_US][table_index].mod_vid[index],
                                       gs_pbits_map_table[MV_CUST_FLOW_DIR_US][table_index].mod_pbits[index],
                                       gs_pbits_map_table[MV_CUST_FLOW_DIR_US][table_index].pkt_fwd[index].trg_port, 
                                       gs_pbits_map_table[MV_CUST_FLOW_DIR_US][table_index].pkt_fwd[index].trg_queue,
                                       gs_pbits_map_table[MV_CUST_FLOW_DIR_US][table_index].pkt_fwd[index].trg_hwf_queue,
                                       gs_pbits_map_table[MV_CUST_FLOW_DIR_US][table_index].pkt_fwd[index].gem_port);    
        
        }
    }
    printk(KERN_INFO "\n\n");

    printk(KERN_INFO "In Downstream Direction    \n----------------------------------\n"); 
    printk(KERN_INFO "MV_CUST VLAN ID Index Table\n----------------------------------\n");         
    printk(KERN_INFO "VID   P-bits_table_index\n");
    for (index=0; index<MV_CUST_VID_INDEX_TABLE_MAX_SIZE; index++) {
        if(gs_vid_index_table[MV_CUST_FLOW_DIR_DS].pbits_map_index[index] < MV_CUST_MAX_PBITS_MAP_TABLE_SIZE)
            printk(KERN_INFO "%4.4d   %d\n", index, gs_vid_index_table[MV_CUST_FLOW_DIR_DS].pbits_map_index[index]);    
    }
    printk(KERN_INFO "\n\n");

    /* Print P-bits mapping tables */
    printk(KERN_INFO "MV_CUST P-bits Flow Mapping Tables\n----------------------------------\n\n");  
    for (table_index=0; table_index<MV_CUST_MAX_PBITS_MAP_TABLE_SIZE; table_index++) {
        if(gs_pbits_map_table[MV_CUST_FLOW_DIR_DS][table_index].in_use != 0) {
                printk(KERN_INFO "P-bits Flow Mapping Table %d\n----------------------------\n", table_index); 
                printk(KERN_INFO "P-bits  in_use  mod_vid mod_pbits trg_port  trg_queue  trg_hwf_queue  gem_port\n"); 
                for (index=0; index<MV_CUST_PBITS_MAP_MAX_ENTRY_NUM; index++) 
                    printk(KERN_INFO "%1.1d       %3.3s     %4.4d    %1.1d        %2.2d        %2.2d         %2.2d              %4.4d\n", 
                                       index,
                                       (gs_pbits_map_table[MV_CUST_FLOW_DIR_DS][table_index].pkt_fwd[index].in_use!=0)? "YES":"",
                                       gs_pbits_map_table[MV_CUST_FLOW_DIR_DS][table_index].mod_vid[index],
                                       gs_pbits_map_table[MV_CUST_FLOW_DIR_DS][table_index].mod_pbits[index],
                                       gs_pbits_map_table[MV_CUST_FLOW_DIR_DS][table_index].pkt_fwd[index].trg_port, 
                                       gs_pbits_map_table[MV_CUST_FLOW_DIR_DS][table_index].pkt_fwd[index].trg_queue,
                                       gs_pbits_map_table[MV_CUST_FLOW_DIR_US][table_index].pkt_fwd[index].trg_hwf_queue,
                                       gs_pbits_map_table[MV_CUST_FLOW_DIR_DS][table_index].pkt_fwd[index].gem_port);    
        
        }
    }
    printk(KERN_INFO "\n\n");    

    /* Print DSCP to P-bits mapping table */
    printk(KERN_INFO "MV_CUST DSCP to P-bits Mapping Table\n----------------------------------\n");  
    if (gs_dscp_map_table.in_use == 0) {
        printk(KERN_INFO "No DSCP to P-bits mapping\n");
    }
    else {
        
        printk(KERN_INFO "DSCP  Pbits\n"); 
        for (index=0; index<MV_CUST_PBITS_MAP_MAX_ENTRY_NUM; index++)
            printk(KERN_INFO "%2.2d     %1.1d\n", 
                   index, gs_dscp_map_table.pbits[index]);    
    }
    printk(KERN_INFO "\n\n");

    //MVCUST_TRACE_PRINT(KERN_INFO "==EXIT==\n\r");    
    return MV_CUST_OK; 
}

/*******************************************************************************
**
**    mv_cust_flow_map_init
**    ___________________________________________________________________________
**
**    DESCRIPTION: The function initializes mv_cust flow mapping data structure.
**
**    INPUTS:
**      None.
**
**    OUTPUTS:
**      None. 
**
**    RETURNS:     
**      On success, the function returns (MV_CUST_OK). On error different types are 
**    returned according to the case.  
**
*******************************************************************************/    
int mv_cust_flow_map_init(void)
{
    uint32_t pbits_index = 0;
    
    /* Initializes VID index table              */
    memset((uint8_t *)&gs_vid_index_table, MV_CUST_INVALID_PBITS_TABLE_INDEX, sizeof(gs_vid_index_table));

    /* Initializes P-bits mapping tables        */
    for (pbits_index=0; pbits_index<MV_CUST_MAX_PBITS_MAP_TABLE_SIZE; pbits_index++) {
        memset((uint8_t *)&gs_pbits_map_table[MV_CUST_FLOW_DIR_US][pbits_index], 0, sizeof(mv_cust_pbits_map_t));
        gs_pbits_map_table[MV_CUST_FLOW_DIR_US][pbits_index].in_use = 0;
        memset((uint8_t *)&gs_pbits_map_table[MV_CUST_FLOW_DIR_DS][pbits_index], 0, sizeof(mv_cust_pbits_map_t));
        gs_pbits_map_table[MV_CUST_FLOW_DIR_DS][pbits_index].in_use = 0;        
    }

    /* Initializes DSCP to P-bits mapping table */
    memset((uint8_t *)&gs_dscp_map_table, 0, sizeof(mv_cust_dscp_pbits_t));
    gs_dscp_map_table.in_use = 0;
    
    return MV_CUST_OK;
}
