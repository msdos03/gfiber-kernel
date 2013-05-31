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

#include "mvCommon.h"
#include <linux/etherdevice.h>
#include <gtHwCntl.h>
#include "mvOs.h"
#include "mvSysHwConfig.h"
#include "eth-phy/mvEthPhy.h"
#include "boardEnv/mvBoardEnvLib.h"
#include "ctrlEnv/mvCtrlEthCompLib.h"

#include "msApi.h"
#include "msApiPrototype.h"
#include "mv_switch.h"
#include "mv_switch_wrap.h"

static sw_port_type_info   sw_ports_tbl[SW_MAX_PORTS_NUM];
static sw_vlan_tbl_type    sw_vlan_tbl[SW_MAX_VLAN_NUM];

static uint8_t   weight_arr[4] = {0};
extern GT_QD_DEV *qd_dev;

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
int32_t mv_switch_find_vid_entry_sw(GT_VTU_ENTRY *vtuEntry, GT_BOOL *found)
{
    if (vtuEntry == NULL || found == NULL)
        return GT_BAD_PARAM;

    if (sw_vlan_tbl[vtuEntry->vid].members) {/* if members not 0, this vid entry exist in HW andd SW */
        memcpy(vtuEntry, &sw_vlan_tbl[vtuEntry->vid].vtu_entry, sizeof(GT_VTU_ENTRY));
        *found = GT_TRUE;
    } else {
        *found = GT_FALSE;
    }

    return GT_OK;
}

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
int32_t mv_switch_record_vid_entry_sw(GT_VTU_ENTRY *vtuEntry)
{
    if (vtuEntry == NULL)
        return GT_BAD_PARAM;
    memcpy(&sw_vlan_tbl[vtuEntry->vid].vtu_entry, vtuEntry, sizeof(GT_VTU_ENTRY));

    return GT_OK;
}

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
)
{
    GT_STATUS    rc = GT_OK;
    GT_VTU_ENTRY tmpVtuEntry;
    uint32_t     found = GT_FALSE;
    uint32_t     port;

    memset(&tmpVtuEntry,0,sizeof(GT_VTU_ENTRY));
    tmpVtuEntry.vid   = vid;

    rc = mv_switch_find_vid_entry_sw(&tmpVtuEntry, &found);
    if (rc != GT_OK) {
        printk(KERN_ERR "%s:%d:==ERROR==mv_switch_find_vid_entry_sw failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        return GT_FAIL;
    }

    if (found == GT_FALSE)/* VTU entry does not exist */
    {
        /* Add new VTU entry with VLAN ID vid with lport as members of the Vlan vid. */
        tmpVtuEntry.DBNum                           = 0;
        tmpVtuEntry.vid                             = vid;
        tmpVtuEntry.vidPriOverride                  = GT_FALSE;
        tmpVtuEntry.vidPriority                     = 0;
        tmpVtuEntry.vidExInfo.useVIDFPri            = GT_FALSE;
        tmpVtuEntry.vidExInfo.vidFPri               = 0;
        tmpVtuEntry.vidExInfo.useVIDQPri            = GT_FALSE;
        tmpVtuEntry.vidExInfo.vidQPri               = 0;
        tmpVtuEntry.vidExInfo.vidNRateLimit         = GT_FALSE;

    }

    /* In case the VTU entry exist:
     *
     *      Update the VTU entry with update vtuEntry.vtuData.memberTagP[port].
     *      The previous VTU data stored in tmpVtuEntry
     */

    for (port=0; port < qd_dev->numOfPorts; port++)
    {
        if (sw_vlan_tbl[vid].members & (1 << port)){
            if (port == lport)
                tmpVtuEntry.vtuData.memberTagP[port] = eMode;/* update egress mode only */
            else
                tmpVtuEntry.vtuData.memberTagP[port] = sw_vlan_tbl[vid].egr_mode[port];
        }
        else if (port == lport) {
            tmpVtuEntry.vtuData.memberTagP[port] = eMode;
        }
        else if (gmac0Idx == port) {/* add GMAC_0 to VLAN if gmac0Idx valid */
            tmpVtuEntry.vtuData.memberTagP[port] = MEMBER_EGRESS_UNMODIFIED;
        }
        else{
            tmpVtuEntry.vtuData.memberTagP[port] = NOT_A_MEMBER;

        }
    }

    rc = gvtuAddEntry(qd_dev,&tmpVtuEntry);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gvtuAddEntry failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        return GT_FAIL;
    }

    /* Record HW VT entry info to sw_vlan_tbl */
    rc = mv_switch_record_vid_entry_sw(&tmpVtuEntry);
    if (rc != GT_OK) {
        printk(KERN_ERR "%s:%d:==ERROR==mv_switch_record_vid_entry_sw failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        return GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS    rc = GT_OK;
    GT_VTU_ENTRY tmpVtuEntry;
    uint32_t     found = GT_TRUE;
    GT_U8        port_idx;
    uint32_t     isVlanMember = 0;

    /* Find vid in sw_vlan_tbl first */
    if (!(sw_vlan_tbl[vid].members & (1 << port)))
            found = GT_FALSE;

    if (found == GT_TRUE) /* VTU entry exist */
    {
        memset(&tmpVtuEntry,0,sizeof(GT_VTU_ENTRY));
        tmpVtuEntry.vid = vid;
        /* Read SW VT to get necessary entry info */
        rc = mv_switch_find_vid_entry_sw(&tmpVtuEntry, &found);
        if ( (rc != GT_OK) && (rc != GT_NO_SUCH) )
        {
            printk(KERN_ERR "%s:%d:==ERROR==mv_switch_find_vid_entry_sw failed rc[0x%x]\r\n",
                   __FUNCTION__, __LINE__, rc);

            rc = GT_FAIL;
            goto out_func;
        }

        /* 1. Mark the lport as NOT_A_MEMBER. */
        tmpVtuEntry.vtuData.memberTagP[port] = NOT_A_MEMBER;

        /* 2. Search whether a secure port is a member of the VLAN */
        for(port_idx=0; port_idx < qd_dev->numOfPorts; port_idx++)
        {
            if ((tmpVtuEntry.vtuData.memberTagP[port_idx] != NOT_A_MEMBER) &&
                 ((sw_ports_tbl[port_idx].port_mode == GT_SECURE)||
                  (sw_ports_tbl[port_idx].vlan_blong[vid]==SW_PORT_BELONG)) )
            {
                isVlanMember = 1;
                break;
            }
        }

        if (isVlanMember == 1) /* Only update the VTU */
        {
            rc = gvtuAddEntry(qd_dev,&tmpVtuEntry);
            if (rc != GT_OK)
            {
                printk(KERN_ERR "%s:%d:==ERROR==gvtuAddEntry failed rc[0x%x]\r\n",
                       __FUNCTION__, __LINE__, rc);

                rc = GT_FAIL;
                goto out_func;
            }

            /* Record HW VT entry info to sw_vlan_tbl */
            rc = mv_switch_record_vid_entry_sw(&tmpVtuEntry);
            if (rc != GT_OK) {
                printk(KERN_ERR "%s:%d:==ERROR==mv_switch_record_vid_entry_sw failed rc[0x%x]\r\n",
                       __FUNCTION__, __LINE__, rc);

                return GT_FAIL;
            }

            sw_vlan_tbl[vid].members &= ~(((uint32_t)0x1) << (port));/* Delete port from VID DB */
        }
        else/* Delete VTU */
        {
            rc = gvtuDelEntry(qd_dev, &tmpVtuEntry);
            if (rc != GT_OK)
            {
                printk(KERN_ERR "%s:%d:==ERROR==gvtuDelEntry failed rc[0x%x]\r\n",
                       __FUNCTION__, __LINE__, rc);

                rc = GT_FAIL;
                goto out_func;
            }

            sw_vlan_tbl[vid].members = 0;
        }
    }
    else /* VTU entry does not exist in search */
    {
        printk(KERN_ERR "%s:%d:==ERROR== No Such VID in Port(%d) Vlan List\r\n",__FUNCTION__,__LINE__, port);

        rc = GT_FAIL;
        goto out_func;

    }

out_func:
    return rc;
}

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
	IN uint32_t lport,
    IN uint8_t  static_mac[6]
)
{
    GT_STATUS    rc = GT_OK;
    GT_ATU_ENTRY mac_entry;

    memset(&mac_entry, 0, sizeof(GT_ATU_ENTRY));

    mac_entry.trunkMember = GT_FALSE;
    mac_entry.prio = 0;
    mac_entry.exPrio.useMacFPri = 0;
    mac_entry.exPrio.macFPri = 0;
    mac_entry.exPrio.macQPri = 0;
    mac_entry.DBNum = 0;
    mac_entry.entryState.mcEntryState = GT_UC_STATIC;
    mac_entry.portVec = 1 << lport;
    memcpy(mac_entry.macAddr.arEther, static_mac, 6);

    rc = gfdbAddMacEntry(qd_dev, &mac_entry);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gfdbAddMacEntry failed rc[0x%x]\n\r",
               __FUNCTION__, __LINE__, rc);
        rc = GT_FAIL;
    }

    return rc;
}

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
    IN uint8_t static_mac[6]
)
{
    GT_STATUS    rc = GT_OK;
    GT_ETHERADDR macAddress;

    memcpy(&macAddress.arEther, static_mac, GT_ETHERNET_HEADER_SIZE);

    rc = gfdbDelMacEntry(qd_dev, &macAddress);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gfdbDelMacEntry failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
    IN uint32_t lport,
    IN uint8_t 	mac_per_port
)
{
    GT_STATUS rc = GT_OK;

#if 0
    /* Efi Dror - Not neccary to perform - disable learning on the ports*/

    /* dsable learning on the ports */
    rc = gprtSetLearnDisable(qd_dev, lport,GT_TRUE);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetLearnDisable failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
        goto out_func;
    }

    /* Efi Dror - Not neccary to perform - Flash all adresses in ATU*/

    /* Flash all adresses in ATU */
    rc = gfdbFlush(qd_dev, GT_FLUSH_ALL);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gfdbFlush failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
        goto out_func;
    }
#endif

    /* define the desired limit for the ports */
    rc = gfdbSetPortAtuLearnLimit(qd_dev, lport, mac_per_port);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gfdbSetPortAtuLearnLimit failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
        goto out_func;
    }

#if 0
    /* Efi Dror - Not neccary to perform - re-enable learing on the ports */

    /* re-enable learing on the ports */
    rc = gprtSetLearnDisable(qd_dev, lport,GT_FALSE);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetLearnDisable failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
        goto out_func;
    }
#endif

out_func:
    return rc;
}

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
    IN uint32_t  lport,
    OUT uint32_t *limit
)
{
    GT_STATUS rc = GT_OK;

    rc = gfdbGetPortAtuLearnLimit(qd_dev, lport, (GT_U32*)limit);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gfdbGetPortAtuLearnLimit failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}
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
int32_t mv_switch_clear_dynamic_mac(void)
{
     GT_STATUS rc = GT_OK;

    rc = gfdbClearAtuDynamicEntry(qd_dev);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gfdbClearAtuDynamicEntry failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
int32_t mv_switch_set_mirror(IN uint32_t  sport, IN uint32_t  dport, IN GT_MIRROR_MODE mode, IN GT_BOOL enable)
{
    GT_STATUS rc = GT_OK;

    switch(mode)
    {
        case SW_MIRROR_INGRESS:
	     /*enable mirror*/
	     if(enable == GT_TRUE)
	     {
    	         /*enable ingress monitor source register*/
    	         rc = gprtSetIngressMonitorSource(qd_dev, sport, GT_TRUE);
                if (rc != GT_OK)
                {
                    printk(KERN_ERR "%s:%d:==ERROR==gprtSetIngressMonitorSource failed rc[0x%x]\r\n",
                       __FUNCTION__, __LINE__, rc);

                    rc = GT_FAIL;
                }

    	         /*set ingress monitor destination register*/
    	         rc = gsysSetIngressMonitorDest(qd_dev, dport);
                if (rc != GT_OK)
                {
                    printk(KERN_ERR "%s:%d:==ERROR==gprtSetIngressMonitorSource failed rc[0x%x]\r\n",
                       __FUNCTION__, __LINE__, rc);

                    rc = GT_FAIL;
                }
	     }
	     else
	     {
	          /*disable ingress monitor source register*/
    	         rc = gprtSetIngressMonitorSource(qd_dev, sport, GT_FALSE);
                if (rc != GT_OK)
                {
                    printk(KERN_ERR "%s:%d:==ERROR==gprtSetIngressMonitorSource failed rc[0x%x]\r\n",
                       __FUNCTION__, __LINE__, rc);

                    rc = GT_FAIL;
                }
	     }

	     break;

        case SW_MIRROR_EGRESS:

	     /*enable mirror*/
	     if(enable == GT_TRUE)
	     {
    	         /*enable egress monitor source register*/
    	         rc = gprtSetEgressMonitorSource(qd_dev, sport, GT_TRUE);
                if (rc != GT_OK)
                {
                    printk(KERN_ERR "%s:%d:==ERROR==gprtSetEgressMonitorSource failed rc[0x%x]\r\n",
                       __FUNCTION__, __LINE__, rc);

                    rc = GT_FAIL;
                }

    	         /*set egress monitor destination register*/
    	         rc = gsysSetEgressMonitorDest(qd_dev, dport);
                if (rc != GT_OK)
                {
                    printk(KERN_ERR "%s:%d:==ERROR==gsysSetEgressMonitorDest failed rc[0x%x]\r\n",
                       __FUNCTION__, __LINE__, rc);

                    rc = GT_FAIL;
                }
	     }
	     else
	     {
	          /*disable egress monitor source register*/
    	         rc = gprtSetEgressMonitorSource(qd_dev, sport, GT_FALSE);
                if (rc != GT_OK)
                {
                    printk(KERN_ERR "%s:%d:==ERROR==gprtSetEgressMonitorSource failed rc[0x%x]\r\n",
                       __FUNCTION__, __LINE__, rc);

                    rc = GT_FAIL;
                }
	     }

	     break;

	 default:
	     break;
    }

    return rc;
}

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
int32_t mv_switch_get_mirror(IN uint32_t  sport, IN uint32_t  dport, IN GT_MIRROR_MODE mode, OUT GT_BOOL* enable)
{
    GT_STATUS rc = GT_OK;

    switch(mode)
    {
        case SW_MIRROR_INGRESS:

            /*Get ingress monitor source register status*/
            rc = gprtGetIngressMonitorSource(qd_dev, sport, enable);
            if (rc != GT_OK)
            {
                printk(KERN_ERR "%s:%d:==ERROR==gprtGetIngressMonitorSource failed rc[0x%x]\r\n",
                   __FUNCTION__, __LINE__, rc);

                rc = GT_FAIL;
            }

	     break;

        case SW_MIRROR_EGRESS:

            /*Get egress monitor source register status*/
            rc = gprtGetEgressMonitorSource(qd_dev, sport, enable);
            if (rc != GT_OK)
            {
                printk(KERN_ERR "%s:%d:==ERROR==gprtGetEgressMonitorSource failed rc[0x%x]\r\n",
                   __FUNCTION__, __LINE__, rc);

                rc = GT_FAIL;
            }

	     break;

	 default:
	     break;
    }

    return rc;
}

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
int32_t mv_switch_set_mtu(IN uint32_t mtu)
{
    uint32_t  i;
    GT_STATUS rc = GT_OK;
    GT_JUMBO_MODE   jumbo_mode;

    /* Set jumbo frames mode */
    if(mtu <= 1522)
        jumbo_mode = GT_JUMBO_MODE_1522;
    else if(mtu <= 2048)
        jumbo_mode = GT_JUMBO_MODE_2048;
    else
       	jumbo_mode = GT_JUMBO_MODE_10240;

    for(i=0; i<qd_dev->numOfPorts; i++)
    {
    	 /*Set switch MTU*/
	      rc = gsysSetJumboMode(qd_dev, i, jumbo_mode);
	      if (rc != GT_OK)
          {
              printk(KERN_ERR "%s:%d:==ERROR==gsysSetJumboMode failed rc[0x%x]\r\n",
                 __FUNCTION__, __LINE__, rc);

              rc = GT_FAIL;
				return rc;
          }
    }
    return rc;
}

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
int32_t mv_switch_get_mtu(OUT uint32_t* mtu)
{
    GT_STATUS rc = GT_OK;
    GT_JUMBO_MODE   jumbo_mode;

	/*Get switch MTU*/
	rc = gsysGetJumboMode(qd_dev, 4/*uplink port for DEV_88E6351_FAMILY switch*/, &jumbo_mode);
	if (rc != GT_OK)
	{
		printk(KERN_ERR "%s:%d:==ERROR==gsysSetJumboMode failed rc[0x%x]\r\n",
		   __FUNCTION__, __LINE__, rc);

		rc = GT_FAIL;
	}

    /* Convert jumbo frames mode to MTU size */
    if(jumbo_mode == GT_JUMBO_MODE_1522)
		*mtu = 1522;
    else if(jumbo_mode == GT_JUMBO_MODE_2048)
		*mtu = 2048;
    else
		*mtu = 10240;

    return rc;
}



#if 0 /* The masked one is old code, just remains as a reference*/
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
    IN uint32_t lport,
    IN GT_BOOL 	mode
)
{
    GT_STATUS rc = GT_OK;

    rc = gprtSetForwardUnknown(qd_dev, lport, mode);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetForwardUnknown failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
    IN  uint32_t lport,
    OUT uint32_t *mode
)
{
    GT_STATUS rc = GT_OK;

    rc = gprtGetForwardUnknown(qd_dev, lport, mode);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtGetForwardUnknown failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}
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
    IN uint32_t lport,
    IN GT_BOOL  mode
)
{
    GT_STATUS rc = GT_OK;

    rc = gprtSetDiscardTagged(qd_dev, lport, mode);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetDiscardTagged failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
    IN  uint32_t lport,
    OUT uint32_t *mode
)
{
    GT_STATUS rc = GT_OK;

    rc = gprtGetDiscardTagged(qd_dev, lport, mode);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetDiscardTagged failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_OK;
    }

    return rc;
}

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
    IN uint32_t lport,
    IN GT_BOOL 	mode
)
{
    GT_STATUS rc = GT_OK;

    rc = gprtSetDiscardUntagged(qd_dev, lport, mode);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetDiscardUntagged failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_OK;
    }

    return rc;
}

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
    IN  uint32_t lport,
    OUT uint32_t *mode
)
{
    GT_STATUS rc = GT_OK;

    rc = gprtGetDiscardUntagged(qd_dev, lport, mode);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetDiscardUntagged failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    rc = gvlnSetPortVid(qd_dev, lport, vid);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gvlnSetPortVid failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_OK;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    rc = gvlnGetPortVid(qd_dev, lport, vid);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gvlnGetPortVid failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}


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
)
{
    GT_STATUS rc = GT_OK;

    rc = gcosSetPortDefaultTc(qd_dev, lport, pri);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gcosSetPortDefaultTc failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_OK;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    rc = gcosGetPortDefaultTc(qd_dev, lport, pri);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gcosGetPortDefaultTc failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
    IN uint32_t lport,
    IN uint16_t vid,
    IN uint16_t gmac0Idx
)
{
    GT_STATUS rc = GT_OK;

    rc = mv_switch_port_add_vid_set_egrs_mode(lport, vid, gmac0Idx, MEMBER_EGRESS_UNMODIFIED);

    return rc;
}

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
    IN uint32_t lport,
    IN uint16_t vid
)
{
    GT_STATUS rc = GT_OK;
    uint32_t  port_bm;

    /* Verify whether the port is a member of this VID */
    port_bm = (uint32_t)(1 << lport);
    if ( sw_vlan_tbl[vid].members & port_bm )
    {
        rc = mv_switch_prv_del_vid_per_port(lport, vid);
        if (rc != GT_OK)
        {
            printk(KERN_ERR "%s:%d:==ERROR==tpm_sw_prv_del_vid_per_port failed rc[0x%x]\r\n",
                   __FUNCTION__, __LINE__, rc);

            rc = GT_FAIL;
        }
        else
        {
            if (sw_ports_tbl[lport].vlan_blong[vid] == SW_PORT_BELONG )
            {
                if (sw_ports_tbl[lport].cnt)
                    sw_ports_tbl[lport].cnt--;
                sw_ports_tbl[lport].vlan_blong[vid] = SW_PORT_NOT_BELONG;
                sw_vlan_tbl[vid].egr_mode[lport]    = NOT_A_MEMBER;
            }
        }
            }


    return rc;
        }

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
*       On error different types are returned according to the case see tpm_error_code_t.
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
)
{
    GT_STATUS    rc = GT_OK;
    GT_VTU_ENTRY vtuEntry;
    GT_BOOL      found = GT_FALSE;

    /* If the port is the member of vlan, set */
    if (sw_vlan_tbl[vid].members & (1 << lport))
    {
        sw_vlan_tbl[vid].egr_mode[lport] = eMode;

        memset(&vtuEntry, 0, sizeof(GT_VTU_ENTRY));
        vtuEntry.vid = vid;

        rc = mv_switch_find_vid_entry_sw(&vtuEntry, &found);
        if (rc != GT_OK && rc != GT_NO_SUCH)
        {
            printk(KERN_ERR"%s:%d:==ERROR==mv_switch_find_vid_entry_sw failed rc[0x%x]\r\n",
                   __FUNCTION__, __LINE__, rc);

            return GT_FAIL;
        }

        printk(KERN_DEBUG "%s: found[%d]\n\r", __FUNCTION__, found);

        vtuEntry.vtuData.memberTagP[lport] = eMode;

        rc = gvtuAddEntry(qd_dev, &vtuEntry);
        if (rc != GT_OK)
        {
            printk(KERN_ERR"%s:%d:==ERROR==gvtuAddEntry failed rc[0x%x]\r\n",
                   __FUNCTION__, __LINE__, rc);

            return GT_FAIL;
        }

        /* Record HW VT entry info to sw_vlan_tbl */
        rc = mv_switch_record_vid_entry_sw(&vtuEntry);
        if (rc != GT_OK) {
            printk(KERN_ERR "%s:%d:==ERROR==mv_switch_record_vid_entry_sw failed rc[0x%x]\r\n",
                   __FUNCTION__, __LINE__, rc);

            return GT_FAIL;
        }
    }
    else
    {
        printk(KERN_ERR"%s:%d:==ERROR==port[%d] is not the member of vlan[%d]\r\n",
               __FUNCTION__, __LINE__, lport, vid);
        rc = GT_FAIL;
    }

    return rc;
}

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
    IN  uint32_t vid,
    OUT uint32_t *found
)
{
    GT_STATUS    rc = GT_OK;
    GT_VTU_ENTRY tmpVtuEntry;
    uint32_t     portIndex;
    uint32_t     port;

    memset(&tmpVtuEntry,0,sizeof(GT_VTU_ENTRY));
	tmpVtuEntry.vid = vid;

    rc = gvtuFindVidEntry(qd_dev, &tmpVtuEntry, found);
    if ( (rc != GT_OK) && (rc != GT_NO_SUCH) )
    {
        printk(KERN_ERR "%s:%d:==ERROR==gvtuFindVidEntry failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }
    else if (rc == GT_NO_SUCH)
    {
        printk(KERN_ERR "%s: VTU not vid[%d]\n\r", __FUNCTION__, tmpVtuEntry.vid);
    }
    else
    {
        if (found)
        {
            printk("DBNum:%i, VID:%i \n",tmpVtuEntry.DBNum,tmpVtuEntry.vid);
            printk("Tag: ");

            for(portIndex=0; portIndex < qd_dev->numOfPorts; portIndex++)
            {
                port = portIndex;
                printk("%d %d;",port,tmpVtuEntry.vtuData.memberTagP[port]);
            }

            printk("\n");

            printk("vidPriOverride[%d],vidPriority[%d],useVIDFPri[%d],vidFPri[%d],useVIDQPri[%d],vidQPri[%d],vidNRateLimit[%d]\n",
                   tmpVtuEntry.vidPriOverride,
                   tmpVtuEntry.vidPriority,
                   tmpVtuEntry.vidExInfo.useVIDFPri,
                   tmpVtuEntry.vidExInfo.vidFPri,
                   tmpVtuEntry.vidExInfo.useVIDQPri,
                   tmpVtuEntry.vidExInfo.vidQPri,
                   tmpVtuEntry.vidExInfo.vidNRateLimit);
        }
    }

    return rc;
}



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
)
{
    GT_STATUS rc = GT_OK;
    uint32_t  port_bm;
    uint16_t  vlan_idx;

    sw_ports_tbl[port].port_mode = GT_SECURE;

    port_bm = 1 << port;

    for (vlan_idx=0; vlan_idx < SW_MAX_VLAN_NUM; vlan_idx++)
    {
        if ((sw_vlan_tbl[vlan_idx].members & port_bm) &&
            (sw_ports_tbl[port].vlan_blong[vlan_idx] == SW_PORT_NOT_BELONG))
        {
            rc |= mv_switch_prv_del_vid_per_port(port, vlan_idx);

            if (rc != GT_OK)
            {
                printk(KERN_ERR
                       "%s:%d:==ERROR== failed to remove port [%d] from VLAN [%d]\r\n",
                       __FUNCTION__,__LINE__,port, vlan_idx);
            }

            sw_vlan_tbl[vlan_idx].egr_mode[port] = NOT_A_MEMBER;
        }
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;
    uint32_t  port_bm;
    uint16_t  vlan_idx;

    sw_ports_tbl[port].port_mode = GT_FALLBACK;

    port_bm = 1 << port;

    for (vlan_idx=0; vlan_idx < SW_MAX_VLAN_NUM; vlan_idx++)
    {
        /* If a VLAN has been defined (there is a member in the VLAN) and
           the specified port is not a member */
        if ( sw_vlan_tbl[vlan_idx].members &&
             !(sw_vlan_tbl[vlan_idx].members & port_bm))
        {
            /* Update VTU table */
            rc |= mv_switch_prv_port_add_vid(port, vlan_idx, 0xFFFF, MEMBER_EGRESS_UNMODIFIED);

            if (rc != GT_OK)
            {
                printk(KERN_ERR
                       "%s:%d:==ERROR== failed to add port [%d] to VLAN [%d]\r\n",
                       __FUNCTION__,__LINE__,port, vlan_idx);
                return rc;
            }

            sw_vlan_tbl[vlan_idx].members |= port_bm;
            sw_vlan_tbl[vlan_idx].egr_mode[port] = MEMBER_EGRESS_UNMODIFIED;

        }
    }

    return rc;
}

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
*       vid_filter  - set to 1 means the lport will DROP all packets which are NOT in
*                    the allowed VID list (built using API tpm_sw_port_add_vid).
*                    set to 0 - means that the list of VIDs allowed
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
    IN uint32_t lport,
    IN uint8_t  vid_filter
)
{
    GT_STATUS     rc = GT_OK;
    GT_DOT1Q_MODE mode;

    if (vid_filter == 1)
    {
        /* Port should be moved to the secure mode and removed from all VLANs */

        if (sw_ports_tbl[lport].port_mode == GT_SECURE)
        {
            /* The port is already in the secure mode - do noting */
            return rc;
        }

        rc   = mv_switch_prv_set_secure_mode(lport);
        mode = GT_SECURE;
    }
    else
    {
        /* Port should be moved to the fallback mode and added to all VLANs */
        if (sw_ports_tbl[lport].port_mode == GT_FALLBACK)
        {
            /* The port is already in the fallback mode - do noting */
            return rc;
        }

        rc   = mv_switch_prv_set_fallback_mode(lport);
        mode = GT_FALLBACK;
    }

    if (rc != GT_OK)
    {
        printk(KERN_INFO
               "%s:%d:==ERROR== failed to set new port mode - retval[%d]\r\n",
               __FUNCTION__,__LINE__,rc);
        return GT_FAIL;
    }

    rc = gvlnSetPortVlanDot1qMode(qd_dev, lport, mode);
    if (rc != GT_OK)
    {
        printk(KERN_INFO
               "%s:%d:==ERROR==gvlnSetPortVlanDot1qMode failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__,rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS     rc = GT_OK;
    GT_DOT1Q_MODE mode;

    rc = gvlnGetPortVlanDot1qMode(qd_dev, lport, &mode);
    if (rc != GT_OK)
    {
        printk(KERN_INFO
               "%s:%d:==ERROR==gvlnGetPortVlanDot1qMode failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__,rc);

        rc = GT_FAIL;
    }

    if (GT_SECURE == mode)
        *vid_filter = 1;
    else
        *vid_filter = 0;
    return rc;
}

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
    IN uint32_t      uni_port,
    IN GT_PORT_SCHED_MODE  mode
)
{
    GT_STATUS           rc = GT_OK;

    rc = gprtSetPortSched(qd_dev, uni_port, mode);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetPortSched failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

/*******************************************************************************
* mv_switch_set_uni_q_weight
*
* DESCRIPTION:
*       The API configures the weight of a queues for all
*       Ethernet UNI ports in the integrated switch.
*
* INPUTS:
*       owner_id - APP owner id - should be used for all API calls.
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
    IN uint8_t queue_id,
    IN uint8_t weight
)
{
    GT_STATUS     rc = GT_OK;
    uint32_t      len;
    uint32_t      i,j;
    GT_QoS_WEIGHT Weight;

    len = 0;
    /* Update global weight_array */
    weight_arr[queue_id-1] = weight;

    for (i = 0; i < SW_QOS_NUM_OF_QUEUES; i++)
    {
        for(j = 0; j < weight_arr[i];j++)
        {
            Weight.queue[j] = i;
            len++;
        }
    }

    Weight.len = len;

    rc = gsysSetQoSWeight(qd_dev, &Weight);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gsysSetQoSWeight failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS     rc = GT_OK;
    GT_U32		  irlRes;
    GT_PIRL2_DATA pirl_2_Data;
    GT_BOOL pause_state;

    memset(&pirl_2_Data,0,sizeof(GT_PIRL_DATA));

    irlRes = 0;

    /* Gets the PIRL2 default params for uni_port
    rc = gpirl2ReadResource(qd_dev, uni_port, irlRes, &pirl_2_Data);
    if (rc != GT_OK)
    {
        printk(KERN_ERR
               "%s:%d:==ERROR==pirl2ReadResource failed rc[0x%x]\r\n",
               __FUNCTION__,
               __LINE__, rc);

        rc = GT_FAIL;
        goto out_func;
    }*/

    /* configure tc,cbs,cir on the uni_port */
    pirl_2_Data.ingressRate             = cir;
    pirl_2_Data.customSetup.isValid     = GT_FALSE;
    pirl_2_Data.accountQConf 		    = GT_FALSE;
    pirl_2_Data.accountFiltered	        = GT_TRUE;
    pirl_2_Data.mgmtNrlEn               = GT_TRUE;
    pirl_2_Data.saNrlEn                 = GT_FALSE;
    pirl_2_Data.daNrlEn                 = GT_FALSE;
    pirl_2_Data.samplingMode            = GT_FALSE;
    pirl_2_Data.actionMode              = PIRL_ACTION_USE_LIMIT_ACTION;

    /*Decide which mode to adopt when deal with overload traffic.
	*  If pause state is ON, select FC mode, otherwize select drop mode.
	*/
    mv_switch_get_port_pause_state(uni_port, &pause_state);
    if(pause_state == GT_TRUE)
    {
        pirl_2_Data.ebsLimitAction          = ESB_LIMIT_ACTION_FC;
    }
	else
	{
    pirl_2_Data.ebsLimitAction          = ESB_LIMIT_ACTION_DROP;
	}

    /*pirl_2_Data.ebsLimitAction          = ESB_LIMIT_ACTION_DROP;	*/

    pirl_2_Data.fcDeassertMode          = GT_PIRL_FC_DEASSERT_EMPTY;
    pirl_2_Data.bktRateType		        = BUCKET_TYPE_TRAFFIC_BASED;
    pirl_2_Data.priORpt                 = GT_TRUE;
    pirl_2_Data.priMask                 = 0;
    pirl_2_Data.bktTypeMask             = 0x7fff;
	/*Input code mode*/
    pirl_2_Data.byteTobeCounted	        = count_mode;
    /*pirl_2_Data.byteTobeCounted	        = GT_PIRL2_COUNT_ALL_LAYER2;*/

    /*Set custom param: cbs, ebs*/
    pirl_2_Data.customSetup.cbsLimit   = cbs;
    pirl_2_Data.customSetup.ebsLimit     = ebs;

    rc = gpirl2WriteResource(qd_dev, uni_port, irlRes, &pirl_2_Data);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gpirl2WriteResource failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

/*******************************************************************************
* mv_switch_get_uni_ingr_police_rate
*
* DESCRIPTION:
*       The API gets an ingress policing function for an Ethernet UNI lport.
*
* INPUTS:
*       owner_id - APP owner id - should be used for all API calls.
*       uni_port - uni lport for configuring the ingress policer function.

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
)
{
    GT_STATUS     rc = GT_OK;
    GT_PIRL2_DATA pirl_2_Data;
    GT_U32		  irlUnit;

    irlUnit =  0; /* IRL Unit 0 - bucket to be used (0 ~ 4) */
	memset(&pirl_2_Data, 0, sizeof(GT_PIRL2_DATA));

    rc = gpirl2ReadResource(qd_dev, uni_port, irlUnit, &pirl_2_Data);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==pirl2ReadResource failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    *count_mode = pirl_2_Data.byteTobeCounted;
    *cir = pirl_2_Data.ingressRate;
    *cbs = pirl_2_Data.customSetup.cbsLimit;
    *ebs = pirl_2_Data.customSetup.ebsLimit;

    return rc;
}

/*******************************************************************************
* mv_switch_set_uni_tc_ingr_police_rate
*
* DESCRIPTION:
*       The API Configures a policer function for a traffic class for an Ethernet UNI lport.
*       There are 4 globally defined traffic classes in the integrated switch.
*
* INPUTS:
*       owner_id - APP owner id - should be used for all API calls.
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
    IN uint32_t	uni_port,
    IN uint32_t	tc,
	IN uint32_t cir,
	IN uint32_t	cbs
)
{
    GT_STATUS     rc = GT_OK;
    GT_U32		  irlRes;
    GT_PIRL2_DATA pirl_2_Data;
    GT_U32		  priMask = 1;

    irlRes = tc;
    memset(&pirl_2_Data,0,sizeof(GT_PIRL_DATA));

    /* Gets the PIRL2 default params for uni_port
    rc = gpirl2ReadResource(qd_dev, uni_port, irlRes, &pirl_2_Data);
    if (rc != GT_OK)
    {
        printk(KERN_ERR
               "%s:%d:==ERROR==pirl2ReadResource failed rc[0x%x]\r\n",
               __FUNCTION__,
               __LINE__, rc);

        rc = GT_FAIL;
        goto out_func;
    }*/

    /* configure tc,cbs,cir on the uni_port */
    pirl_2_Data.ingressRate             = cir;
    pirl_2_Data.customSetup.isValid     = GT_FALSE;
    pirl_2_Data.accountQConf 		    = GT_FALSE;
    pirl_2_Data.accountFiltered	        = GT_TRUE;
    pirl_2_Data.mgmtNrlEn               = GT_TRUE;
    pirl_2_Data.saNrlEn                 = GT_FALSE;
    pirl_2_Data.daNrlEn                 = GT_FALSE;
    pirl_2_Data.samplingMode            = GT_FALSE;
    pirl_2_Data.actionMode              = PIRL_ACTION_USE_LIMIT_ACTION;
    pirl_2_Data.ebsLimitAction          = ESB_LIMIT_ACTION_DROP;
    pirl_2_Data.fcDeassertMode          = GT_PIRL_FC_DEASSERT_EMPTY;
    pirl_2_Data.bktRateType		        = BUCKET_TYPE_TRAFFIC_BASED;
    pirl_2_Data.priORpt                 = GT_FALSE;
    pirl_2_Data.priMask                 = priMask << tc;
    pirl_2_Data.bktTypeMask             = 0x7fff;
    pirl_2_Data.byteTobeCounted	        = GT_PIRL2_COUNT_ALL_LAYER2;

    pirl_2_Data.customSetup.cbsLimit    = cbs;

    rc = gpirl2WriteResource(qd_dev, uni_port, irlRes, &pirl_2_Data);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gpirl2WriteResource failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
	IN  uint32_t uni_port,
	OUT uint32_t *tc,
	OUT uint32_t *cir,
	OUT uint32_t *cbs
)
{
	GT_STATUS     rc = GT_OK;
	GT_U32		  irlRes;
	GT_PIRL2_DATA pirl_2_Data[5];
	uint8_t       i;

	irlRes = 0;/* bucket to be used (0 ~ 4) */

	for (irlRes = 0; irlRes <= 4; irlRes ++) {
		/* Gets the CBS params */
		rc = gpirl2ReadResource(qd_dev, uni_port, irlRes, &pirl_2_Data[irlRes]);
		if (rc != GT_OK) {
			printk(KERN_ERR "%s:%d:==ERROR==gpirl2ReadResource failed rc[0x%x]\r\n",
				__FUNCTION__, __LINE__, rc);

			rc = GT_FAIL;
		}
		tc[irlRes]  = 0;
		cbs[irlRes] = pirl_2_Data[irlRes].customSetup.cbsLimit;
		cir[irlRes] = pirl_2_Data[irlRes].ingressRate;

		for (i=0; i < SW_QOS_NUM_OF_QUEUES; i++) {
			if (pirl_2_Data[irlRes].priMask & (1 << i))
				tc[irlRes] = i;
		}
	}

	return rc;
}

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
)
{
    GT_STATUS     rc = GT_OK;
    GT_ERATE_TYPE fRate;

    fRate.fRate = frame_rate_limit_val;
	 fRate.kbRate = frame_rate_limit_val;

    rc = grcSetELimitMode(qd_dev, trg_lport, mode);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==grcSetELimitMode failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    rc = grcSetEgressRate(qd_dev, trg_lport, &fRate);
    if (rc != GT_OK)
    {
        printk(KERN_INFO "%s:%d:==ERROR==grcSetEgressRate failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
    IN  uint32_t trg_lport,
    OUT GT_PIRL_ELIMIT_MODE *mode,
	OUT uint32_t *frame_rate_limit_val
)
{
    GT_STATUS     rc = GT_OK;
    GT_ERATE_TYPE fRate;

    rc = grcGetELimitMode(qd_dev, trg_lport, mode);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==grcGetELimitMode failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    rc = grcGetEgressRate(qd_dev, trg_lport, &fRate);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==grcGetEgressRate failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

     /*frame based limit*/
	 if(mode == GT_PIRL_ELIMIT_FRAME)
	 {
    *frame_rate_limit_val = fRate.fRate;
	 }
    else/*rate based limit*/
	 {
        *frame_rate_limit_val = fRate.kbRate;
	 }

    return rc;
}

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
	IN uint32_t time_out
)
{
    GT_STATUS rc = GT_OK;

    rc = gfdbSetAgingTimeout(qd_dev, time_out);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gfdbSetAgingTimeout failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    rc = gprtSetLearnEnable(qd_dev, lport, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetLearnEnable failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    rc = gprtGetLearnEnable(qd_dev, lport, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtGetLearnEnable failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
	IN uint32_t lport,
    IN uint32_t memPorts[],
    IN uint8_t  memPortsLen
)
{
    GT_STATUS rc = GT_OK;

    rc = gvlnSetPortVlanPorts(qd_dev, lport, (GT_LPORT*)memPorts, memPortsLen);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gvlnSetPortVlanPorts failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
	IN  uint32_t lport,
    OUT uint32_t memPorts[],
    OUT uint8_t  *memPortsLen
)
{
    GT_STATUS rc = GT_OK;

    rc = gvlnGetPortVlanPorts(qd_dev, lport, (GT_LPORT*)memPorts, memPortsLen);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gvlnGetPortVlanPorts failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
	OUT uint32_t *time_out
)
{
    GT_STATUS rc = GT_OK;

    rc = gfdbGetAgingTimeout(qd_dev, (GT_U32*)time_out);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gfdbGetAgingTimeout failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

/*******************************************************************************
* mv_switch_set_ingress_limit_mode
*
* DESCRIPTION:
*       This routine sets the port's rate control ingress limit mode.
*
* INPUTS:
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
	IN uint32_t           lport,
    IN GT_RATE_LIMIT_MODE mode
)
{
    GT_STATUS rc = GT_OK;

    rc = grcSetLimitMode(qd_dev, lport, mode);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==grcSetLimitMode failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

/*******************************************************************************
* mv_switch_get_ingress_limit_mode
*
* DESCRIPTION:
*       This routine gets the port's rate control ingress limit mode.
*
* INPUTS:
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
	IN  uint32_t           lport,
    OUT GT_RATE_LIMIT_MODE *mode
)
{
    GT_STATUS rc = GT_OK;

    rc = grcGetLimitMode(qd_dev, lport, mode);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==grcGetLimitMode failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    if (PRI_SEL_TAG_PRI_ONLY == init_pri || \
        PRI_SEL_TAG_AND_IP_PRI == init_pri)
    {
        rc = gqosUserPrioMapEn(qd_dev, lport, GT_TRUE);
    }
    else
    {
        rc = gqosUserPrioMapEn(qd_dev, lport, GT_FALSE);
    }
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gqosUserPrioMapEn failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        return GT_FAIL;
    }

    if (PRI_SEL_IP_PRI_ONLY == init_pri || \
        PRI_SEL_TAG_AND_IP_PRI == init_pri)
    {
        rc = gqosIpPrioMapEn(qd_dev, lport, GT_TRUE);
    }
    else
    {
        rc = gqosIpPrioMapEn(qd_dev, lport, GT_FALSE);
    }
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gqosIpPrioMapEn failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        return GT_FAIL;
    }

    if (PRI_SEL_TAG_AND_IP_PRI == init_pri)
    {
        rc = gqosSetPrioMapRule(qd_dev, lport, tag_if_both);
        if (rc != GT_OK)
        {
            printk(KERN_ERR "%s:%d:==ERROR==gqosSetPrioMapRule failed rc[0x%x]\r\n",
                   __FUNCTION__, __LINE__, rc);
            return GT_FAIL;
        }
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL ip_en, tag_en;

    rc = gqosGetUserPrioMapEn(qd_dev, lport, &tag_en);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gqosGetUserPrioMapEn failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        rc = GT_FAIL;
    }

    rc = gqosGetIpPrioMapEn(qd_dev, lport, &ip_en);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gqosGetIpPrioMapEn failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        rc = GT_FAIL;
    }

    *init_pri = (ip_en << 1) | tag_en;

    rc = gqosGetPrioMapRule(qd_dev, lport, tag_if_both);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gqosGetPrioMapRule failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    rc = gcosSetUserPrio2Tc(qd_dev, tag_pri, q_pri);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gcosSetUserPrio2Tc failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        return GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    rc = gcosGetUserPrio2Tc(qd_dev, tag_pri, q_pri);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gcosGetUserPrio2Tc failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        return GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    rc = gcosSetDscp2Tc(qd_dev, dscp, q_pri);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gcosSetDscp2Tc failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        return GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    rc = gcosGetDscp2Tc(qd_dev, dscp, q_pri);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gcosGetDscp2Tc failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        return GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    rc = gprtSetForwardUnknown(qd_dev, lport, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetForwardUnknown failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    rc = gprtGetForwardUnknown(qd_dev, lport, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtGetForwardUnknown failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    rc = gprtSetDefaultForward(qd_dev, lport, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetDefaultForward failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    rc = gprtGetDefaultForward(qd_dev, lport, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtGetDefaultForward failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    rc = gsysSetFloodBC(qd_dev, always_on);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gsysSetFloodBC failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    rc = gsysGetFloodBC(qd_dev, always_on);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gsysGetFloodBC failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    rc = gprtSetHeaderMode(qd_dev, lport, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetHeaderMode failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    rc = gprtGetHeaderMode(qd_dev, lport, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtGetHeaderMode failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;
	uint16_t data;

	rc = mv_switch_set_port_reg(3, 26, preamble);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==mv_switch_set_port_reg failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        rc = GT_FAIL;
    }

	mvOsDelay(10);

	data = 0xb002 | (lport << 8);
	rc = mv_switch_set_port_reg(2, 26, data);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==mv_switch_set_port_reg failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;
	uint16_t data;

	data = 0xc002 | (lport << 8);
	rc = mv_switch_set_port_reg(2, 26, data);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==mv_switch_set_port_reg failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        rc = GT_FAIL;
    }

	mvOsDelay(10);

	rc = mv_switch_get_port_reg(3, 26, &data);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==mv_switch_get_port_reg failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
        rc = GT_FAIL;
    }

	*preamble = data;

    return rc;
}

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
)
{
    GT_STATUS     rc = GT_OK;

    if(GT_FALSE == autoneg_state)
    {
        rc = gprtPortAutoNegEnable(qd_dev, lport, GT_FALSE);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtPortAutoNegEnable failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }
        return rc;
    }
    else
    {
        rc = gprtPortAutoNegEnable(qd_dev, lport, GT_TRUE);
        if (rc != GT_OK)
        {
            printk(KERN_ERR "%s:%d:==ERROR==gprtPortAutoNegEnable failed rc[0x%x]\r\n",
                   __FUNCTION__, __LINE__, rc);

            rc = GT_FAIL;
            return rc;
        }
    }

    rc = gprtSetPortAutoMode(qd_dev, lport, autoneg_mode);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetPortAutoMode failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS     rc = GT_OK;

    rc = gprtGetPortAutoNegState(qd_dev, lport, autoneg_state);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtGetPortAutoNegState failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
        return rc;
    }

    rc = gprtGetPortAutoMode(qd_dev, lport, autoneg_mode);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtGetPortAutoMode failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}
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
)
{
    GT_STATUS     rc = GT_OK;

    rc = gprtPortRestartAutoNeg(qd_dev, lport);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtPortRestartAutoNeg failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}
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
)
{
    GT_STATUS     rc = GT_OK;
    GT_BOOL state;

    if(phy_port_state == GT_TRUE)
    {
	state = GT_FALSE;
    }
    else
    {
    	state = GT_TRUE;
    }
    rc = gprtPortPowerDown(qd_dev, (GT_LPORT)lport, state);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtPortPowerDown failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS     rc = GT_OK;
    GT_BOOL state;


    rc = gprtGetPortPowerDown(qd_dev, (GT_LPORT)lport, &state);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtGetPortPowerDown failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    if(state == GT_TRUE)
    {
	*phy_port_state = GT_FALSE;
    }
    else
    {
    	*phy_port_state = GT_TRUE;
    }

    return rc;
}
/*******************************************************************************
* mv_switch_get_port_link_status
*
* DESCRIPTION:
*       The API return realtime port link status of an Ethernet  lport.
* INPUTS:
*       lport        - logical port  number for getting the port link status.
*
*
* OUTPUTS:
*        port_link_status  -  0:port link is ON
*				        1:port link is DOWN									.
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
    OUT GT_BOOL * port_link_status
)
{
    GT_STATUS     rc = GT_OK;
    GT_BOOL state;


    rc = gprtGetPhyLinkStatus(qd_dev, lport, &state);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtGetPhyLinkStatus failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    *port_link_status = state;

    return rc;
}

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
)
{
    GT_STATUS     rc = GT_OK;

    rc = gprtGetDuplex(qd_dev, lport, port_duplex_status);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtGetDuplex failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS     rc = GT_OK;

    rc = gprtGetSpeedMode(qd_dev, lport, speed);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtGetSpeedMode failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
)
{

    GT_STATUS     rc = GT_OK;

    rc = gprtSetPause(qd_dev, port, state);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetPause failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS     rc = GT_OK;

    rc = gprtGetPause(qd_dev, port, state);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtGetPause failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}
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
)
{
    GT_BOOL force, pause;

    if(gpcsGetForcedFC(qd_dev, port, &force) != GT_OK)
    {
	    printk("gpcsGetForcedFC failed (port %d)\n", (GT_UINT)port);
	    return GT_FAIL;
    }
    if(force)
    {
        if(gpcsGetFCValue(qd_dev, port, &pause) != GT_OK) {
	        printk("gpcsGetFCValue failed (port %d)\n", (GT_UINT)port);
	        return GT_FAIL;
        }
    }
    else
    {
	    if(gprtGetPauseEn(qd_dev, port, &pause) != GT_OK) {
	        printk("gprtGetPauseEn failed (port %d)\n", (GT_UINT)port);
	        return GT_FAIL;
        }
    }

	*state = pause;

    return GT_OK;
}
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
)
{

    GT_STATUS     rc = GT_OK;

    rc = gprtSetPortLoopback(qd_dev, port, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetPortLoopback failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}
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
)
{

    GT_STATUS     rc = GT_OK;

    rc = gprtGetPortLoopback(qd_dev, port, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtGetPortLoopback failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS     rc = GT_OK;

    rc = gprtSetPortLineLoopback(qd_dev, port, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetPortLineLoopback failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}
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
)
{

    GT_STATUS     rc = GT_OK;

    rc = gprtGetPortLineLoopback(qd_dev, port, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtGetPortLineLoopback failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}


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
)
{
    GT_STATUS     rc = GT_OK;

    rc = gprtSetPortDuplexMode(qd_dev, port, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetPortDuplexMode failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}
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
)
{
    GT_STATUS     rc = GT_OK;

    rc = gprtGetPortDuplexMode(qd_dev, port, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtGetPortDuplexMode failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}
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
)
{

    GT_STATUS     rc = GT_OK;

    rc = gprtSetPortSpeed(qd_dev, port, speed);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetPortSpeed failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
)
{

    GT_STATUS     rc = GT_OK;

    rc = gprtGetPortSpeed(qd_dev, port, speed);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==mv_switch_get_port_speed failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}


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
)
{

    GT_STATUS     rc = GT_OK;

    rc = gpcsSetForcedLink(qd_dev, port, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gpcsSetForcedLink failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}


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
)
{
    GT_STATUS     rc = GT_OK;

    rc = gpcsGetForcedLink(qd_dev, port, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gpcsGetForcedLink failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}



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
)
{

    GT_STATUS     rc = GT_OK;

    rc = gpcsSetLinkValue(qd_dev, port, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gpcsSetLinkValue failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
)
{

    GT_STATUS     rc = GT_OK;

    rc = gpcsGetLinkValue(qd_dev, port, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gpcsGetLinkValue failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}


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
)

{
   GT_STATUS rc = GT_OK;
   rc = gstatsFlushPort(qd_dev,lport);
   if (rc != GT_OK)
   {
        printk(KERN_ERR "%s:%d:==ERROR==gstatsFlushAll failed rc[0x%x]\r\n",
              __FUNCTION__, __LINE__, rc);
        rc = GT_FAIL;

   }
    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;

    rc = gstatsFlushAll(qd_dev);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gstatsFlushAll failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
	IN uint8_t  regAddr,
    IN uint16_t data
)
{
    GT_STATUS rc = GT_OK;

    rc = hwWriteGlobalReg(qd_dev, regAddr, data);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==hwWriteGlobalReg failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
    IN  uint8_t  regAddr,
    OUT uint16_t *data
)
{
    GT_STATUS rc = GT_OK;

    rc = hwReadGlobalReg(qd_dev, regAddr, data);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==hwReadGlobalReg failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
    IN  uint8_t  lport,
    IN  uint8_t  regAddr,
    IN  uint16_t data
)
{
    GT_STATUS rc = GT_OK;

    rc = hwWritePortReg(qd_dev, lport, regAddr, data);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==hwWritePortReg failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
    IN  uint8_t  lport,
    IN  uint8_t  regAddr,
    OUT uint16_t *data
)
{
    GT_STATUS rc = GT_OK;

    rc = hwReadPortReg(qd_dev, lport, regAddr, data);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==hwWritePortReg failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

/*******************************************************************************
* mv_switch_print_port_counters
*
* DESCRIPTION:
*       This function prints all counters of the given port
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
    IN  uint32_t lport
)
{
    GT_STATUS             rc = GT_OK;
    GT_STATS_COUNTER_SET3 statsCounterSet;

    rc = gstatsGetPortAllCounters3(qd_dev, lport, &statsCounterSet);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gstatsGetPortAllCounters3 failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        return GT_FAIL;
    }

    printk("=======Counters for lport[%d]=======",lport);
    printk("InGoodOctetsLo  %08ld    ", statsCounterSet.InGoodOctetsLo);
    printk("InGoodOctetsHi  %08ld   \n", statsCounterSet.InGoodOctetsHi);
    printk("InBadOctets     %08ld    ", statsCounterSet.InBadOctets);
    printk("OutFCSErr       %08ld   \n", statsCounterSet.OutFCSErr);
    printk("InUnicasts      %08ld    ", statsCounterSet.InUnicasts);
    printk("Deferred        %08ld   \n", statsCounterSet.Deferred);
    printk("InBroadcasts    %08ld    ", statsCounterSet.InBroadcasts);
    printk("InMulticasts    %08ld   \n", statsCounterSet.InMulticasts);
    printk("64Octets        %08ld    ", statsCounterSet.Octets64);
    printk("127Octets       %08ld   \n", statsCounterSet.Octets127);
    printk("255Octets       %08ld    ", statsCounterSet.Octets255);
    printk("511Octets       %08ld   \n", statsCounterSet.Octets511);
    printk("1023Octets      %08ld    ", statsCounterSet.Octets1023);
    printk("MaxOctets       %08ld   \n", statsCounterSet.OctetsMax);
    printk("OutOctetsLo     %08ld    ", statsCounterSet.OutOctetsLo);
    printk("OutOctetsHi     %08ld   \n", statsCounterSet.OutOctetsHi);
    printk("OutUnicasts     %08ld    ", statsCounterSet.OutUnicasts);
    printk("Excessive       %08ld   \n", statsCounterSet.Excessive);
    printk("OutMulticasts   %08ld    ", statsCounterSet.OutMulticasts);
    printk("OutBroadcasts   %08ld   \n", statsCounterSet.OutBroadcasts);
    printk("Single          %08ld    ", statsCounterSet.Single);
    printk("OutPause        %08ld   \n", statsCounterSet.OutPause);
    printk("InPause         %08ld    ", statsCounterSet.InPause);
    printk("Multiple        %08ld   \n", statsCounterSet.Multiple);
    printk("Undersize       %08ld    ", statsCounterSet.Undersize);
    printk("Fragments       %08ld   \n", statsCounterSet.Fragments);
    printk("Oversize        %08ld    ", statsCounterSet.Oversize);
    printk("Jabber          %08ld   \n", statsCounterSet.Jabber);
    printk("InMACRcvErr     %08ld    ", statsCounterSet.InMACRcvErr);
    printk("InFCSErr        %08ld   \n", statsCounterSet.InFCSErr);
    printk("Collisions      %08ld    ", statsCounterSet.Collisions);
    printk("Late            %08ld   \n", statsCounterSet.Late);


    rc = gstatsFlushAll(qd_dev);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gstatsFlushAll failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
    IN  uint32_t          lport,
    GT_STATS_COUNTER_SET3 *statsCounterSet
)
{
    GT_STATUS             rc = GT_OK;

    rc = gstatsGetPortAllCounters3(qd_dev, lport, statsCounterSet);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gstatsGetPortAllCounters3 failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        return GT_FAIL;
    }

    return rc;
}

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
    IN  uint32_t          lport,
    GT_PORT_STAT2         *ctr
)
{
    GT_STATUS             rc = GT_OK;

    rc = gprtGetPortCtr2(qd_dev, lport, ctr);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtGetPortCtr2 failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        return GT_FAIL;
    }

    return rc;
}


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
    IN  uint16_t db_num
)
{
    GT_STATUS    rc = GT_OK;
    GT_ATU_ENTRY atu_entry;
    GT_U8        mc_mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; /* initial MAC address */


    memcpy(atu_entry.macAddr.arEther, &mc_mac, 6);
    atu_entry.DBNum = db_num;

    while ((rc = gfdbGetAtuEntryNext(qd_dev, &atu_entry)) == GT_OK)
    {
        printk("Printing ATU Entry: db = [%d], MAC = [0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x], Port Vector = [0x%x]\n",
               atu_entry.DBNum,
               atu_entry.macAddr.arEther[0],
               atu_entry.macAddr.arEther[1],
               atu_entry.macAddr.arEther[2],
               atu_entry.macAddr.arEther[3],
               atu_entry.macAddr.arEther[4],
               atu_entry.macAddr.arEther[5],
			   (uint32_t)atu_entry.portVec);
    }

    return rc;
}
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
int32_t mv_switch_flush_vtu(void)
{
	GT_STATUS    rc = GT_OK;
	uint32_t   port;

	if ((rc = gvtuFlush(qd_dev)) != GT_OK) {
		printk("Flush VTU on the Switch failed with RC = [%d]\n", rc);
		return rc;
	}
	memset(sw_vlan_tbl,  0,  sizeof(sw_vlan_tbl));

	for(port = 0; port < qd_dev->numOfPorts; port++)
	{
		memset(&(sw_ports_tbl[port].vlan_blong), 0, sizeof(sw_ports_tbl[port].vlan_blong));
	}
	return rc;
}
/*******************************************************************************
* mv_switch_flush_atu
*
* DESCRIPTION:
*       Flush ATU on the Switch
*
* INPUTS:
*       flush_cmd     FLUSH all or FLUSH all dynamic
*       db_num        ATU DB Num, only 0 should be used, since there is only one ATU DB right now.
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
)
{
	GT_STATUS    rc = GT_OK;

	if((rc = gfdbFlushInDB(qd_dev, flush_cmd, db_num)) != GT_OK) {
		printk("Flush ATU on the Switch failed with RC = [%d]\n", rc);
		return rc;
	}

	return rc;
}

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
)
{
    GT_STATUS     rc = GT_OK;

    rc = gprtSetPortSpeedDuplexMode(qd_dev, port, speed, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gprtSetPortSpeedDuplexMode failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS     rc = GT_OK;

    rc = gpcsSetFCValue(qd_dev, port, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gpcsSetFCValue failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS     rc = GT_OK;

    rc = gpcsSetForcedFC(qd_dev, port, enable);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gpcsSetForcedFC failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

        rc = GT_FAIL;
    }

    return rc;
}

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
)
{
    GT_STATUS rc = GT_OK;
    uint32_t  port_bm;

    /* Verify whether the port is already a member of this VID */
    port_bm = (uint32_t)(1 << lport);
    if (!((sw_vlan_tbl[vid].members & port_bm) && (sw_vlan_tbl[vid].egr_mode[lport] == eMode)))
    {
        rc = mv_switch_prv_port_add_vid(lport, vid, gmac0Idx, eMode);
        if (rc != GT_OK)
        {
            printk(KERN_ERR "%s:%d:==ERROR== failed rc[0x%x]\r\n",
                   __FUNCTION__, __LINE__, rc);

            return GT_FAIL;
        }

        /* add port to vid's member bit map and set port egress mode */
        sw_vlan_tbl[vid].members |= port_bm;
        sw_vlan_tbl[vid].egr_mode[lport] = eMode;

        /* Always add GMAC_0 to VLAN */
        if (!(sw_vlan_tbl[vid].members & (1 << gmac0Idx)))
        {
            sw_vlan_tbl[vid].members |= (uint32_t)(1 << gmac0Idx);
            sw_vlan_tbl[vid].egr_mode[gmac0Idx] = MEMBER_EGRESS_UNMODIFIED;
        }
    }

    /* Add the specified port to the SW Port table */
    if (sw_ports_tbl[lport].vlan_blong[vid] == SW_PORT_NOT_BELONG)
    {
        sw_ports_tbl[lport].cnt++;
        sw_ports_tbl[lport].vlan_blong[vid] = SW_PORT_BELONG;
    }

    return rc;
}

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
)
{
    uint16_t   vid;
    uint32_t   port;
    uint32_t   i;
    GT_STATUS  rc = GT_OK;
    GT_BOOL    mode;

    memset(sw_ports_tbl, 0, sizeof(sw_ports_tbl));
    memset(sw_vlan_tbl,  0,  sizeof(sw_vlan_tbl));

    for(vid = 0; vid < SW_MAX_VLAN_NUM; vid++)
    {
        for(port = 0; port < qd_dev->numOfPorts; port++)
        {
           sw_vlan_tbl[vid].egr_mode[port] = NOT_A_MEMBER;
        }
    }

    for(port = 0; port < qd_dev->numOfPorts; port++)
    {
       sw_ports_tbl[port].port_mode = GT_DISABLE;
    }

    /* Initialize Packet Ingress Rate Limit */
    rc = gpirl2Initialize(qd_dev);
    if (rc != GT_OK)
    {
        printk(KERN_ERR
               "%s:%d:==ERROR==gpirl2Initialize failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);
    }

    gfdbGetLearn2All(qd_dev,&mode);

    printk(KERN_DEBUG "gfdbGetLearn2All mode1[%d]\n\r",mode);

    /* Enable Learn2All mode.*/
    rc = gfdbSetLearn2All(qd_dev,GT_TRUE);
    if (rc != GT_OK)
    {
        printk(KERN_ERR "%s:%d:==ERROR==gfdbSetLearn2All failed rc[0x%x]\r\n",
               __FUNCTION__, __LINE__, rc);

    }

    /* Initialize weight array with default values */
    for (i = 0; i < SW_QOS_NUM_OF_QUEUES; i++)
    {
        weight_arr[i] = MV_TPM_DEFAULT_WEIGHT;
    }

    gfdbGetLearn2All(qd_dev,&mode);

    printk(KERN_DEBUG "gfdbGetLearn2All mode2[%d]\n\r", mode);

    return;
}

