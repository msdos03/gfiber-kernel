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
* mv_cust_flow_map.h
*
* DESCRIPTION:
*   Victor  - initial version created.   12/Dec/2011 
*
*******************************************************************************/
#ifndef __mv_cust_flow_map_h__
#define __mv_cust_flow_map_h__


/****************************************************************************** 
 *                        Data Enum and Structure                                                                                                                                  
 ******************************************************************************/
#define MVCUST_TRACE_PRINT(format, ...)   printk("%s(%d):  "format,__FUNCTION__,__LINE__, ##__VA_ARGS__)

#define MV_CUST_OK                        (0)
#define MV_CUST_FAIL                      (1)


#define MV_CUST_VID_NOT_CARE_VALUE        (4096) /* Does not care for VID    */
#define MV_CUST_PBITS_NOT_CARE_VALUE      (8)     /* Does not care for P-bits */
#define MV_CUST_DSCP_NOT_CARE_VALUE       (64)     /* Does not care for DSCP   */

#define MV_CUST_MAX_TRG_PORT_VALUE        (2+8-1)/* Maximum target port value, T-CONT7 */
#define MV_CUST_MAX_TRG_QUEUE_VALUE       (7)    /* Maximum target queue value         */
#define MV_CUST_MAX_GEM_PORT_VALUE        (4095) /* Maximum GEM port value             */

#define MV_CUST_INVALID_PBITS_TABLE_INDEX (0xff) /* Invalid Pbits table index value in VID index table*/

#define MV_CUST_DEFAULT_UNTAG_RULE        (4096+1)/* Default untagged  rule        */
#define MV_CUST_DEFAULT_SINGLE_TAG_RULE   (4096+2)/* Default sinlge tagged  rule   */
#define MV_CUST_DEFAULT_DOUBLE_UNTAG_RULE (4096+3)/* Default double tagged  rule   */

/* VLAN ID index table definition for flow mapping */
#define MV_CUST_VID_INDEX_TABLE_MAX_SIZE  (4096+4)
typedef struct {
    uint8_t pbits_map_index[MV_CUST_VID_INDEX_TABLE_MAX_SIZE];
} mv_cust_vid_index_t;

/* P-bits flow mapping table definition */
typedef uint32_t mv_cust_trg_port_type_t;
typedef uint32_t mv_cust_gem_port_key_t;
typedef struct mv_cust_pkt_frwd {
    uint32_t                in_use;
    mv_cust_trg_port_type_t trg_port;                                         
    uint32_t                trg_queue;                      
    uint32_t                trg_hwf_queue;    
    mv_cust_gem_port_key_t  gem_port;
} mv_cust_pkt_frwd_t;

#define MV_CUST_PBITS_MAP_MAX_ENTRY_NUM   (8+1)
#define MV_CUST_MAX_PBITS_MAP_TABLE_SIZE  (64)

typedef struct {
    uint32_t           in_use;
    int                mod_vid[MV_CUST_PBITS_MAP_MAX_ENTRY_NUM];
    int                mod_pbits[MV_CUST_PBITS_MAP_MAX_ENTRY_NUM];    
    mv_cust_pkt_frwd_t pkt_fwd[MV_CUST_PBITS_MAP_MAX_ENTRY_NUM];
} mv_cust_pbits_map_t;

/* DSCP to P-bits mapping table definition */
#define MV_CUST_DSCP_PBITS_TABLE_MAX_SIZE  (64)
typedef struct {
    uint32_t in_use;
    uint8_t  pbits[MV_CUST_DSCP_PBITS_TABLE_MAX_SIZE];
} mv_cust_dscp_pbits_t;
 
typedef enum
{
    MV_CUST_FLOW_DIR_US   = 0,
    MV_CUST_FLOW_DIR_DS   = 1,
    MV_CUST_FLOW_DIR_NUM  = 2
} mv_cust_flow_dir_e;

typedef struct
{
    mv_cust_flow_dir_e dir;
    int                vid;
    int                pbits;
    int                dscp;  
    int                mod_vid;
    int                mod_pbits;
    mv_cust_pkt_frwd_t pkt_frwd;
} mv_cust_ioctl_flow_map_t;

/****************************************************************************** 
 *                        Function Declaration                                                                                                                                  
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
int mv_cust_set_trace_flag(uint32_t enTrace);

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
int mv_cust_map_rule_set(mv_cust_ioctl_flow_map_t *cust_flow);

/*******************************************************************************
**
**    mv_cust_dscp_map_set
**    ___________________________________________________________________________
**
**    DESCRIPTION: The function sets GPON DSCP to P-bits mapping rules
**
**  INPUTS:
**    dscp_map   - DSCP to P-bits mapping rules.
**
**  OUTPUTS:
**    None. 
**
**    RETURNS:     
**    On success, the function returns (0). On error different types are 
**  returned according to the case.  
**
*******************************************************************************/
int mv_cust_dscp_map_set(mv_cust_dscp_pbits_t *dscp_map);

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
int mv_cust_map_rule_del(uint16_t vid, uint8_t pbits, mv_cust_flow_dir_e dir);

/*******************************************************************************
**
**    mv_cust_dscp_map_del
**    ___________________________________________________________________________
**
**    DESCRIPTION: The function deletes DSCP to P-bits mapping rules
**
**  INPUTS:
**    None.
**
**  OUTPUTS:
**    None. 
**
**    RETURNS:     
**    On success, the function returns (0). On error different types are 
**  returned according to the case.  
**
*******************************************************************************/
int mv_cust_dscp_map_del(void);

/*******************************************************************************
**
**    mv_cust_map_rule_clear
**    ___________________________________________________________________________
**
**    DESCRIPTION: The function clears all GPON flow mapping rules
**
**  INPUTS:
**    None.
**
**  OUTPUTS:
**    None. 
**
**    RETURNS:     
**    On success, the function returns (0). On error different types are 
**  returned according to the case.  
**
*******************************************************************************/
int mv_cust_map_rule_clear(void);

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
**    On success, the function returns (0). On error different types are 
**  returned according to the case.  
**
*******************************************************************************/
int mv_cust_tag_map_rule_get(mv_cust_ioctl_flow_map_t *cust_flow);

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
int mv_cust_untag_map_rule_get(mv_cust_ioctl_flow_map_t *cust_flow);

/*******************************************************************************
**
**    mv_cust_map_table_print
**    ___________________________________________________________________________
**
**    DESCRIPTION: The function displays valid GPON flow mapping tables and DSCP 
**               to P-bits mapping tablefor untagged frames.
**
**  INPUTS:
**    None.
**
**  OUTPUTS:
**    None.
**
**    RETURNS:     
**    On success, the function returns (0). On error different types are 
**  returned according to the case.  
**
*******************************************************************************/
int mv_cust_map_table_print(void);

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
**      On success, the function returns (0). On error different types are 
**    returned according to the case.  
**
*******************************************************************************/    
int mv_cust_flow_map_init(void);


#endif /* __mv_cust_flow_map_h__ */
