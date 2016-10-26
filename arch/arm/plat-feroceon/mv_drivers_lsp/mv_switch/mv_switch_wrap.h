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
*******************************************************************************/

#define SW_SRC_PORT_MIN        (1)
#define SW_SRC_PORT_MAX        (4)

#define SW_QOS_NUM_OF_QUEUES   (4)

#define SW_MAX_VLAN_NUM        (4095)
#define SW_MAX_PORTS_NUM       (7)

#define SW_PORT_NOT_BELONG     (0)
#define SW_PORT_BELONG         (1)

#define MV_TPM_DEFAULT_WEIGHT  (2)


/* Initial Q_PRI and F_PRI selection */

#define PRI_SEL_USE_PORTS_DEF    (0)
#define PRI_SEL_TAG_PRI_ONLY     (1)
#define PRI_SEL_IP_PRI_ONLY      (2)
#define PRI_SEL_TAG_AND_IP_PRI   (3)
#define PRI_SEL_TYPE_MAX         (4)

#define SW_QOS_NUM_OF_TAG_PRI    (7)
#define SW_QOS_DSCP_MAX          (64)

/* LPort data base*/
typedef struct _sw_port_info_type_t
{
    GT_DOT1Q_MODE   port_mode;
    uint32_t        cnt;
    uint32_t        vlan_blong[4095];
} sw_port_type_info;

typedef struct _sw_vlan_tbl_type
{
    uint32_t members;        /* bitmap of the ports max number of ports 32, if its value is 0, indicates that the VID do not exist in HW */
    uint8_t  egr_mode[SW_MAX_PORTS_NUM]; /* egress mode of each port */
    GT_VTU_ENTRY vtu_entry; /* Add this member to record HW VT info to SW table, descrease access time for VT table */
}sw_vlan_tbl_type;

/* auto negotiation type */
typedef enum _sw_autoneg_type_t
{
    SW_AUTONEG_1000_FDX = 1<<0,
    SW_AUTONEG_1000_HDX = 1<<1,
    SW_AUTONEG_100_FDX  = 1<<2,
    SW_AUTONEG_100_HDX  = 1<<3,
    SW_AUTONEG_10_FDX   = 1<<4,
    SW_AUTONEG_10_HDX   = 1<<5
}sw_autoneg_type_t;

/*******************************************************************************
* mv_switch_find_vid_entry_sw
*
* DESCRIPTION:
*       The API find expected VTU entry in sw_vlan_tbl.
*
* INPUTS:
*       vtuEntry      - VTU entry, supply VID
*
* OUTPUTS:
*       vtuEntry      - VTU entry, store entry info if found
*       found         - find expected entry or not
*
* RETURNS:
*       On success -  GT_OK.
*
* COMMENTS:
*
*******************************************************************************/
int32_t mv_switch_find_vid_entry_sw(GT_VTU_ENTRY *vtuEntry, GT_BOOL *found);

/*******************************************************************************
* mv_switch_record_vid_entry_sw
*
* DESCRIPTION:
*       The API store expected VTU entry in sw_vlan_tbl.
*
* INPUTS:
*       vtuEntry      - VTU entry
*
* OUTPUTS:
*       None
*
* RETURNS:
*       On success -  GT_OK.
*
* COMMENTS:
*
*******************************************************************************/
int32_t mv_switch_record_vid_entry_sw(GT_VTU_ENTRY *vtuEntry);

/*******************************************************************************
* mv_switch_prv_port_add_vid
*
* DESCRIPTION:
*       The API adds a VID to the list of the allowed VIDs per lport.
*
* INPUTS:
*       lport     - logic port id to set
*       vid       - vlan id
*       gmac0Idx  - port GMAC0 connects to, if its value is 0xFFFF, do not care GMAC0 port
*       eMode     - egress mode
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       see the example sample802_1qSetup().
*
*******************************************************************************/
int32_t mv_switch_prv_port_add_vid
(
    uint32_t lport,
    uint16_t vid,
    uint16_t gmac0Idx,
    uint8_t  eMode
);

/*******************************************************************************
* mv_switch_prv_del_vid_per_port
*
* DESCRIPTION:
*       The API delete and existing VID from the list of VIDs allowed per lport.
*
* INPUTS:
*       port     - port for deleting the VID.
*       vid      - VLAN id.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_prv_del_vid_per_port
(
    uint32_t port,
    uint16_t vid
);

/*******************************************************************************
* mv_switch_add_static_mac
*
* DESCRIPTION:
*       This function creates a static MAC entry in the MAC address table for a
*       specific lport in the integrated switch
*
* INPUTS:
*       lport		- Packet origination.
*       static_mac	- 6byte network order MAC source address.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_add_static_mac
(
	uint32_t lport,
    uint8_t  static_mac[6]
);

/*******************************************************************************
* mv_switch_del_static_mac
*
* DESCRIPTION:
*       This function removes an existing static MAC entry from the MAC address
*       table  in the integrated switch.
*
* INPUTS:
*       owner_id	- APP owner id - should be used for all API calls.
*       static_mac	- 6byte network order MAC source address.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_del_static_mac
(
    uint8_t static_mac[6]
);

/*******************************************************************************
* mv_switch_set_port_max_macs
*
* DESCRIPTION:
*       This function limits the number of MAC addresses per lport.
*
* INPUTS:
*       lport		 - Packet origination.
*       mac_per_port - maximum number of MAC addresses per port (1-255).
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       The following care is needed when enabling this feature:
*			1) disable learning on the ports
*			2) flush all non-static addresses in the ATU
*			3) define the desired limit for the ports
*			4) re-enable learing on the ports
*
*******************************************************************************/
int32_t mv_switch_set_port_max_macs
(
      uint32_t lport,
      uint8_t 	mac_per_port
);

/*******************************************************************************
* mv_switch_get_port_max_macs
*
* DESCRIPTION:
*      Port's auto learning limit. When the limit is non-zero value, the number
*		of MAC addresses that can be learned on this lport are limited to the value
*		specified in this API. When the learn limit has been reached any frame
*		that ingresses this lport with a source MAC address not already in the
*		address database that is associated with this lport will be discarded.
*		Normal auto-learning will resume on the lport as soon as the number of
*		active unicast MAC addresses associated to this lport is less than the
*		learn limit.
*		CPU directed ATU Load, Purge, or Move will not have any effect on the
*		learn limit.
*		This feature is disabled when the limit is zero.
*		The following care is needed when enabling this feature:
*			1) dsable learning on the ports
*			2) flush all non-static addresses in the ATU
*			3) define the desired limit for the ports
*			4) re-enable learing on the ports
*
* INPUTS:
*       owner_id	 - APP owner id - should be used for all API calls.
*       lport		 - Packet origination.
*
*
* OUTPUTS:
*       limit - maximum number of MAC addresses per lport (1-255).
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_get_port_max_macs
(
      uint32_t  lport,
      uint32_t *limit
);
/*******************************************************************************
* mv_switch_clear_dynamic_mac
*
* DESCRIPTION:
*       Clear all dynamic MAC.
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
int32_t mv_switch_clear_dynamic_mac
(
void
);
/*******************************************************************************
* mv_switch_set_mirror
*
* DESCRIPTION:
*       Set port mirror.
*
* INPUTS:
*       sport		 - Source port.
*       dport		 - Destination port.
*       mode		 - mirror mode.
*       enable		 - enable/disable mirror.
*
* OUTPUTS:
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
int32_t mv_switch_set_mirror
(
IN uint32_t  sport,
IN uint32_t  dport,
IN GT_MIRROR_MODE mode,
IN GT_BOOL enable
);

/*******************************************************************************
* mv_switch_get_mirror
*
* DESCRIPTION:
*       Get port mirror status.
*
* INPUTS:
*       sport		 - Source port.
*       dport		 - Destination port.
*       mode		 - mirror mode.
*
* OUTPUTS:
*       enable		 - enable/disable mirror.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
int32_t mv_switch_get_mirror
(
IN uint32_t  sport,
IN uint32_t  dport,
IN GT_MIRROR_MODE mode,
OUT GT_BOOL* enable
);

/*******************************************************************************
* mv_switch_set_mtu
*
* DESCRIPTION:
*       Set switch MTU size.
*
* INPUTS:
*       mtu		 - MTU size.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
int32_t mv_switch_set_mtu
(
IN uint32_t mtu
);

/*******************************************************************************
* mv_switch_get_mtu
*
* DESCRIPTION:
*       Get switch MTU size.
*
* INPUTS:
*
*
* OUTPUTS:
*       mtu		 - MTU size.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
int32_t mv_switch_get_mtu
(
OUT uint32_t* mtu
);
#if 0
/*******************************************************************************
* mv_switch_set_port_flooding
*
* DESCRIPTION:
*       This function control the flooding behavior (unknown Dest MAC address) per lport.
*
* INPUTS:
*       owner_id	 - APP owner id - should be used for all API calls.
*       egress_port	 - Egress lport for applying the flood settings.
*       allow_flood  - permit flooding of unknown DA: set to 1,
*                      do not permit flooding of uknown DA: set to 0.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_set_port_flooding
(
      uint32_t lport,
      GT_BOOL 	mode
);

/*******************************************************************************
* mv_switch_get_port_flooding
*
* DESCRIPTION:
*       This routine gets Forward Unknown mode of a switch port.
*		When this mode is set to GT_TRUE, normal switch operation occurs.
*		When this mode is set to GT_FALSE, unicast frame with unknown DA addresses
*		will not egress out this port.
*
* INPUTS:
*       owner_id	 - APP owner id - should be used for all API calls.
*       egress_port	 - Egress port for applying the flood settings.
*
*
* OUTPUTS:
*       mode  -  GT_TRUE: header mode enabled
*				 GT_FALSE otherwise.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_get_port_flooding
(
      uint32_t lport,
      uint32_t *mode
);
#endif
/*******************************************************************************
* mv_switch_set_port_tagged
*
* DESCRIPTION:
*       The API allows or drops tagged packets on a per lport basis.
*
* INPUTS:
*       lport		 -  lport for applying the filtering of tagged packets.
*       mode         - set to 1 = discard tagged packets per lport
*                      set to 0 = allow tagged packets per lport.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_set_port_tagged
(
      uint32_t lport,
      GT_BOOL  mode
);

/*******************************************************************************
* mv_switch_get_port_tagged
*
* DESCRIPTION:
*       This routine gets DiscardTagged bit for the given lport.
*
* INPUTS:
*       lport		 - lport for applying the filtering of tagged packets.

*
* OUTPUTS:
*      mode  - GT_TRUE if DiscardTagged bit is set, GT_FALSE otherwise
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_get_port_tagged
(
      uint32_t lport,
      uint32_t *mode
);

/*******************************************************************************
* mv_switch_set_port_untagged
*
* DESCRIPTION:
*       The API allows or drops untagged packets on a per lport basis.
*
* INPUTS:
*       lport		   -  lport for applying the filtering of tagged packets.
*       mode           - set to 1 = discard untagged packets per lport
*                        set to 0 = alow untagged packets per lport.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_set_port_untagged
(
      uint32_t lport,
      GT_BOOL 	mode
);

/*******************************************************************************
* tpm_sw_get_port_untagged
*
* DESCRIPTION:
*       This routine gets DiscardUntagged bit for the given lport.
*
* INPUTS:
*       lport		   -  lport for applying the filtering of tagged packets.

*
* OUTPUTS:
*       mode  - GT_TRUE if DiscardUntagged bit is set, GT_FALSE otherwise
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_get_port_untagged
(
      uint32_t lport,
      uint32_t *mode
);

/*******************************************************************************
* mv_switch_set_port_def_vlan
*
* DESCRIPTION:
*       The API sets port default vlan id.
*
* INPUTS:
*       lport	   - lport for applying the filtering of tagged packets.
*       vid        - the port vlan id.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_set_port_def_vlan
(
    IN uint32_t lport,
    IN GT_U16 	vid
);

/*******************************************************************************
* mv_switch_get_port_def_vlan
*
* DESCRIPTION:
*       This routine gets DiscardUntagged bit for the given lport.
*
* INPUTS:
*       lport	   - lport for applying the filtering of tagged packets.
*
* OUTPUTS:
*       vid        - the port vlan id
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_get_port_def_vlan
(
    IN  uint32_t lport,
    OUT GT_U16 	*vid
);

/*******************************************************************************
* mv_switch_set_port_def_pri
*
* DESCRIPTION:
*       The API sets port default priority.
*
* INPUTS:
*       lport	   - lport for applying the filtering of tagged packets.
*       pri        - the port priority.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_set_port_def_pri
(
    IN uint32_t lport,
    IN GT_U8 	pri
);

/*******************************************************************************
* mv_switch_get_port_def_pri
*
* DESCRIPTION:
*       The API gets port default priority.
*
* INPUTS:
*       lport	   - lport for applying the filtering of tagged packets.
*
* OUTPUTS:
*       pri        - the port priority.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_get_port_def_pri
(
    IN  uint32_t lport,
    OUT GT_U8 	*pri
);

/*******************************************************************************
* mv_switch_port_add_vid
*
* DESCRIPTION:
*       The API adds a VID to the list of the allowed VIDs per lport.
*
* INPUTS:
*       owner_id - APP owner id - should be used for all API calls.
*       lport     -  lport for adding the vid.
*       vid      - VLAN id.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       see the example sample802_1qSetup().
*
*******************************************************************************/
int32_t mv_switch_port_add_vid
(
      uint32_t lport,
      uint16_t vid,
      uint16_t gmac0Idx
);

/*******************************************************************************
* mv_switch_del_vid_per_port
*
* DESCRIPTION:
*       The API delete and existing VID from the list of VIDs allowed per lport.
*
* INPUTS:
*       owner_id - APP owner id - should be used for all API calls.
*       lport     -  lport for deleting the VID.
*       vid      - VLAN id.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_del_vid_per_port
(
      uint32_t lport,
      uint16_t vid
);

/*******************************************************************************
* mv_switch_set_port_vid_egress_mode
*
* DESCRIPTION:
*       The API sets the egress mode for a member port of a vlan.
*
* INPUTS:
*       lport     - logic port id to set
*       vid       - vlan id
*       vid       - egress mode
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       MEMBER_EGRESS_UNMODIFIED - 0
*       NOT_A_MEMBER             - 1
*       MEMBER_EGRESS_UNTAGGED   - 2
*       MEMBER_EGRESS_TAGGED     - 3
*
*******************************************************************************/
int32_t mv_switch_set_port_vid_egress_mode
(
    IN uint32_t lport,
    IN uint16_t vid,
    IN uint8_t  eMode
);

/*******************************************************************************
* mv_switch_port_print_vid
*
* DESCRIPTION:
*       The API return VID to the list of the allowed VIDs per lport.
*
* INPUTS:
*       owner_id - APP owner id - should be used for all API calls.
*       vid     -  searching VID.
*
* OUTPUTS:
*        found    - GT_TRUE, if the appropriate entry exists.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_port_print_vid
(
      uint32_t vid,
      uint32_t *found
);

/*******************************************************************************
* mv_switch_prv_set_secure_mode
*
* DESCRIPTION:
*       Change a port mode in the SW data base and remove it from all VLANs
*
* INPUTS:
*       port       -  secure port number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_prv_set_secure_mode
(
    uint32_t port
);

/*******************************************************************************
* mv_switch_prv_set_fallback_mode
*
* DESCRIPTION:
*       Change a port mode in the SW data base and add it to all VLANs
*
* INPUTS:
*       port       -  secure port number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_prv_set_fallback_mode
(
    uint32_t port
);

/*******************************************************************************
* mv_switch_set_vid_filter_per_port
*
* DESCRIPTION:
*       The API sets the filtering mode of a certain lport.
*       If the lport is in filtering mode, only the VIDs added by the
*       tpm_sw_port_add_vid API will be allowed to ingress and egress the lport.
*
* INPUTS:
*       lport       -  lport for setting the filtering mode.
*       vid_filter  - set to 1 - means the lport will DROP all packets which are NOT in
*                    the allowed VID list (built using API tpm_sw_port_add_vid).
*                    set to '0' - means that the list of VIDs allowed
*                    per lport has no significance (the list is not deleted).
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_set_vid_filter_per_port
(
      uint32_t lport,
      uint8_t  vid_filter
);

/*******************************************************************************
* mv_switch_get_vid_filter_per_port
*
* DESCRIPTION:
*       The API gets the filtering mode of a certain lport.
*       If the lport is in filtering mode, only the VIDs added by the
*       tpm_sw_port_add_vid API will be allowed to ingress and egress the lport.
*
* INPUTS:
*       lport       -  lport for setting the filtering mode.
*
* OUTPUTS:
*       vid_filter  - set to - means the lport will DROP all packets which are NOT in
*                    the allowed VID list (built using API tpm_sw_port_add_vid).
*                    set to  - means that the list of VIDs allowed
*                    per lport has no significance (the list is not deleted).
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case  see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_get_vid_filter_per_port
(
    IN  uint32_t lport,
    OUT uint8_t  *vid_filter
);

/*******************************************************************************
* mv_switch_set_port_sched_mode()
*
* DESCRIPTION:      Configures the scheduling mode per Ethernet port.
*
* INPUTS:
*       uni_port          - UNI port for setting the scheduling mode
*       sched_mode        - scheduler mode per port
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*DDD
*******************************************************************************/
int32_t mv_switch_set_port_sched_mode
(
      uint32_t            uni_port,
      GT_PORT_SCHED_MODE  mode
);

/*******************************************************************************
* mv_switch_set_uni_q_weight
*
* DESCRIPTION:
*       The API configures the weight of a queues for all
*       Ethernet UNI ports in the integrated switch.
*
* INPUTS:
*       weight   - weight value per queue (1-8).queue (value 1-3).
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.  DDD
*
*******************************************************************************/
int32_t mv_switch_set_uni_q_weight
(
      uint8_t queue_id,
      uint8_t weight
);

/*******************************************************************************
* mv_switch_set_uni_ingr_police_rate
*
* DESCRIPTION:
*       The API Configures an ingress policing function for an Ethernet UNI lport.
*
* INPUTS:
*       owner_id - APP owner id, should be used for all API calls.
*       uni_port - uni lport for configuring the ingress policer function.
*       count_mode - count mode:
*	                           GT_PIRL2_COUNT_FRAME
*	                           GT_PIRL2_COUNT_ALL_LAYER1
*	                           GT_PIRL2_COUNT_ALL_LAYER2
*	                           GT_PIRL2_COUNT_ALL_LAYER3
*       cir      - comited info rate.
*		      cbs	        - Committed Burst Size limit (expected to be 2kBytes)
*		      ebs        - Excess Burst Size limit ( 0 ~ 0xFFFFFF)

*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t mv_switch_set_uni_ingr_police_rate
(
    IN uint32_t	uni_port,
    IN GT_PIRL2_COUNT_MODE	count_mode,
    IN uint32_t cir,
    IN uint32_t cbs,
    IN uint32_t ebs
);

/*******************************************************************************
* mv_switch_get_uni_ingr_police_rate
*
* DESCRIPTION:
*       The API gets an ingress policing function for an Ethernet UNI lport.
*
* INPUTS:
*       owner_id - APP owner id, should be used for all API calls.
*       uni_port - uni lport for configuring the ingress policer function.
*
*
* OUTPUTS:
*       count_mode - count mode:
*	                           GT_PIRL2_COUNT_FRAME
*	                           GT_PIRL2_COUNT_ALL_LAYER1
*	                           GT_PIRL2_COUNT_ALL_LAYER2
*	                           GT_PIRL2_COUNT_ALL_LAYER3
*       cir      - comited info rate.
*		      cbs	        - Committed Burst Size limit (expected to be 2kBytes)
*		      ebs        - Excess Burst Size limit ( 0 ~ 0xFFFFFF)
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int32_t mv_switch_get_uni_ingr_police_rate
(
    IN  uint32_t uni_port,
    OUT GT_PIRL2_COUNT_MODE	*count_mode,
    OUT uint32_t *cir,
    OUT uint32_t *cbs,
    OUT uint32_t *ebs
);

/*******************************************************************************
* mv_switch_set_uni_tc_ingr_police_rate
*
* DESCRIPTION:
*       The API Configures a policer function for a traffic class for an Ethernet UNI lport.
*       There are 4 globally defined traffic classes in the integrated switch.
*
* INPUTS:
*       uni_port - uni lport for configuring the ingress policer function.
*       tc       - traffic class ( a combination of p-bits and DSCP values).
*       cir      - comited info rate.
*       cbs      - comited burst rate.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
int32_t mv_switch_set_uni_tc_ingr_police_rate
(
      uint32_t	uni_port,
      uint32_t	tc,
	  uint32_t cir,
	  uint32_t	cbs
);

/*******************************************************************************
* mv_switch_get_uni_tc_ingr_police_rate
*
* DESCRIPTION:
*       This routine gets the lport's ingress data limit for priority 0 or 1 or 2 or 3 frames.
*
* INPUTS:
*       owner_id - APP owner id - should be used for all API calls.
*       uni_port - uni lport for configuring the ingress policer function.
*
* OUTPUTS:
*
*       tc       - traffic class ( a combination of p-bits and DSCP values).
*       cir      - comited info rate.
*       cbs      - comited burst rate
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t mv_switch_get_uni_tc_ingr_police_rate
(
      uint32_t uni_port,
      uint32_t *tc,
	  uint32_t *cir,
      uint32_t *cbs
);

/*******************************************************************************
* mv_switch_set_uni_egr_rate_limit
*
* DESCRIPTION:
*       The API Configures the egress frame rate limit of an Ethernet UNI lport
* INPUTS:
*       owner_id            - APP owner id - should be used for all API calls.
*       trg_lport            - uni lport for configuring the egress rate limit.
*		  mode					  - frame/rate limit mode
*       frame_rate_limit_val  - egress rate limit value.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       GT_ERATE_TYPE used kbRate - frame rate valid values are:
*							        7600,..., 9600,
*							        10000, 20000, 30000, 40000, ..., 100000,
*							        110000, 120000, 130000, ..., 1000000.
*
*
*******************************************************************************/
int32_t mv_switch_set_uni_egr_rate_limit
(
IN uint32_t	trg_lport,
IN GT_PIRL_ELIMIT_MODE mode,
IN uint32_t frame_rate_limit_val
);

/*******************************************************************************
* mv_switch_get_uni_egr_rate_limit
*
* DESCRIPTION:
*       The API return the egress frame rate limit of an Ethernet UNI lport
* INPUTS:
*       owner_id        - APP owner id - should be used for all API calls.
*       trg_lport        - uni lport for configuring the egress rate limit.

*
* OUTPUTS:
*		   mode					  - frame/rate limit mode
*        rate_limit_val  - egress rate limit value..
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       GT_ERATE_TYPE used kbRate - frame rate valid values are:
*							        7600,..., 9600,
*							        10000, 20000, 30000, 40000, ..., 100000,
*							        110000, 120000, 130000, ..., 1000000.

*
*******************************************************************************/
int32_t mv_switch_get_uni_egr_rate_limit
(
     IN uint32_t trg_lport,
     OUT GT_PIRL_ELIMIT_MODE *mode,
	 OUT uint32_t *frame_rate_limit_val
);

/*******************************************************************************
* mv_switch_set_age_time
*
* DESCRIPTION:
*       This function Sets the Mac address table size.
*
* INPUTS:
*       owner_id	- APP owner id - should be used for all API calls.
*       time_out    - Aging Time value
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
int32_t mv_switch_set_age_time
(
	  uint32_t time_out
);

/*******************************************************************************
* mv_switch_set_mac_learn
*
* DESCRIPTION:
*       Enable/disable automatic learning of new source MAC addresses on port
*       ingress.
*
* INPUTS:
*       lport       - logical port number to set.
*       enable - GT_TRUE for enable  or GT_FALSE otherwise
*
* OUTPUTS:
*       None
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case, see tpm_error_code_t.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
int32_t mv_switch_set_mac_learn
(
	IN uint32_t lport,
	IN GT_BOOL  enable
);
/*******************************************************************************
* mv_switch_get_mac_learn
*
* DESCRIPTION:
*       Get automatic learning status of new source MAC addresses on port ingress.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       lport       - logical port number to set.
*       enable - GT_TRUE if enabled  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
int32_t mv_switch_get_mac_learn
(
	IN uint32_t lport,
    OUT GT_BOOL  *enable
);
/*******************************************************************************
* mv_switch_set_port_vlan_ports
*
* DESCRIPTION:
*       This routine sets the port VLAN group port membership list.
*
* INPUTS:
*       owner_id	- APP owner id - should be used for all API calls.
*       lport       - logical port number to set.
*       memPorts    - array of logical ports in the same vlan.
*       memPortsLen - number of members in memPorts array
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
int32_t mv_switch_set_port_vlan_ports
(
	  uint32_t lport,
      uint32_t memPorts[],
      uint8_t  memPortsLen
);

/*******************************************************************************
* mv_switch_get_port_vlan_ports
*
* DESCRIPTION:
*       This routine gets the port VLAN group port membership list.
*
* INPUTS:
*       owner_id	- APP owner id - should be used for all API calls.
*       lport       - logical port number to set.
*
*
* OUTPUTS:
*       memPorts    - array of logical ports in the same vlan.
*       memPortsLen - number of members in memPorts array
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
int32_t mv_switch_get_port_vlan_ports
(
	  uint32_t lport,
      uint32_t memPorts[],
      uint8_t  *memPortsLen
);

/*******************************************************************************
* mv_switch_get_age_time
*
* DESCRIPTION:
*       This function Sets the Mac address table size.
*
* INPUTS:
*       owner_id	- APP owner id - should be used for all API calls.
*
* OUTPUTS:
*       time_out    - time out value.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
int32_t mv_switch_get_age_time
(
	  uint32_t *time_out
);

/*******************************************************************************
* mv_switch_set_ingress_limit_mode
*
* DESCRIPTION:
*       This routine sets the port's rate control ingress limit mode.
*
* INPUTS:
*       owner_id	- APP owner id - should be used for all API calls.
*       lport       - logical port number to set.
*       mode 	    - rate control ingress limit mode.
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*    GT_LIMT_ALL = 0,        limit and count all frames
*    GT_LIMIT_FLOOD,         limit and count Broadcast, Multicast and flooded unicast frames
*    GT_LIMIT_BRDCST_MLTCST, limit and count Broadcast and Multicast frames
*    GT_LIMIT_BRDCST         limit and count Broadcast frames
*
*******************************************************************************/
int32_t mv_switch_set_ingress_limit_mode
(
	  uint32_t           lport,
      GT_RATE_LIMIT_MODE mode
);

/*******************************************************************************
* mv_switch_get_ingress_limit_mode
*
* DESCRIPTION:
*       This routine gets the port's rate control ingress limit mode.
*
* INPUTS:
*       owner_id	- APP owner id - should be used for all API calls.
*       lport       - logical port number to set.
*
*
* OUTPUTS:
*        mode 	    - rate control ingress limit mode.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*    GT_LIMT_ALL = 0,        limit and count all frames
*    GT_LIMIT_FLOOD,         limit and count Broadcast, Multicast and flooded unicast frames
*    GT_LIMIT_BRDCST_MLTCST, limit and count Broadcast and Multicast frames
*    GT_LIMIT_BRDCST         limit and count Broadcast frames
*
*******************************************************************************/
int32_t mv_switch_get_ingress_limit_mode
(
	  uint32_t           lport,
      GT_RATE_LIMIT_MODE *mode
);

/*******************************************************************************
* mv_switch_set_tag_pri_mapping
*
* DESCRIPTION:
*       This routine sets initial QPri and FPri selection.
*
* INPUTS:
*       lport       - Port number to set init pri.
*       init_pri    - Initial QPri and FPri selection type.
*       tag_if_both - Use Tag information for the initial QPri assignment if the frame is both
*                     tagged and its also IPv4 or IPv6 and if InitialPri uses Tag & IP priority.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       PRI_SEL_USE_PORTS_DEF  - 0
*       PRI_SEL_TAG_PRI_ONLY   - 1
*       PRI_SEL_IP_PRI_ONLY    - 2
*       PRI_SEL_TAG_AND_IP_PRI - 3
*
*******************************************************************************/
int32_t mv_switch_set_priority_selection
(
    IN  uint8_t  lport,
    IN  uint8_t  init_pri,
    IN  GT_BOOL  tag_if_both
);

/*******************************************************************************
* mv_switch_get_priority_selection
*
* DESCRIPTION:
*       This routine gets initial QPri and FPri selection.
*
* INPUTS:
*       lport       - Port number to set init pri.
*
* OUTPUTS:
*       init_pri    - Initial QPri and FPri selection type.
*       tag_if_both - Use Tag information for the initial QPri assignment if the frame is both
*                     tagged and its also IPv4 or IPv6 and if InitialPri uses Tag & IP priority.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       PRI_SEL_USE_PORTS_DEF  - 0
*       PRI_SEL_TAG_PRI_ONLY   - 1
*       PRI_SEL_IP_PRI_ONLY    - 2
*       PRI_SEL_TAG_AND_IP_PRI - 3
*
*******************************************************************************/
int32_t mv_switch_get_priority_selection
(
    IN  uint8_t  lport,
    OUT uint8_t *init_pri,
    OUT GT_BOOL *tag_if_both
);

/*******************************************************************************
* mv_switch_set_tag_pri_mapping
*
* DESCRIPTION:
*       This routine maps a tag priority to a queue priority.
*
* INPUTS:
*       tag_pri     - Source tag priority number.
*       q_pri       - Target queue priority number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       tag_pri 0-7.
*       q_pri   0-3.
*
*******************************************************************************/
int32_t mv_switch_set_tag_pri_mapping
(
    IN  uint8_t  tag_pri,
    IN  uint8_t  q_pri
);

/*******************************************************************************
* mv_switch_get_tag_pri_mapping
*
* DESCRIPTION:
*       This routine gets the queue priority for a tag priority mapping to.
*
* INPUTS:
*       tag_pri     - Source tag priority number.
*
* OUTPUTS:
*       q_pri       - Target queue priority number.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       tag_pri 0-7.
*       q_pri   0-3.
*
*******************************************************************************/
int32_t mv_switch_get_tag_pri_mapping
(
    IN  uint8_t  tag_pri,
    OUT uint8_t *q_pri
);

/*******************************************************************************
* mv_switch_set_ip_pri_mapping
*
* DESCRIPTION:
*       This routine maps a dscp value to a queue priority.
*
* INPUTS:
*       dscp        - Source dscp value.
*       q_pri       - Target queue priority number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       dscp    0-63.
*       q_pri   0-3.
*
*******************************************************************************/
int32_t mv_switch_set_ip_pri_mapping
(
    IN  uint8_t  dscp,
    IN  uint8_t  q_pri
);

/*******************************************************************************
* mv_switch_get_ip_pri_mapping
*
* DESCRIPTION:
*       This routine gets the queue priority for a dscp value mapping to.
*
* INPUTS:
*       dscp        - Source dscp value.
*
* OUTPUTS:
*       q_pri       - Target queue priority number.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       dscp    0-63.
*       q_pri   0-3.
*
*******************************************************************************/
int32_t mv_switch_get_ip_pri_mapping
(
    IN  uint8_t  dscp,
    OUT uint8_t *q_pri
);

/*******************************************************************************
* mv_switch_set_unknown_unicast_flood
*
* DESCRIPTION:
*       This routine enable/disable unknown unicast frame egress on a specific port.
*
* INPUTS:
*       lport   - Port number to egress unknown unicast frames.
*       enable  - Enable unknown unicast flooding.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t mv_switch_set_unknown_unicast_flood
(
    IN  uint8_t  lport,
    OUT GT_BOOL  enable
);

/*******************************************************************************
* mv_switch_get_unknown_unicast_flood
*
* DESCRIPTION:
*       This routine gets unknown unicast frame egress mode of a specific port.
*
* INPUTS:
*       lport   - Port number to egress unknown unicast frames.
*
* OUTPUTS:
*       enable  - Enable unknown unicast flooding.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t mv_switch_get_unknown_unicast_flood
(
    IN  uint8_t  lport,
    OUT GT_BOOL *enable
);

/*******************************************************************************
* mv_switch_set_unknown_multicast_flood
*
* DESCRIPTION:
*       This routine enable/disable unknown multicast frame egress on a specific port.
*
* INPUTS:
*       lport   - Port number to egress unknown multicast frames.
*       enable  - Enable unknown multicast flooding.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t mv_switch_set_unknown_multicast_flood
(
    IN  uint8_t  lport,
    OUT GT_BOOL  enable
);

/*******************************************************************************
* mv_switch_get_unknown_multicast_flood
*
* DESCRIPTION:
*       This routine gets unknown multicast frame egress mode of a specific port.
*
* INPUTS:
*       lport   - Port number to egress unknown multicast frames.
*
* OUTPUTS:
*       enable  - Enable unknown multicast flooding.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t mv_switch_get_unknown_multicast_flood
(
    IN  uint8_t  lport,
    OUT GT_BOOL *enable
);

/*******************************************************************************
* mv_switch_set_broadcast_flood
*
* DESCRIPTION:
*       This routine decides whether the switch always floods the broadcast
*       frames to all portsr or uses the multicast egress mode (per port).
*
* INPUTS:
*       always_on - always floods the broadcast regardless the multicast egress mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t mv_switch_set_broadcast_flood
(
    IN  GT_BOOL  always_on
);

/*******************************************************************************
* mv_switch_get_broadcast_flood
*
* DESCRIPTION:
*       This routine gets the global mode of broadcast flood.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       always_on - always floods the broadcast regardless the multicast egress mode.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t mv_switch_get_broadcast_flood
(
    IN  GT_BOOL *always_on
);

/*******************************************************************************
* mv_switch_set_marvell_header_mode
*
* DESCRIPTION:
*       This routine sets ingress and egress header mode of a switch port.
*
* INPUTS:
*       lport  - logical port number to set.
*       enable - enable marvell header.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t mv_switch_set_marvell_header_mode
(
    IN  uint8_t  lport,
    IN  GT_BOOL  enable
);

/*******************************************************************************
* mv_switch_get_marvell_header_mode
*
* DESCRIPTION:
*       This routine gets ingress and egress header mode of a switch port.
*
* INPUTS:
*       lport  - logical port number to set.
*
* OUTPUTS:
*       enable - enable marvell header.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t mv_switch_get_marvell_header_mode
(
    IN  uint8_t  lport,
    IN  GT_BOOL *enable
);

/*******************************************************************************
* mv_switch_set_port_preamble
*
* DESCRIPTION:
*       This routine sets preamble of a switch port.
*
* INPUTS:
*       lport  - logical port number to set.
*       preamble - preamble length.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t mv_switch_set_port_preamble
(
    IN  uint8_t  lport,
    IN  uint16_t preamble
);

/*******************************************************************************
* mv_switch_get_port_preamble
*
* DESCRIPTION:
*       This routine gets preamble of a switch port.
*
* INPUTS:
*       lport  - logical port number to set.
*
* OUTPUTS:
*       preamble - preamble length.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t mv_switch_get_port_preamble
(
    IN  uint8_t   lport,
    OUT uint16_t *preamble
);

/*******************************************************************************
* mv_switch_set_port_autoneg_mode
*
* DESCRIPTION:
*       The API Configures the auto negotiation state of an Ethernet  lport.
* INPUTS:
*       lport        		- logical port  number for setting the auto negotiation state.
*       autoneg_state  - autonegotiation state, enabled or disabled.
*       autoneg_mode   - enum:
*                        SPEED_AUTO_DUPLEX_AUTO: Auto for both speed and duplex
*                        SPEED_1000_DUPLEX_AUTO: 1000Mbps and auto duplex
*                        SPEED_100_DUPLEX_AUTO:  100Mbps and auto duplex
*                        SPEED_10_DUPLEX_AUTO:   10Mbps and auto duplex
*                        SPEED_AUTO_DUPLEX_FULL: Auto for speed only and Full duplex
*                        SPEED_AUTO_DUPLEX_HALF: Auto for speed only and Half duplex. (1000Mbps is not supported)
*                        SPEED_1000_DUPLEX_FULL: 1000Mbps Full duplex.
*                        SPEED_1000_DUPLEX_HALF: 1000Mbps half duplex.
*                        SPEED_100_DUPLEX_FULL:  100Mbps Full duplex.
*                        SPEED_100_DUPLEX_HALF:  100Mbps half duplex.
*                        SPEED_10_DUPLEX_FULL:   10Mbps Full duplex.
*                        SPEED_10_DUPLEX_HALF:   10Mbps half duplex.

*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case,see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
int32_t mv_switch_set_port_autoneg_mode
(
    IN uint32_t  lport,
    IN GT_BOOL           autoneg_state,
    IN GT_PHY_AUTO_MODE  autoneg_mode
);


/*******************************************************************************
* mv_switch_get_port_autoneg_mode
*
* DESCRIPTION:
*       The API return the the auto negotiation state of an Ethernet  lport.
* INPUTS:
*       lport        - logical port  number for getting the auto negotiation state.
*
*
* OUTPUTS:
*       autoneg_state  - autonegotiation state, enabled or disabled.
*       autoneg_mode   - enum:
*                        SPEED_AUTO_DUPLEX_AUTO: Auto for both speed and duplex
*                        SPEED_1000_DUPLEX_AUTO: 1000Mbps and auto duplex
*                        SPEED_100_DUPLEX_AUTO:  100Mbps and auto duplex
*                        SPEED_10_DUPLEX_AUTO:   10Mbps and auto duplex
*                        SPEED_AUTO_DUPLEX_FULL: Auto for speed only and Full duplex
*                        SPEED_AUTO_DUPLEX_HALF: Auto for speed only and Half duplex. (1000Mbps is not supported)
*                        SPEED_1000_DUPLEX_FULL: 1000Mbps Full duplex.
*                        SPEED_1000_DUPLEX_HALF: 1000Mbps half duplex.
*                        SPEED_100_DUPLEX_FULL:  100Mbps Full duplex.
*                        SPEED_100_DUPLEX_HALF:  100Mbps half duplex.
*                        SPEED_10_DUPLEX_FULL:   10Mbps Full duplex.
*                        SPEED_10_DUPLEX_HALF:   10Mbps half duplex.

*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case, see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
int32_t mv_switch_get_port_autoneg_mode
(
    IN  uint32_t lport,
    OUT GT_BOOL           *autoneg_state,
    OUT GT_PHY_AUTO_MODE  *autoneg_mode
);

/*******************************************************************************
* mv_switch_restart_port_autoneg
*
* DESCRIPTION:
*       The API restart the auto negotiation of an Ethernet  lport.
* INPUTS:
*       lport        - logical port  number for restarting the auto negotiation state.
*
*
* OUTPUTS:
*        NONE.
*								.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
int32_t mv_switch_restart_port_autoneg
(
    IN  uint32_t lport
);
/*******************************************************************************
* mv_switch_set_phy_port_state
*
* DESCRIPTION:
*       The API Configures the PHY port  state of an Ethernet  lport.
* INPUTS:
*       lport            - logical port to set.
*       phy_port_state  - PHY port  state to set.
*				     0:normal state
*				     1:power down
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
int32_t mv_switch_set_phy_port_state
(
    IN uint32_t  lport,
    IN GT_BOOL  phy_port_state
);
/*******************************************************************************
* mv_switch_get_phy_port_state
*
* DESCRIPTION:
*       The API return the PHY port  state of an Ethernet  lport.
* INPUTS:
*       lport        - logical port  number for getting the PHY port state.
*
*
* OUTPUTS:
*        phy_port_state  -  0:normal state
*				       1:power down									.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
int32_t mv_switch_get_phy_port_state
(
    IN  uint32_t lport,
    OUT GT_BOOL * phy_port_state
);
/*******************************************************************************
* mv_switch_get_phy_port_state
*
* DESCRIPTION:
*       The API return the PHY port  state of an Ethernet  lport.
* INPUTS:
*       lport        - logical port  number for getting the PHY port state.
*
*
* OUTPUTS:
*        phy_port_state  -  0:normal state
*				       1:power down									.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
int32_t mv_switch_get_port_link_status
(
    IN  uint32_t lport,
    OUT GT_BOOL * phy_port_state
);
/*******************************************************************************
* mv_switch_get_port_duplex_status
*
* DESCRIPTION:
*       The API return realtime port duplex status of an Ethernet  lport.
* INPUTS:
*       lport        - logical port  number for getting the port link status.
*
*
* OUTPUTS:
*        port_duplex_status  -  0:half deplex mode
*				               1:full deplex mode					.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case, see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
int32_t mv_switch_get_port_duplex_status
(
    IN  uint32_t lport,
    OUT GT_BOOL * port_duplex_status
);

/*******************************************************************************
* mv_switch_get_port_speed_mode
*
* DESCRIPTION:
*       The API return realtime port speed mode of an Ethernet  lport.
* INPUTS:
*       lport        - logical port  number for getting the port link status.
*
*
* OUTPUTS:
*        port_duplex_status  -  0:10M
*				               1:100M
*						 2:1000M
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case, see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
int32_t mv_switch_get_port_speed_mode
(
    IN  uint32_t lport,
    OUT GT_PORT_SPEED_MODE   *speed
);
/*******************************************************************************
* mv_switch_set_port_pause
*
* DESCRIPTION:
*       This routine will set the pause bit in Autonegotiation Advertisement
*		Register. And restart the autonegotiation.
*
* INPUTS:
*		port -	The logical port number, unless SERDES device is accessed
*				The physical address, if SERDES device is accessed
*		state - GT_PHY_PAUSE_MODE enum value.
*				GT_PHY_NO_PAUSE		- disable pause
* 				GT_PHY_PAUSE		- support pause
*				GT_PHY_ASYMMETRIC_PAUSE	- support asymmetric pause
*				GT_PHY_BOTH_PAUSE	- support both pause and asymmetric pause
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
* COMMENTS:
* data sheet register 4.10 Autonegotiation Advertisement Register
*******************************************************************************/

int32_t mv_switch_set_port_pause
(
IN GT_LPORT  port,
IN GT_PHY_PAUSE_MODE state
);
/*******************************************************************************
* mv_switch_get_port_pause
*
* DESCRIPTION:
*       This routine will get the pause bit in Autonegotiation Advertisement
*		Register.
*
* INPUTS:
*		port -	The logical port number, unless SERDES device is accessed
*				The physical address, if SERDES device is accessed
*
*
* OUTPUTS:
*		state - GT_PHY_PAUSE_MODE enum value.
*				GT_PHY_NO_PAUSE		- disable pause
* 				GT_PHY_PAUSE		- support pause
*				GT_PHY_ASYMMETRIC_PAUSE	- support asymmetric pause
*				GT_PHY_BOTH_PAUSE	- support both pause and asymmetric pause
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
* COMMENTS:
* data sheet register 4.10 Autonegotiation Advertisement Register
*******************************************************************************/

int32_t mv_switch_get_port_pause
(
IN GT_LPORT  port,
OUT  GT_PHY_PAUSE_MODE *state
);
/*******************************************************************************
* mv_switch_get_port_pause_state
*
* DESCRIPTION:
*     This routine will get the current pause state.
*		Register.
*
* INPUTS:
*		port -	The logical port number, unless SERDES device is accessed
*				The physical address, if SERDES device is accessed
*
*
* OUTPUTS:
*		state -
*				GT_FALSE: MAC Pause not implemented in the link partner or in MyPause
*				GT_TRUE:MAC Pause is implemented in the link partner and in MyPause
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case, see tpm_error_code_t.
* COMMENTS:
*       None.
*******************************************************************************/

int32_t mv_switch_get_port_pause_state
(
IN GT_LPORT  port,
OUT  GT_BOOL *state
);
/*******************************************************************************
* mv_switch_set_port_loopback
*
* DESCRIPTION:
* Enable/Disable Internal Port Loopback.
* For 10/100 Fast Ethernet PHY, speed of Loopback is determined as follows:
*   If Auto-Negotiation is enabled, this routine disables Auto-Negotiation and
*   forces speed to be 10Mbps.
*   If Auto-Negotiation is disabled, the forced speed is used.
*   Disabling Loopback simply clears bit 14 of control register(0.14). Therefore,
*   it is recommended to call gprtSetPortAutoMode for PHY configuration after
*   Loopback test.
* For 10/100/1000 Gigagbit Ethernet PHY, speed of Loopback is determined as follows:
*   If Auto-Negotiation is enabled and Link is active, the current speed is used.
*   If Auto-Negotiation is disabled, the forced speed is used.
*   All other cases, default MAC Interface speed is used. Please refer to the data
*   sheet for the information of the default MAC Interface speed.
*
*
* INPUTS:
* port - The logical port number, unless SERDES device is accessed
*        The physical address, if SERDES device is accessed
* enable - If GT_TRUE, enable loopback mode
*				 If GT_FALSE, disable loopback mode
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.14 - Loop_back
*
*******************************************************************************/

int32_t mv_switch_set_port_loopback
(
IN GT_LPORT  port,
IN GT_BOOL   enable
);
/*******************************************************************************
* mv_switch_get_port_loopback
*
* DESCRIPTION:
* Get Internal Port Loopback state.
* For 10/100 Fast Ethernet PHY, speed of Loopback is determined as follows:
*   If Auto-Negotiation is enabled, this routine disables Auto-Negotiation and
*   forces speed to be 10Mbps.
*   If Auto-Negotiation is disabled, the forced speed is used.
*   Disabling Loopback simply clears bit 14 of control register(0.14). Therefore,
*   it is recommended to call gprtSetPortAutoMode for PHY configuration after
*   Loopback test.
* For 10/100/1000 Gigagbit Ethernet PHY, speed of Loopback is determined as follows:
*   If Auto-Negotiation is enabled and Link is active, the current speed is used.
*   If Auto-Negotiation is disabled, the forced speed is used.
*   All other cases, default MAC Interface speed is used. Please refer to the data
*   sheet for the information of the default MAC Interface speed.
*
*
* INPUTS:
* port - The logical port number, unless SERDES device is accessed
*        The physical address, if SERDES device is accessed
*
* OUTPUTS:
* enable - If GT_TRUE,  loopback mode is enabled
* If GT_FALSE,  loopback mode is disabled
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.14 - Loop_back
*
*******************************************************************************/
int32_t mv_switch_get_port_loopback
(
IN GT_LPORT  port,
OUT GT_BOOL   *enable
);
/*******************************************************************************
* mv_switch_set_port_line_loopback
*
* DESCRIPTION:
* Enable/Disable Port Line Loopback.
*
*
* INPUTS:
* port - The logical port number, unless SERDES device is accessed
*        The physical address, if SERDES device is accessed
* enable - If GT_TRUE, enable loopback mode
* If GT_FALSE, disable loopback mode
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register FE:28.4, GE:21_2.14  - Loop_back
*
*******************************************************************************/

int32_t mv_switch_set_port_line_loopback
(
	IN GT_LPORT  port,
	IN GT_BOOL   enable
);

/*******************************************************************************
* mv_switch_get_port_line_loopback
*
* DESCRIPTION:
* Get Port Line Loopback status.
*
*
* INPUTS:
* port - The logical port number, unless SERDES device is accessed
*        The physical address, if SERDES device is accessed
*
* OUTPUTS:
* enable - If GT_TRUE, enable loopback mode
* If GT_FALSE, disable loopback mode* enable - If GT_TRUE, enable loopback mode
* If GT_FALSE, disable loopback mode
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register FE:28.4, GE:21_2.14  - Loop_back
*
*******************************************************************************/

int32_t mv_switch_get_port_line_loopback
(
	IN GT_LPORT  port,
	OUT GT_BOOL*   enable
);

/*******************************************************************************
* mv_switch_set_port_duplex_mode
*
* DESCRIPTION:
* 		Sets duplex mode for a specific logical port. This function will keep
*		the speed and loopback mode to the previous value, but disable others,
*		such as Autonegotiation.
*
* INPUTS:
*		port -	The logical port number, unless SERDES device is accessed
*				The physical address, if SERDES device is accessed
* 		enable	- Enable/Disable dulpex mode
*
* OUTPUTS:
* 		None.
*
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL 	- on error
*
* COMMENTS:
* 		data sheet register 0.8 - Duplex Mode
*
*******************************************************************************/
int32_t mv_switch_set_port_duplex_mode
(
IN GT_LPORT  port,
IN GT_BOOL   enable
);
/*******************************************************************************
* mv_switch_get_port_duplex_mode
*
* DESCRIPTION:
* 		Gets duplex mode for a specific logical port.
*
* INPUTS:
*		port -	The logical port number, unless SERDES device is accessed
*				The physical address, if SERDES device is accessed
*
* OUTPUTS:
*		enable	- Dulpex mode :Enabled or Disabled
*
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL 	- on error
*
* COMMENTS:
* 		data sheet register 0.8 - Duplex Mode
*
*******************************************************************************/
int32_t mv_switch_get_port_duplex_mode
(
IN GT_LPORT  port,
OUT GT_BOOL*   enable
);
/*******************************************************************************
* mv_switch_set_port_speed
*
* DESCRIPTION:
*       This routine will disable auto-negotiation and set the PHY port speed .
*
* INPUTS:
*		port -	The logical port number, unless SERDES device is accessed
*
*		speed -    PHY_SPEED_10_MBPS -10 Mbps
*				PHY_SPEED_100_MBPS -100 Mbps
* 				PHY_SPEED_1000_MBPS -100 Mbps.
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
* COMMENTS:
*
*******************************************************************************/

int32_t mv_switch_set_port_speed
(
IN GT_LPORT  port,
IN GT_PHY_SPEED speed
);
/*******************************************************************************
* mv_switch_get_port_speed
*
* DESCRIPTION:
*       This routine will get current PHY port speed .
*
* INPUTS:
*		port -	The logical port number, unless SERDES device is accessed
*
*
* OUTPUTS:
*		speed -    PHY_SPEED_10_MBPS -10 Mbps
*				PHY_SPEED_100_MBPS -100 Mbps
* 				PHY_SPEED_1000_MBPS -100 Mbps.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
* COMMENTS:
*
*******************************************************************************/

int32_t mv_switch_get_port_speed
(
IN GT_LPORT  port,
OUT GT_PHY_SPEED *speed
);

/*******************************************************************************
* mv_switch_set_port_forced_link
*
* DESCRIPTION:
*       This routine will disable auto-negotiation and set the PHY port speed .
*
* INPUTS:
*		port -	The logical port number, unless SERDES device is accessed
*
*		enable-
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
* COMMENTS:
*
*******************************************************************************/

int32_t mv_switch_set_port_forced_link
(
IN GT_LPORT  port,
IN GT_BOOL   enable
);

/*******************************************************************************
* mv_switch_get_port_forced_link
*
* DESCRIPTION:
* 		Gets duplex mode for a specific logical port.
*
* INPUTS:
*		port -	The logical port number, unless SERDES device is accessed
*				The physical address, if SERDES device is accessed
*
* OUTPUTS:
*		enable	- Forced Link mode :Enabled or Disabled
*
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL 	- on error
*
* COMMENTS:
* 		data sheet register 0.8 - Duplex Mode
*
*******************************************************************************/
int32_t mv_switch_get_port_forced_link
(
IN GT_LPORT  port,
OUT GT_BOOL*   enable
);


/*******************************************************************************
* mv_switch_set_port_link_value
*
* DESCRIPTION:
*       This routine will disable auto-negotiation and set the PHY port speed .
*
* INPUTS:
*		port -	The logical port number, unless SERDES device is accessed
*
*		enable - Forced Link Value : Enabled or Disabled
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
* COMMENTS:
*
*******************************************************************************/
int32_t mv_switch_set_port_link_value
(
IN GT_LPORT  port,
IN GT_BOOL   enable
);

/*******************************************************************************
* mv_switch_get_port_link_value
*
* DESCRIPTION:
*       This routine will disable auto-negotiation and set the PHY port speed .
*
* INPUTS:
*		port -	The logical port number, unless SERDES device is accessed
*
*		enable - Forced Link Value : Enabled or Disabled
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
* COMMENTS:
*
*******************************************************************************/

int32_t mv_switch_get_port_link_value
(
IN GT_LPORT  port,
OUT GT_BOOL*   enable
);





/*------------------------------------------------------------------------------
                            Debug functions
------------------------------------------------------------------------------*/


/*******************************************************************************
* mv_switch_clear_port_counters
*
* DESCRIPTION:
*       This function gets all counters of the given port
*
* INPUTS:
*       owner_id	- APP owner id - should be used for all API calls.
*       lport       - Port number to write the register for.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
int32_t mv_switch_clear_port_counters
(
    void
);

/*******************************************************************************
* mv_switch_set_global_reg
*
* DESCRIPTION:
*       This function sets value to the global register.
*
* INPUTS:
*       owner_id	- APP owner id - should be used for all API calls.
*       regAddr     - The register's address.
*       data        - The data to be written.
*
* OUTPUTS:
*       NONE.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
int32_t mv_switch_set_global_reg
(
	  uint8_t  regAddr,
      uint16_t data
);

/*******************************************************************************
* mv_switch_get_global_reg
*
* DESCRIPTION:
*       This function gets value from the global register.
*
* INPUTS:
*       owner_id	- APP owner id - should be used for all API calls.
*       regAddr     - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
int32_t mv_switch_get_global_reg
(
      uint8_t  regAddr,
      uint16_t *data
);

/*******************************************************************************
* mv_switch_set_port_reg
*
* DESCRIPTION:
*       This function sets value to the port register.
*
* INPUTS:
*       owner_id	- APP owner id - should be used for all API calls.
*       lport       - Port number to write the register for.
*       regAddr     - The register's address.
*       data        - The data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
int32_t mv_switch_set_port_reg
(
      uint8_t  lport,
      uint8_t  regAddr,
      uint16_t data
);

/*******************************************************************************
* mv_switch_get_port_reg
*
* DESCRIPTION:
*       This function gets value from the port register.
*
* INPUTS:
*       owner_id	- APP owner id - should be used for all API calls.
*       lport       - Port number to write the register for.
*       regAddr     - The register's address.
*
* OUTPUTS:
*       data        - The data to be written.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
int32_t mv_switch_get_port_reg
(
      uint8_t  lport,
      uint8_t  regAddr,
      uint16_t *data
);

/*******************************************************************************
* mv_switch_print_port_counters
*
* DESCRIPTION:
*       This function gets all counters of the given port
*
* INPUTS:
*       owner_id	- APP owner id - should be used for all API calls.
*       lport       - Port number to write the register for.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        Clear on read.
*
*******************************************************************************/
int32_t mv_switch_print_port_counters
(
      uint32_t lport
);

/*******************************************************************************
* mv_switch_clean_port_counters
*
* DESCRIPTION:
*       This function clean all counters of the given port
*
* INPUTS:
*       owner_id	- APP owner id, should be used for all API calls.
*       lport       - Port number to write the register for.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case, see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
int32_t mv_switch_clean_port_counters
(
   IN  uint32_t lport
);

/*******************************************************************************
* mv_switch_get_port_counters
*
* DESCRIPTION:
*       This function gets all counters of the given port
*
* INPUTS:
*       owner_id	- APP owner id - should be used for all API calls.
*       lport       - Port number to write the register for.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        Clear on read.
*
*******************************************************************************/
int32_t mv_switch_get_port_counters
(
      uint32_t              lport,
      GT_STATS_COUNTER_SET3 *statsCounterSet
);

/*******************************************************************************
* mv_switch_get_port_drop_counters
*
* DESCRIPTION:
*       This function gets the port InDiscards, InFiltered, and OutFiltered counters.
*
* INPUTS:
*       owner_id	- APP owner id - should be used for all API calls.
*       lport       - Port number to write the register for.
*
* OUTPUTS:
*       ctr         - the counters value.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        Clear on read.
*
*******************************************************************************/
int32_t mv_switch_get_port_drop_counters
(
      uint32_t              lport,
      GT_PORT_STAT2         *ctr
);

/*******************************************************************************
* mv_switch_print_fdb
*
* DESCRIPTION:
*       This function gets all FDB table.
*
* INPUTS:
*       owner_id	- APP owner id - should be used for all API calls.
*       db_num      - ATU MAC Address Database number. If multiple address
*					 databases are not being used, DBNum should be zero.
*					 If multiple address databases are being used, this value
*					 should be set to the desired address database number.
*
* OUTPUTS:
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
int32_t mv_switch_print_fdb
(
      uint16_t db_num
);
/*******************************************************************************
* mv_switch_flush_vtu
*
* DESCRIPTION:
*       Flush VTU on the Switch
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
int32_t mv_switch_flush_vtu(void);

/*******************************************************************************
* mv_switch_flush_atu
*
* DESCRIPTION:
*       Flush ATU on the Switch
*
* INPUTS:
*       flush_cmd     FLUSH all or FLUSH all dynamic
*       db_num        ATU DB Num, only 0 should be used, since there is only one ATU DB right now .
*
* OUTPUTS:
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
int32_t mv_switch_flush_atu
(
    IN  GT_FLUSH_CMD flush_cmd,
    IN  uint16_t db_num
);

/*******************************************************************************
* mv_switch_set_port_speed_duplex_mode
*
* DESCRIPTION:
*       This routine will disable auto-negotiation and set the PHY port speed and duplex mode.
*
* INPUTS:
*		port -	The logical port number, unless SERDES device is accessed
*
*		speed -    PHY_SPEED_10_MBPS -10 Mbps
*				PHY_SPEED_100_MBPS -100 Mbps
* 				PHY_SPEED_1000_MBPS -100 Mbps.
*		enable - Enable/Disable full duplex mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
* COMMENTS:
*
*******************************************************************************/
int32_t mv_switch_set_port_speed_duplex_mode
(
IN GT_LPORT port,
IN GT_PHY_SPEED speed,
IN GT_BOOL enable
);

/*******************************************************************************
* mv_switch_set_port_forced_fc_value
*
* DESCRIPTION:
*       This routine will set forced flow control value when forced flow control enabled.
*
* INPUTS:
*		port -	The logical port number, unless SERDES device is accessed
*
*		enable-
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
* COMMENTS:
*
*******************************************************************************/
int32_t mv_switch_set_port_forced_fc_value
(
IN GT_LPORT  port,
IN GT_BOOL   enable
);

/*******************************************************************************
* mv_switch_set_port_forced_flow_control
*
* DESCRIPTION:
*       This routine will enable/disable forced flow control state.
*
* INPUTS:
*		port -	The logical port number, unless SERDES device is accessed
*
*		enable-
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
* COMMENTS:
*
*******************************************************************************/
int32_t mv_switch_set_port_forced_flow_control
(
IN GT_LPORT  port,
IN GT_BOOL   enable
);

/*******************************************************************************
* mv_switch_port_add_vid_set_egrs_mode
*
* DESCRIPTION:
*       The API adds a VID to the list of the allowed VIDs per lport
*       and sets the egress mode for the port.
*
* INPUTS:
*       lport     - logic port id to set
*       vid       - vlan id
*       gmac0Idx  - port GMAC0 connects to
*       eMode     - egress mode
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case see tpm_error_code_t.
*
* COMMENTS:
*       MEMBER_EGRESS_UNMODIFIED - 0
*       NOT_A_MEMBER             - 1
*       MEMBER_EGRESS_UNTAGGED   - 2
*       MEMBER_EGRESS_TAGGED     - 3
*
*******************************************************************************/
int32_t mv_switch_port_add_vid_set_egrs_mode
(
    IN uint32_t lport,
    IN uint16_t vid,
    IN uint16_t gmac0Idx,
    IN uint8_t  eMode
);

/*******************************************************************************
* mv_switch_drv_init
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
void mv_switch_drv_init
(
    void
);

