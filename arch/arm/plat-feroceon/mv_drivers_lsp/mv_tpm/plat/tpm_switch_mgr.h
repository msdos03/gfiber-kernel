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
* mv_tpm_sw_config.h
*
*
*  MODULE : TPM Switch
*
*  DESCRIPTION : This file config tpm switch
*
*  MODIFICATION HISTORY:
*           28Apr2010   DimaM  - initial version created.
*
* FILE REVISION NUMBER:
*       $Revision: 1.27 $
*******************************************************************************/

#ifndef __INC_TPM_SW_CONFIG
#define __INC_TPM_SW_CONFIG

#define PORT_SCHED_MODE_WRRB     (0)
#define PORT_SCHED_MODE_SPRI     (1)
#define PORT_SCHED_MODE_SPRI3    (2)
#define PORT_SCHED_MODE_SPRI2_3  (3)

#define SW_QOS_NUM_OF_QUEUES     (4)

#define INT_GE_PHY_SWITCH_PORT   (0)

#define SW_TRUNK_ID_MAX          (0xf)
#define SW_TRUNK_MASK_NUM_MAX    (0x7)
#define SW_TRUNK_MASK_MAX        (0x7f)

#define SW_TRUNK_ID_REG          (5)
#define SW_TRUNK_ID_BIT_OFF      (8)
#define SW_TRUNK_BIT_OFF         (14)
#define SW_TRUNK_ID_BIT_LEN      (4)

#define SW_TRUNK_MAPPING_REG     (8)
#define SW_TRUNK_MAPPING_BIT_OFF (0)
#define SW_TRUNK_MAPPING_BIT_LEN (7)
#define SW_TRUNK_MAPPING_ID_BIT_OFF     (11)
#define SW_REG_UPDATE_BIT_OFF    (15)

#define SW_TRUNK_MASK_REG         (7)
#define SW_TRUNK_MASK_NUM_BIT_OFF (12)
#define SW_TRUNK_MASK_NUM_BIT_LEN (3)
#define SW_TRUNK_MASK_BIT_OFF     (0)
#define SW_TRUNK_MASK_BIT_LEN     (7)

#define SW_CLEAR_REG_BIT(REG, OFF, LEN) (REG) = (~((0xFFFF >> (16 - (LEN))) << OFF) & (REG))

/* enum of PHY access way */
typedef enum {
        PHY_SMI_MASTER_CPU,/*Phy directly accessed through LSP function*/
        PHY_SMI_MASTER_SWT,/*Phy accessed through switch*/
} tpm_phy_ctrl_t;

/*******************************************************************************
* tpm_sw_set_static_mac_w_ports_mask
*
* DESCRIPTION:
*       This function creates or destory a static MAC entry in the MAC address
*       table for several specific ports in the integrated switch
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       ports_mask  - Ports mask.
*       static_mac  - 6byte network order MAC source address.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success  -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_static_mac_w_ports_mask
(
    uint32_t    owner_id,
    uint32_t    ports_mask,
    uint8_t     static_mac[6]
);

/*******************************************************************************
* tpm_sw_get_port_max_macs
*
* DESCRIPTION:
*        Port's auto learning limit. When the limit is non-zero value, the number
*        of MAC addresses that can be learned on this lport are limited to the value
*        specified in this API. When the learn limit has been reached any frame
*        that ingresses this lport with a source MAC address not already in the
*        address database that is associated with this lport will be discarded.
*        Normal auto-learning will resume on the lport as soon as the number of
*        active unicast MAC addresses associated to this lport is less than the
*        learn limit.
*        CPU directed ATU Load, Purge, or Move will not have any effect on the
*        learn limit.
*        This feature is disabled when the limit is zero.
*        The following care is needed when enabling this feature:
*            1) dsable learning on the ports
*            2) flush all non-static addresses in the ATU
*            3) define the desired limit for the ports
*            4) re-enable learing on the ports
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
*
* OUTPUTS:
*       limit       - maximum number of MAC addresses per lport (1-255).
*
* RETURNS:
*       On success  - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_port_max_macs
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint32_t           *limit
);


/*******************************************************************************
* tpm_sw_port_get_vid
*
* DESCRIPTION:
*       The API return VID to the list of the allowed VIDs per lport.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       vid        -  searching VID.
*
* OUTPUTS:
*       found      - GT_TRUE, if the appropriate entry exists.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_port_get_vid
(
    uint32_t  owner_id,
    uint32_t  vid,
    uint32_t *found
);


/*******************************************************************************
* tpm_sw_get_vid_filter_per_port
*
* DESCRIPTION:
*        This routine gets protected mode of a switch lport.
*        When this mode is set to GT_TRUE, frames are allowed to egress lport
*        defined by the 802.1Q VLAN membership for the frame's VID 'AND'
*        by the lport's VLANTable if 802.1Q is enabled on the lport. Both must
*        allow the frame to Egress.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       mode       - GT_TRUE: header mode enabled
*                    GT_FALSE otherwise
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_vid_filter_per_port
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint32_t           *mode
);

/*******************************************************************************
* tpm_sw_get_uni_tc_ingr_police_rate
*
* DESCRIPTION:
*       This routine gets the lport's ingress data limit for priority 0 or 1 or 2 or 3 frames.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*
*       tc         - traffic class ( a combination of p-bits and DSCP values).
*       cir        - comited info rate.
*       cbs        - comited burst rate
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_uni_tc_ingr_police_rate
(
    uint32_t             owner_id,
    tpm_src_port_type_t  src_port,
    uint32_t            *tc,
    uint32_t            *cir,
    uint32_t            *cbs
);

/*******************************************************************************
* tpm_sw_get_uni_egr_rate_limit
*
* DESCRIPTION:
*       The API return the egress frame rate limit of an Ethernet UNI lport
* INPUTS:
*       owner_id        - APP owner id - should be used for all API calls.
*       src_port        - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       mode            - frame/rate limit mode
*       rate_limit_val  - egress rate limit value..
*
* RETURNS:
*       On success      - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       GT_ERATE_TYPE used kbRate - frame rate valid values are:
*                                    7600,..., 9600,
*                                    10000, 20000, 30000, 40000, ..., 100000,
*                                    110000, 120000, 130000, ..., 1000000.

*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_uni_egr_rate_limit
(
    uint32_t             owner_id,
    tpm_src_port_type_t  src_port,
    tpm_limit_mode_t    *mode,
    uint32_t            *frame_rate_limit_val
);

/*******************************************************************************
* tpm_sw_set_atu_size
*
* DESCRIPTION:
*       This function Sets the Mac address table size.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       size        - Table size
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success  - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        size 0(256) - 5(8192)
*                256 entries Mac address table.
*                512 entries Mac address table.
*                1024 entries Mac address table.
*                2048 entries Mac address table.
*                4096 entries Mac address table.
*                8192 entries Mac address table.
*******************************************************************************/

tpm_error_code_t tpm_sw_set_atu_size
(
    uint32_t    owner_id,
    uint32_t    size
);

/*******************************************************************************
* tpm_sw_set_port_vlan_ports
*
* DESCRIPTION:
*       This routine sets the port VLAN group port membership list.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*       memPorts    - array of logical ports in the same vlan.
*       memPortsLen - number of members in memPorts array
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success  - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_port_vlan_ports
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint32_t            memPorts[],
    uint8_t             memPortsLen
);

/*******************************************************************************
* tpm_sw_get_port_vlan_ports
*
* DESCRIPTION:
*       This routine gets the port VLAN group port membership list.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
*
* OUTPUTS:
*       memPorts    - array of logical ports in the same vlan.
*       memPortsLen - number of members in memPorts array
*
* RETURNS:
*       On success  - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_port_vlan_ports
(
    uint32_t             owner_id,
    tpm_src_port_type_t  src_port,
    uint32_t             memPorts[],
    uint8_t             *memPortsLen
);

/*******************************************************************************
* tpm_sw_set_ingress_limit_mode
*
* DESCRIPTION:
*       This routine sets the port's rate control ingress limit mode.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       src_port     - Source port in UNI port index, UNI0, UNI1...UNI4.
*       mode         - rate control ingress limit mode.
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success   - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*    GT_LIMT_ALL = 0,        limit and count all frames
*    GT_LIMIT_FLOOD,         limit and count Broadcast, Multicast and flooded unicast frames
*    GT_LIMIT_BRDCST_MLTCST, limit and count Broadcast and Multicast frames
*    GT_LIMIT_BRDCST         limit and count Broadcast frames
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_ingress_limit_mode
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint32_t            mode
);

/*******************************************************************************
* tpm_sw_get_ingress_limit_mode
*
* DESCRIPTION:
*       This routine gets the port's rate control ingress limit mode.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       src_port     - Source port in UNI port index, UNI0, UNI1...UNI4.
*
*
* OUTPUTS:
*        mode        - rate control ingress limit mode.
*
* RETURNS:
*       On success   -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*    GT_LIMT_ALL = 0,        limit and count all frames
*    GT_LIMIT_FLOOD,         limit and count Broadcast, Multicast and flooded unicast frames
*    GT_LIMIT_BRDCST_MLTCST, limit and count Broadcast and Multicast frames
*    GT_LIMIT_BRDCST         limit and count Broadcast frames
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_ingress_limit_mode
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint32_t           *mode
);

/*******************************************************************************
* tpm_sw_set_priority_selection
*
* DESCRIPTION:
*       This function sets initial QPri and FPri selection.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*       init_pri    - Initial QPri and FPri selection type.
*       tag_if_both - Use Tag information for the initial QPri assignment if the frame is both
*                     tagged and its also IPv4 or IPv6 and if InitialPri uses Tag & IP priority.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       PRI_SEL_USE_PORTS_DEF  - 0
*       PRI_SEL_TAG_PRI_ONLY   - 1
*       PRI_SEL_IP_PRI_ONLY    - 2
*       PRI_SEL_TAG_AND_IP_PRI - 3
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_priority_selection
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint8_t             init_pri,
    bool                tag_if_both
);

/*******************************************************************************
* tpm_sw_get_priority_selection
*
* DESCRIPTION:
*       This function gets initial QPri and FPri selection.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       init_pri    - Initial QPri and FPri selection type.
*       tag_if_both - Use Tag information for the initial QPri assignment if the frame is both
*                     tagged and its also IPv4 or IPv6 and if InitialPri uses Tag & IP priority.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       PRI_SEL_USE_PORTS_DEF  - 0
*       PRI_SEL_TAG_PRI_ONLY   - 1
*       PRI_SEL_IP_PRI_ONLY    - 2
*       PRI_SEL_TAG_AND_IP_PRI - 3
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_priority_selection
(
    uint32_t             owner_id,
    tpm_src_port_type_t  src_port,
    uint8_t             *init_pri,
    bool                *tag_if_both
);

/*******************************************************************************
* tpm_sw_set_tag_pri_mapping
*
* DESCRIPTION:
*       This function maps a tag priority to a queue priority.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       tag_pri     - Source tag priority number.
*       q_pri       - Target queue priority number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       tag_pri 0-7.
*       q_pri   0-3.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_tag_pri_mapping
(
     uint32_t owner_id,
     uint8_t  tag_pri,
     uint8_t  q_pri
);

/*******************************************************************************
* tpm_sw_get_tag_pri_mapping
*
* DESCRIPTION:
*       This function gets the queue priority for a tag priority mapping to.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       tag_pri    - Source tag priority number.
*
* OUTPUTS:
*       q_pri      - Target queue priority number.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       tag_pri 0-7.
*       q_pri   0-3.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_tag_pri_mapping
(
    uint32_t owner_id,
    uint8_t  tag_pri,
    uint8_t *q_pri
);

/*******************************************************************************
* tpm_sw_set_ip_pri_mapping
*
* DESCRIPTION:
*       This function maps a dscp value to a queue priority.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       dscp       - Source dscp value.
*       q_pri      - Target queue priority number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       dscp    0-63.
*       q_pri   0-3.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_ip_pri_mapping
(
     uint32_t owner_id,
     uint8_t  dscp,
     uint8_t  q_pri
);

/*******************************************************************************
* tpm_sw_get_ip_pri_mapping
*
* DESCRIPTION:
*       This function gets the queue priority for a dscp value mapping to.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       dscp       - Source dscp value.
*
* OUTPUTS:
*       q_pri      - Target queue priority number.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       dscp    0-63.
*       q_pri   0-3.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_ip_pri_mapping
(
    uint32_t owner_id,
    uint8_t  dscp,
    uint8_t *q_pri
);

/*******************************************************************************
* tpm_sw_set_broadcast_flood
*
* DESCRIPTION:
*       This function decides whether the switch always floods the broadcast
*       frames to all portsr or uses the multicast egress mode (per port).
*
* INPUTS:
*       owner_id - APP owner id - should be used for all API calls.
*       always_on - always floods the broadcast regardless the multicast egress mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_broadcast_flood
(
     uint32_t owner_id,
     bool     always_on
);

/*******************************************************************************
* tpm_sw_get_broadcast_flood
*
* DESCRIPTION:
*       This function gets the global mode of broadcast flood.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       always_on - always floods the broadcast regardless the multicast egress mode.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_broadcast_flood
(
     uint32_t owner_id,
     bool    *always_on
);

/*------------------------------------------------------------------------------
                            Debug functions
------------------------------------------------------------------------------*/

/*******************************************************************************
* tpm_sw_set_global_reg
*
* DESCRIPTION:
*       This function sets value to the global register.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       regAddr     - The register's address.
*       data        - The data to be written.
*
* OUTPUTS:
*       NONE.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_global_reg
(
    uint32_t             owner_id,
    uint8_t              regAddr,
    uint16_t             data
);

/*******************************************************************************
* tpm_sw_get_global_reg
*
* DESCRIPTION:
*       This function gets value from the global register.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       regAddr     - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_global_reg
(
     uint32_t             owner_id,
     uint8_t              regAddr,
    uint16_t             *data
);

/*******************************************************************************
* tpm_sw_set_port_reg
*
* DESCRIPTION:
*       This function sets value to the port register.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       lport       - Port number to write the register for.
*       regAddr     - The register's address.
*       data        - The data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_port_reg
(
     uint32_t            owner_id,
     uint8_t             lport,
     uint8_t             regAddr,
     uint16_t            data
);

/*******************************************************************************
* tpm_sw_get_port_reg
*
* DESCRIPTION:
*       This function gets value from the port register.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       lport       - Port number to write the register for.
*       regAddr     - The register's address.
*
* OUTPUTS:
*       data        - The data to be written.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_port_reg
(
     uint32_t            owner_id,
     uint8_t             lport,
     uint8_t             regAddr,
    uint16_t            *data
);

/*******************************************************************************
* tpm_sw_get_port_counters
*
* DESCRIPTION:
*       This function gets all counters of the given port
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       lport       - Port number to write the register for.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_print_port_counters
(
     uint32_t            owner_id,
     uint8_t             lport
);

/*******************************************************************************
* tpm_sw_clear_port_counters
*
* DESCRIPTION:
*       This function gets all counters of the given port
*
* INPUTS:
*       owner_id    - APP owner id , should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case , see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_clear_port_counters
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port
);

/*******************************************************************************
* tpm_sw_pm_1_read
*
*
* INPUTS:
*       wner_id         - APP owner id should be used for all API calls.
*       src_port        - Source port in UNI port index, UNI0, UNI1...UNI4.
*       tpm_swport_pm_3 - Holds PM data
*
* OUTPUTS:
*       PM data is supplied structure.
*
* RETURNS:
* TPM_RC_OK - on success, else error code
*
*******************************************************************************/
tpm_error_code_t tpm_sw_pm_1_read
(
     uint32_t             owner_id,
     tpm_src_port_type_t  src_port,
     tpm_swport_pm_1_t   *tpm_swport_pm_1
);

/*******************************************************************************
* tpm_sw_pm_3_read
*
*
* INPUTS:
*       owner_id        - APP owner id should be used for all API calls.
*       src_port        - Source port in UNI port index, UNI0, UNI1...UNI4.
*       tpm_swport_pm_3_all_t - Holds PM data
*
* OUTPUTS:
*       PM data is supplied structure.
*
* RETURNS:
* TPM_RC_OK - on success, else error code
*
*******************************************************************************/
tpm_error_code_t tpm_sw_pm_3_read
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    tpm_swport_pm_3_all_t  *tpm_swport_pm_3
);

/*******************************************************************************
* tpm_sw_prv_clear_port_counters
*
* DESCRIPTION:
*       This function gets all counters of the given port
*
* INPUTS:
*       owner_id    - APP owner id , should be used for all API calls.
*       lport       - Port number to write the register for.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case , see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_prv_clear_port_counters
(
    void
);

/*******************************************************************************
* tpm_sw_get_fdb
*
* DESCRIPTION:
*       This function gets all FDB table.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       db_num      - ATU MAC Address Database number. If multiple address
*                     databases are not being used, DBNum should be zero.
*                     If multiple address databases are being used, this value
*                     should be set to the desired address database number.
*
* OUTPUTS:
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_fdb
(
     uint32_t            owner_id,
     uint16_t            db_num
);
/*******************************************************************************
* tpm_sw_flush_vtu
*
* DESCRIPTION:
*       Flush VTU on the Switch
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_flush_vtu(uint32_t owner_id);

/*******************************************************************************
* tpm_sw_flush_atu
*
* DESCRIPTION:
*       Flush ATU on the Switch
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       flush_type  - FLUSH all or FLUSH all dynamic
*       db_num      - ATU DB Num, only 0 should be used, since there is only one ATU DB right now.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_flush_atu(uint32_t owner_id, tpm_flush_atu_type_t flush_type, uint16_t db_num);

/*******************************************************************************
* tpm_phy_set_port_speed_duplex_mode
*
* DESCRIPTION:
*       This routine will disable auto-negotiation and set the PHY port speed and duplex mode.
*
* INPUTS:
*       owner_id    - APP owner id  should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*       speed       -    PHY_SPEED_10_MBPS   - 10Mbps
*                        PHY_SPEED_100_MBPS  - 100Mbps
*                        PHY_SPEED_1000_MBPS - 1000Mbps.
*       enable      - Enable/Disable full dulpex mode
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_phy_set_port_speed_duplex_mode
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    tpm_phy_speed_t     speed,
    bool                enable
);

/*******************************************************************************
* tpm_sw_port_add_vid_set_egrs_mode
*
* DESCRIPTION:
*       The API adds a VID to the list of the allowed VIDs per UNI port,
*       and sets the egress mode for the port.
*
* INPUTS:
*       owner_id   - APP owner id should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*       vid        - vlan id
*       eMode      - egress mode
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case see tpm_error_code_t.
*
* COMMENTS:
*       MEMBER_EGRESS_UNMODIFIED - 0
*       NOT_A_MEMBER             - 1
*       MEMBER_EGRESS_UNTAGGED   - 2
*       MEMBER_EGRESS_TAGGED     - 3
*
*******************************************************************************/
tpm_error_code_t tpm_sw_port_add_vid_set_egrs_mode
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint16_t            vid,
    uint8_t             eMode
);

/*******************************************************************************
* tpm_sw_init
*
* DESCRIPTION:
*       This function init all parameters
*
* INPUTS:
*       NONE.
*
* OUTPUTS:
*       NONE.
*
* RETURNS:
*       NONE
*
* COMMENTS:
*        There is a temprorary function.
*
*******************************************************************************/
void tpm_sw_init
(
    void
);

/*******************************************************************************
* tpm_sw_set_debug_trace_flag
*
* DESCRIPTION:
*       This function sets TPM trace flag.
*
* INPUTS:
*       enDis    - enable or disable
*
* OUTPUTS:
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_debug_trace_flag
(
     uint32_t            enDis
);

/*******************************************************************************
* tpm_sw_pm_print_from_gmac
*
*
* INPUTS:
*       port        - GMAC port.
* OUTPUTS:
*       tpm_swport_pm_1 - Holds PM data1
*       tpm_swport_pm_3 - Holds PM data all
*
* RETURNS:
* TPM_RC_OK - on success, else error code
*
*******************************************************************************/
tpm_error_code_t tpm_sw_pm_print_from_gmac(int port);

#endif
