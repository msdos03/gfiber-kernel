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
#include "tpm_sysfs_setup.h"

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
extern struct semaphore    tpm_sfs_2_ioctl_sem;
extern tpm_ioctl_mng_t     tpm_sfs_2_ioctl_command;
#endif

const char *sw_qos_init_pri_str[] =
{
    "USE_PORTS_DEF",
    "TAG_PRI_ONLY",
    "IP_PRI_ONLY",
    "TAG_AND_IP_PRI",
    "UNKNOWN"
};


/* ========================================================================== */
/*                            Debug SW Attributes                                 */
/* ========================================================================== */

/*******************************************************************************
* sfs_tpm_sw_set_global_reg
*
* DESCRIPTION:
*       This function set value to the global register.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       regAddr     - The register's address.
*       data        - The data to be written.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_global_reg
(
    IN uint32_t             owner_id,
    IN uint8_t              regAddr,
    IN uint16_t             data
)
{
    tpm_sw_set_global_reg(owner_id, regAddr, data);

    return;

}

/*******************************************************************************
* sfs_tpm_sw_get_global_reg
*
* DESCRIPTION:
*           This function gets value from the global register.
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       regAddr     - The register's address.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_global_reg
(
    IN  uint32_t             owner_id,
    IN  uint8_t              regAddr
)
{
    uint16_t             data;

    tpm_sw_get_global_reg(owner_id, regAddr, &data);

    printk(KERN_INFO " regAddr[0x%x] data[0x%x]\n",regAddr,data);

    return;

}

/*******************************************************************************
* sfs_tpm_sw_set_port_reg
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
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_port_reg
(
    IN uint32_t             owner_id,
    IN uint32_t             lport,
    IN uint8_t              regAddr,
    IN uint16_t             data
)
{
    tpm_sw_set_port_reg(owner_id, lport, regAddr, data);

    return;

}

/*******************************************************************************
* sfs_tpm_sw_get_port_reg
*
* DESCRIPTION:
*           This function gets value from the port register.
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       lport       - Port number to write the register for.
*       regAddr     - The register's address.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_reg
(
    IN  uint32_t            owner_id,
    IN  uint8_t             lport,
    IN  uint8_t             regAddr
)
{
    uint16_t             data;

    tpm_sw_get_port_reg(owner_id, lport,regAddr, &data);

    printk(KERN_INFO " lport[%d] regAddr[0x%x] data[0x%x]\n",lport, regAddr, data);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_get_port_cntrs
*
* DESCRIPTION:
*           This function gets all counters of the given port
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       lport       - Port number to write the register for.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_cntrs
(
    IN  uint32_t            owner_id,
    IN  uint8_t             lport
)
{

    tpm_sw_print_port_counters(owner_id, lport);

    return;
}


/*******************************************************************************
* sfs_tpm_sw_get_fdb
*
* DESCRIPTION:
*           This function gets all counters of the given port
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       db_num      - ATU MAC Address Database number. If multiple address
*                     databases are not being used, DBNum should be zero.
*                     If multiple address databases are being used, this value
*                     should be set to the desired address database number.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_fdb
(
    IN  uint32_t            owner_id,
    IN  uint16_t            db_num
)
{

    tpm_sw_get_fdb(owner_id,db_num);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_init
*
* DESCRIPTION:
*           This function init Amber DB
* INPUTS:
*       NONE.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       There is a temprorary function.
*
*******************************************************************************/

void sfs_tpm_sw_init
(
    void
)
{
    tpm_sw_init();

    return;
}


/*******************************************************************************
* sfs_tpm_sw_set_debug_trace_flag
*
* DESCRIPTION:
*           This function sets TPM trace flag
* INPUTS:
*       NONE.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/

void sfs_tpm_sw_set_debug_trace_flag
(
    IN  uint32_t    enDis
)
{
    tpm_sw_set_debug_trace_flag(enDis);

    return;
}


/*******************************************************************************
**
**  sfs_tpm_sw_get_help_show
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print cfg help
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     message length
**
*******************************************************************************/
int sfs_tpm_sw_dbg_help_show(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "========================================================================================================================\n");
    off += sprintf(buf+off, "                                          Debug Switch Commands\n");
    off += sprintf(buf+off, "                                          Insert all parameters in Decimal\n");
    off += sprintf(buf+off, "========================================================================================================================\n");
    off += sprintf(buf+off, "cat help_sw_dbg                                                    - show this help\n");
    off += sprintf(buf+off, "echo [owner_id] [regAddr] [data]                > set_global_reg   - sets value to the global register\n");
    off += sprintf(buf+off, "echo [owner_id] [regAddr]                       > get_global_reg   - gets value of the global register\n");
    off += sprintf(buf+off, "echo [owner_id] [lport 1 -3] [regAddr]  [data]  > set_port_reg     - sets value to the port register\n");
    off += sprintf(buf+off, "echo [owner_id] [lport 1 -3] [regAddr]          > get_port_reg     - gets value of the port register\n");
    off += sprintf(buf+off, "echo [owner_id] [lport 1 -3]                    > get_port_cntrs   - gets and clear all RMON counters of the given port\n");
    off += sprintf(buf+off, "echo [owner_id] [db_num = 0 or other]           > get_fdb          - gets all FDB table.\n");
    off += sprintf(buf+off, "echo                                            > sw_init          - Init the Amber SW API DB\n");
    off += sprintf(buf+off, "echo [debug_trace 0 - 1]                        > debug_trace      - Enable/disable TPM switch trace\n");
    off += sprintf(buf+off, "========================================================================================================================\n");

    return(off);
}


static ssize_t dbg_sw_store(struct device *dev,
                         struct device_attribute *attr,
                         const char *buf, size_t len)
{
    const char* name = attr->attr.name;
//     unsigned long flags  = 0;
    unsigned int  param1 = 0;
    unsigned int  param2 = 0;
    unsigned int  param3 = 0;
    unsigned int  param4 = 0;

    if (!capable(CAP_NET_ADMIN))
    return -EPERM;

    sscanf(buf, "%d %d %d %d", &param1, &param2,&param3,&param4);

    printk(KERN_INFO "param1[%d],param2[%d],param3[%d],param4[%d]\n",
          param1,param2,param3,param4);

//     raw_local_irq_save(flags);

    if (!strcmp(name, "set_global_reg"))
        sfs_tpm_sw_set_global_reg((MV_U32)param1,(MV_U32)param2, (MV_U32)param3);

    else if (!strcmp(name, "get_global_reg"))
        sfs_tpm_sw_get_global_reg((MV_U32)param1,(MV_U32)param2);

    else if (!strcmp(name, "set_port_reg"))
        sfs_tpm_sw_set_port_reg((MV_U32)param1 ,(MV_U32)param2 ,(MV_U8)param3,(MV_U8)param4);

    else if (!strcmp(name, "get_port_reg"))
        sfs_tpm_sw_get_port_reg((MV_U32)param1 ,(MV_U32)param2 ,(MV_U8)param3);

    else if (!strcmp(name, "get_port_cntrs"))
        sfs_tpm_sw_get_port_cntrs((MV_U32)param1 ,(MV_U32)param2);

    else if (!strcmp(name, "get_fdb"))
        sfs_tpm_sw_get_fdb((MV_U32)param1 ,(MV_U32)param2);

    else if (!strcmp(name, "sw_init"))
        sfs_tpm_sw_init();

    else if (!strcmp(name, "debug_trace"))
        sfs_tpm_sw_set_debug_trace_flag((MV_U32)param1);

    else
        printk("%s: illegal operation <%s>\n", __FUNCTION__, attr->attr.name);

//     raw_local_irq_restore(flags);

    return(len);
}

/* ========================================================================== */
/*                            Info Commands                                   */
/* ========================================================================== */
#ifdef CONFIG_MV_TPM_SYSFS_HELP
static ssize_t dbg_sw_show(struct device *dev,
                        struct device_attribute *attr,
                        char *buf)
{
  const char* name = attr->attr.name;

  if (!capable(CAP_NET_ADMIN))
      return -EPERM;

  if (!strcmp(name, "help_sw_dbg"))
      return (sfs_tpm_sw_dbg_help_show(buf));

  return 0;
}
#else
static ssize_t dbg_sw_show(struct device *dev,
                           struct device_attribute *attr,
                           char *buf)
{
	printk(KERN_WARNING "%s: sysfs help is not compiled (CONFIG_MV_TPM_SYSFS_HELP)\n", __FUNCTION__);
	return 0;
}
#endif

/* ========================================================================== */
/*                            get SW Attributes                                 */
/* ========================================================================== */

/*******************************************************************************
* sfs_tpm_sw_get_port_admin
*
* DESCRIPTION:
*       This function return the port administration state, enabled or disabled.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       lport        - Packet origination.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_admin
(
    MV_U32                owner_id,
    MV_U32                lport
)
{
    bool port_state;

    tpm_phy_get_port_admin_state(owner_id, (tpm_src_port_type_t)lport, &port_state);

    printk(KERN_INFO "The administration state of lport[%d] is [%s]\n", lport, ((true==port_state)? "enabled":"disabled"));

    return;

}

/*******************************************************************************
* sfs_tpm_sw_get_port_link_status
*
* DESCRIPTION:
*       This function return the port flow control status, enabled or disabled.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       lport        - Packet origination.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_link_status
(
    MV_U32                owner_id,
    MV_U32                lport
)
{
	bool link_status;

    tpm_phy_get_port_link_status(owner_id, (tpm_src_port_type_t)lport, &link_status);

    printk(KERN_INFO "The current link status of lport[%d] is [%s]\n", lport, ((true==link_status)? "UP":"DOWN"));

    return;

}

/*******************************************************************************
* sfs_tpm_sw_get_port_autoneg
*
* DESCRIPTION:
*       This function return the port auto negotiation state and mode.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       lport        - Packet origination.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_autoneg
(
    MV_U32                owner_id,
    MV_U32                lport
)
{
    bool               autoneg_state;
    tpm_autoneg_mode_t autoneg_mode;

    tpm_phy_get_port_autoneg_mode(owner_id, (tpm_src_port_type_t)lport, &autoneg_state, &autoneg_mode);

    printk(KERN_INFO "The auto negotiation state of lport[%d] is [%s], mode[%d](0:SPEED_AUTO_DUPLEX_AUTO, 1:SPEED_1000_DUPLEX_AUTO, 2:SPEED_100_DUPLEX_AUTO, 3:SPEED_10_DUPLEX_AUTO, 4:SPEED_AUTO_DUPLEX_FULL, 5:SPEED_AUTO_DUPLEX_HALF, 6:SPEED_1000_DUPLEX_FULL, 7:SPEED_1000_DUPLEX_HALF, 8:SPEED_100_DUPLEX_FULL, 9:SPEED_100_DUPLEX_HALF, 10:SPEED_10_DUPLEX_FULL, 11:SPEED_10_DUPLEX_HALF)\n", lport, ((true==autoneg_state)? "enabled":"disabled"), autoneg_mode);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_get_port_duplex_config
*
* DESCRIPTION:
*       This function return the port duplex mode, enabled or disabled.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       lport        - Packet origination.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_duplex_config
(
    MV_U32                owner_id,
    MV_U32                lport
)
{
    bool duplex_mode;

    tpm_phy_get_port_duplex_mode(owner_id, (tpm_src_port_type_t)lport, &duplex_mode);

    printk(KERN_INFO "The duplex configuration of lport[%d] is [%s]\n", lport, ((true==duplex_mode)? "enabled":"disabled"));

    return;

}

/*******************************************************************************
* sfs_tpm_sw_get_port_duplex_status
*
* DESCRIPTION:
*       This function return the port duplex current status, enabled or disabled.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       lport        - Packet origination.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_duplex_status
(
    MV_U32                owner_id,
    MV_U32                lport
)
{
    bool duplex_status;

    tpm_phy_get_port_duplex_status(owner_id, (tpm_src_port_type_t)lport, &duplex_status);

    printk(KERN_INFO "The duplex status of lport[%d] is [%s]\n", lport, ((true==duplex_status)? "enabled":"disabled"));

    return;

}

/*******************************************************************************
* sfs_tpm_sw_get_port_fc_config
*
* DESCRIPTION:
*       This function return the port flow control configuration state, enabled or disabled.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       lport        - Packet origination.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_fc_config
(
    MV_U32                owner_id,
    MV_U32                lport
)
{
    bool fc_state;

    tpm_phy_get_port_flow_control_support(owner_id, (tpm_src_port_type_t)lport, &fc_state);

    printk(KERN_INFO "The flow control configuration of lport[%d] is [%s]\n", lport, ((true==fc_state)? "enabled":"disabled"));

    return;

}

/*******************************************************************************
* sfs_tpm_sw_get_port_fc_status
*
* DESCRIPTION:
*       This function return the port flow control status, enabled or disabled.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       lport        - Packet origination.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_fc_status
(
    MV_U32                owner_id,
    MV_U32                lport
)
{
    bool fc_state;

    tpm_phy_get_port_flow_control_state(owner_id, (tpm_src_port_type_t)lport, &fc_state);

    printk(KERN_INFO "The flow control status of lport[%d] is [%s]\n", lport, ((true==fc_state)? "enabled":"disabled"));

    return;

}

/*******************************************************************************
* sfs_tpm_sw_get_port_speed_config
*
* DESCRIPTION:
*       This function return the port speed configuration state, enabled or disabled.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       lport        - Packet origination.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_speed_config
(
    MV_U32                owner_id,
    MV_U32                lport
)
{
    tpm_phy_speed_t speed;

    tpm_phy_get_port_speed(owner_id, (tpm_src_port_type_t)lport, &speed);

    printk(KERN_INFO "The speed configuration of lport[%d] is [%d]('0'-10M, '1'-100M, '2'-1000M)\n", lport, speed);

    return;

}

/*******************************************************************************
* sfs_tpm_sw_get_port_speed_status
*
* DESCRIPTION:
*       This function return the port speed status, enabled or disabled.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       lport        - Packet origination.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_speed_status
(
    MV_U32                owner_id,
    MV_U32                lport
)
{
    MV_U32   speed;

    tpm_phy_get_port_speed_mode(owner_id, (tpm_src_port_type_t)lport, &speed);

    printk(KERN_INFO "The speed status of lport[%d] is [%d]('0'-10M, '1'-100M, '2'-1000M, '3'-UNKOWN)\n", lport, speed);

    return;

}

/*******************************************************************************
* sfs_tpm_sw_get_port_isolate_vector
*
* DESCRIPTION:
*       This function return the port isolation vector, enabled or isolated.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       lport        - Packet origination.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_isolate_vector
(
    MV_U32                owner_id,
    MV_U32                lport
)
{
    MV_U32   vector;

    tpm_sw_get_isolate_eth_port_vector(owner_id, (tpm_src_port_type_t)lport, &vector);

    printk(KERN_INFO "The isolation vector of lport[%d] is [0x%02x]('0'-isolate, '1'-enable, bit0-UNI0, bit1-UNI1...)\n", lport, vector);

    return;

}

/*******************************************************************************
* sfs_tpm_sw_get_port_loopback
*
* DESCRIPTION:
*       This function return the port loopback status, enabled or disabled.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       lport        - Packet origination.
*       mode         - 0:internal loopback, 1:external loopback.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_loopback
(
    MV_U32                owner_id,
    MV_U32                lport,
    MV_U32                mode
)
{
    tpm_phy_loopback_mode_t loopback_mode;
    bool                    status;

    if(mode > TPM_PHY_EXTERNAL_LOOPBACK)
        loopback_mode = TPM_PHY_EXTERNAL_LOOPBACK;
    else
        loopback_mode = (tpm_phy_loopback_mode_t)mode;

    tpm_phy_get_port_loopback(owner_id, (tpm_src_port_type_t)lport, loopback_mode, &status);

    printk(KERN_INFO "The [%d]('0'-internal, '1'-external) loopback status of lport[%d] is [%s]\n", loopback_mode, lport, ((true==status)? "enabled":"disabled"));

    return;

}

/*******************************************************************************
* sfs_tpm_sw_set_mac_age_time
*
* DESCRIPTION:
*       The API Configures the egress frame rate limit of an Ethernet UNI lport
* INPUTS:
*       owner_id            - APP owner id - should be used for all API calls.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*      None
*
*******************************************************************************/
void sfs_tpm_sw_get_mac_age_time
(
    IN uint32_t                 owner_id
)
{
    MV_U32      time_out;

    tpm_sw_get_mac_age_time(owner_id, &time_out);

    printk(KERN_INFO "The time out is [%d]\n",time_out);

     return;
}

/*******************************************************************************
* sfs_tpm_sw_get_port_max_macs
*
* DESCRIPTION:
*       This function return the limit number of MAC addresses per lport.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       lport         - Packet origination.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_max_macs
(
    MV_U32                owner_id,
    MV_U32                lport
)
{
    uint32_t               limit;

    tpm_sw_get_port_max_macs(owner_id, (tpm_src_port_type_t)lport, &limit);

    printk(KERN_INFO "The limit number of MAC addresses per lport[%d] is [%d]\n",lport,limit);

    return;

}


/*******************************************************************************
* sfs_tpm_sw_get_port_mac_learn
*
* DESCRIPTION:
*       This function return MAC learning state.
*
* INPUTS:
*       owner_id      - APP owner id - should be used for all API calls.
*       lport         - Packet origination.
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_mac_learn
(
    MV_U32      owner_id,
    MV_U32      lport
)
{
    bool   status;

    tpm_sw_get_mac_learn(owner_id, (tpm_src_port_type_t)lport, &status);

    printk(KERN_INFO "The MAC learning status of lport[%d] is [%s]\n", lport, ((true==status)? "enabled":"disabled"));

    return;

}

/*******************************************************************************
* sfs_tpm_sw_get_port_flooding
*
* DESCRIPTION:
*       This function gets Forward Unknown mode of a switch lport.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       egress_port     - Egress lport for applying the flood settings.

*
* OUTPUTS:
*       mode  - NONE.
*
* RETURNS:
*      None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_flooding
(
    MV_U32           owner_id,
    MV_U32           lport
)
{
    uint8_t          mode;

    tpm_sw_get_port_flooding(owner_id, (tpm_src_port_type_t)lport, TPM_FLOOD_UNKNOWN_UNICAST, &mode);

    printk(KERN_INFO "The Forward Unknown mode of a switch lport[%d] is [%d]\n",lport,mode);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_get_port_mc_flooding
*
* DESCRIPTION:
*       This function gets Forward Unknown Multicast mode of a switch lport.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       egress_port     - Egress lport for applying the flood settings.

*
* OUTPUTS:
*       mode  - NONE.
*
* RETURNS:
*      None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_mc_flooding
(
    MV_U32           owner_id,
    MV_U32           lport
)
{
    uint8_t          mode;

    tpm_sw_get_port_flooding (owner_id, (tpm_src_port_type_t)lport, TPM_FLOOD_UNKNOWN_MULTI_BROAD_CAST,&mode);

    printk(KERN_INFO "The Forward Unknown Multicast mode of a switch lport[%d] is [%d]\n",lport,mode);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_get_bc_flooding
*
* DESCRIPTION:
*       This function gets Forward Broadcast mode of a switch lport.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       egress_port     - Egress lport for applying the flood settings.

*
* OUTPUTS:
*       mode  - NONE.
*
* RETURNS:
*      None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_bc_flooding
(
    MV_U32           owner_id
)
{

    bool  mode;

    tpm_sw_get_broadcast_flood(owner_id, &mode);

    printk(KERN_INFO "The Broadcast flooding is [%s]\n",(mode==true)?"allowed":"forbidden");

    return;
}

/*******************************************************************************
* sfs_tpm_sw_get_port_tagged
*
* DESCRIPTION:
*       The API gets DiscardTagged bit for the given lport
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       lport         -  lport for applying the filtering of tagged packets.
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*      None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_tagged
(
    MV_U32    owner_id,
    MV_U32    lport
)
{

    uint32_t     mode;

    tpm_sw_get_port_tagged(owner_id, (tpm_src_port_type_t)lport, &mode);

    printk(KERN_INFO "The DiscardTagged bit for the given lport[%d] is [%d]\n",lport, mode);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_get_port_untagged
*
* DESCRIPTION:
*       The API  gets DiscardUntagged bit for the given lport.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       lport         -  lport for applying the filtering of tagged packets.

*
* OUTPUTS:
*       NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_untagged
(
    MV_U32    owner_id,
    MV_U32    lport
)
{

    uint32_t     mode;

    tpm_sw_get_port_untagged(owner_id, (tpm_src_port_type_t)lport, &mode);

    printk(KERN_INFO "The DiscardUnTagged bit for the given lport[%d] is [%d]\n",lport, mode);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_get_port_def_vlan
*
* DESCRIPTION:
*       The API gets port default vlan id.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       lport       - lport for applying the filtering of tagged packets.
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
void sfs_tpm_sw_get_port_def_vlan
(
    MV_U32    owner_id,
    MV_U32    lport
)
{
    MV_U16    vid;
    tpm_sw_get_port_def_vlan(owner_id,(tpm_src_port_type_t)lport,&vid);
    printk(KERN_INFO "The default vlan id of port[%d] is [%d]\n",lport,vid);
    return;
}

/*******************************************************************************
* sfs_tpm_sw_get_port_def_pri
*
* DESCRIPTION:
*       The API gets port default priority.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       lport       - lport for applying the filtering of tagged packets.
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
void sfs_tpm_sw_get_port_def_pri
(
    MV_U32    owner_id,
    MV_U32    lport
)
{
    MV_U8     pri;

    tpm_sw_get_port_def_pri(owner_id,(tpm_src_port_type_t)lport,&pri);
    printk(KERN_INFO "The default priority of port[%d] is [%d]\n",lport,pri);
    return;
}

/*******************************************************************************
* sfs_tpm_sw_get_port_vid
*
* DESCRIPTION:
*       The API look for vid in VTU DB.
*
* INPUTS:
*       owner_id - APP owner id - should be used for all API calls.
*       vid     -  lport for adding the vid.

*
* OUTPUTS:
*       NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_port_vid
(
    MV_U32    owner_id,
    MV_U32    vid
)
{

  MV_U32 found;

  tpm_sw_port_get_vid(owner_id, (tpm_src_port_type_t)vid ,&found);
}

/*******************************************************************************
* sfs_tpm_sw_get_port_vid_filter
*
* DESCRIPTION:
*        This routine gets protected mode of a switch lport.
*        When this mode is set to GT_TRUE, frames are allowed to egress lport
*        defined by the 802.1Q VLAN membership for the frame's VID 'AND'
*        by the lport's VLANTable if 802.1Q is enabled on the lport. Both must
*        allow the frame to Egress.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       lport        -  lport for setting the filtering mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        NONE
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void  sfs_tpm_sw_get_port_vid_filter
(
    MV_U32    owner_id,
    MV_U32    lport
)
{

    uint32_t     mode;

    tpm_sw_get_vid_filter_per_port(owner_id, (tpm_src_port_type_t)lport, &mode);

    printk(KERN_INFO "The protected mode of a switch lport[%d] is [%d]\n", lport, mode);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_get_uni_ingr_police_rate
*
* DESCRIPTION:
*        This routine retrieves the port's ingress data limit based on burst size.
*
* INPUTS:
*       owner_id - APP owner id - should be used for all API calls.
*       uni_port - uni lport for configuring the ingress policer function.

*
* OUTPUTS:
*       NONE.
* RETURNS:
*       On success -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_uni_ingr_police_rate
(
    MV_U32      owner_id,
    MV_U32      uni_port
)
{
    MV_U32  count_mode;
    MV_U32  cir;
    MV_U32  cbs;
    MV_U32  ebs;

    tpm_sw_get_uni_ingr_police_rate(owner_id,(tpm_src_port_type_t)uni_port,&count_mode,&cir, &cbs, &ebs);

    printk(KERN_INFO "The uni_port[%d] ingress data limit mode[%d], cir[%d] cbs[%d] ebs[%d]\r\n",
           uni_port, count_mode, cir, cbs, ebs);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_get_uni_tc_ingr_police_rate
*
* DESCRIPTION:
*       The API Configures a policer function for a traffic class for an Ethernet UNI lport.
*       There are 4 globally defined traffic classes in the integrated switch.
*
* INPUTS:
*       owner_id - APP owner id - should be used for all API calls.
*       uni_port - uni lport for configuring the ingress policer function.
*       tc       - traffic class ( a combination of p-bits and DSCP values).
*
* OUTPUTS:
*       NONE.
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_get_uni_tc_ingr_police_rate
(
    MV_U32      owner_id,
    MV_U32      uni_port

)
{
	uint32_t	cir[5];
	uint32_t	tc[5];
	uint32_t	cbs[5];
	uint32_t	i;

	tpm_sw_get_uni_tc_ingr_police_rate(owner_id,(tpm_src_port_type_t)uni_port,tc,cir,cbs);

	printk(KERN_INFO "The uni_port[%d]: \n",uni_port);
	for(i = 0; i < 5; i ++) {
		if(tc[i])
			printk(KERN_INFO "                ingress data limit[%d] for tc[0x%x]\n", cir[i] ,tc[i]);
	}

	return;
}

/*******************************************************************************
* sfs_tpm_sw_get_uni_egr_rate_limit
*
* DESCRIPTION:
*       The API return the egress frame rate limit of an Ethernet UNI lport
* INPUTS:
*       owner_id            - APP owner id - should be used for all API calls.
*       trg_lport            - uni lport for configuring the egress rate limit.

*
* OUTPUTS:
*       NONE.
* RETURNS:
*       None
*
* COMMENTS:
*      None
*
*******************************************************************************/
void sfs_tpm_sw_get_uni_egr_rate_limit
(
    MV_U32              owner_id,
    MV_U32              trg_lport
)
{
    tpm_limit_mode_t  mode;
    MV_U32            frame_rate_limit_val;

    tpm_sw_get_uni_egr_rate_limit(owner_id,(tpm_src_port_type_t)trg_lport,&mode, &frame_rate_limit_val);

    printk(KERN_INFO "The egress frame rate limit is [%d] mode[%d] for trg_lport[%d]\n",
           frame_rate_limit_val, mode, trg_lport);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_get_port_vlan_ports
*
* DESCRIPTION:
*           This routine gets the port VLAN group port membership list.
* INPUTS:
*       owner_id            - APP owner id - should be used for all API calls.
*       lport               - logical port
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*      None
*
*******************************************************************************/
void sfs_tpm_sw_get_port_vlan_ports
(
    IN uint32_t                 owner_id,
    IN uint32_t                 lport
)
{
   uint32_t             memPorts[TPM_MAX_NUM_PORTS];
   uint8_t              memPortsLen;
   uint32_t             i;

   memset(memPorts, 0, sizeof(GT_LPORT)*TPM_MAX_NUM_PORTS);

   tpm_sw_get_port_vlan_ports(owner_id, (tpm_src_port_type_t)lport, memPorts, &memPortsLen);

   for (i = 0; i < memPortsLen; i++)
   {
       if (memPorts[i] > 0)
       {
           printk("memPorts[%d] = %02d\n\r",i,memPorts[i]);
       }

   }

     return;
}


/*******************************************************************************
* sfs_tpm_sw_get_ingress_limit_mode
*
* DESCRIPTION:
*           This routine gets the port's rate control ingress limit mode.
* INPUTS:
*       owner_id            - APP owner id - should be used for all API calls.
*       lport               - logical port
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*      None
*
*******************************************************************************/
void sfs_tpm_sw_get_ingress_limit_mode
(
    uint32_t             owner_id,
    uint32_t             lport
)
{
    GT_RATE_LIMIT_MODE   mode;


   tpm_sw_get_ingress_limit_mode(owner_id, (tpm_src_port_type_t)lport, &mode);

   printk("lport [%d] mode[%d]\n\r",lport,mode);

   return;
}

/*******************************************************************************
* sfs_tpm_sw_get_priority_selection
*
* DESCRIPTION:
*       This function gets initial QPri and FPri selection.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       lport       - Port number to set init pri.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
* COMMENTS:
*       PRI_SEL_USE_PORTS_DEF  - 0
*       PRI_SEL_TAG_PRI_ONLY   - 1
*       PRI_SEL_IP_PRI_ONLY    - 2
*       PRI_SEL_TAG_AND_IP_PRI - 3
*
*******************************************************************************/
void sfs_tpm_sw_get_priority_selection
(
    uint32_t owner_id,
    uint8_t  lport
)
{
    uint8_t init_pri;
    bool    tag_if_both;

    tpm_sw_get_priority_selection(owner_id,(tpm_src_port_type_t)lport,&init_pri,&tag_if_both);

    printk("UNI port[%d] init_pri[%s] tag_if_both[%s]\n\r",lport,
            sw_qos_init_pri_str[init_pri < PRI_SEL_TYPE_MAX ? init_pri : PRI_SEL_TYPE_MAX], (tag_if_both==true)?"TRUE":"FALSE");
}

/*******************************************************************************
* sfs_tpm_sw_get_tag_pri_mapping
*
* DESCRIPTION:
*       This function gets tag priority mapping.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       tag_pri     - Source tag priority number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
* COMMENTS:
*       tag_pri 0-7.
*       q_pri   0-3.
*
*******************************************************************************/
void sfs_tpm_sw_get_tag_pri_mapping
(
    IN  uint32_t owner_id,
    IN  uint8_t  tag_pri
)
{
    uint8_t q_pri;

    tpm_sw_get_tag_pri_mapping(owner_id,tag_pri,&q_pri);

    printk("tag_pri[%d] -> q_pri[%d]\n\r",tag_pri,q_pri);
}

/*******************************************************************************
* sfs_tpm_sw_get_ip_pri_mapping
*
* DESCRIPTION:
*       This function gets the queue priority for a dscp value mapping to.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       dscp        - Source dscp value.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
* COMMENTS:
*       dscp    0-63.
*       q_pri   0-3.
*
*******************************************************************************/
void sfs_tpm_sw_get_ip_pri_mapping
(
    IN  uint32_t owner_id,
    IN  uint8_t  dscp
)
{
    uint8_t q_pri;

    tpm_sw_get_ip_pri_mapping(owner_id,dscp,&q_pri);

    printk("dscp[%d] -> q_pri[%d]\n\r",dscp,q_pri);
}

/*******************************************************************************
* sfs_tpm_sw_get_gmac_mtu
*
* DESCRIPTION:
*       This function gets the MTU of GMAC.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       gmac        - GMAC, '0'-GMAC0, '1'-GMAC1, '02'-PONMAC, '3'-switch.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
*
*******************************************************************************/
void sfs_tpm_sw_get_gmac_mtu
(
    IN  uint32_t  owner_id,
    IN  uint32_t  gmac
)
{
    tpm_mru_type_t type;
    uint32_t       mtu;

    if(gmac > TPM_NETA_MTU_SWITCH)
        type = TPM_NETA_MTU_SWITCH;
    else
        type = (tpm_mru_type_t)gmac;

    tpm_get_mtu_size(owner_id, type, &mtu);

    printk("The MTU of GMAC[%d]('0'-GMAC0, '1'-GMAC1, '02'-PONMAC, '3'-switch) is [%d]\n\r", gmac, mtu);
}

/*******************************************************************************
* sfs_tpm_sw_get_port_mirror
*
* DESCRIPTION:
*       This function gets port mirror state.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       src_port    - src UNI port(1-4).
*       dst_port    - dst UNI port(1-4).
*       mode        - mirror mode, '0'-ingress mirror, '1'-egress mirror.
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
*
*******************************************************************************/
void sfs_tpm_sw_get_port_mirror
(
    IN  uint32_t  owner_id,
    IN  uint32_t  src_port,
    IN  uint32_t  dst_port,
    IN  uint32_t  mode
)
{
    tpm_sw_mirror_type_t mirror_mode;
    bool                 mirror_state;

    if(mode > TPM_SW_MIRROR_EGRESS)
        mirror_mode = TPM_SW_MIRROR_EGRESS;
    else
        mirror_mode = (tpm_sw_mirror_type_t)mode;

    tpm_sw_get_port_mirror(owner_id, src_port, dst_port, mirror_mode, &mirror_state);

    printk("The src_port[%d] dst_port[%d] mode[%d]('0'-ingress, '1'-egress) mirror status is [%s]\n\r", src_port, dst_port, mirror_mode, ((true==mirror_state)? "enabled":"disabled"));
}

#ifdef CONFIG_MV_TPM_SYSFS_HELP
/*******************************************************************************
**
**  sfs_tpm_sw_get_help_show
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print cfg help
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     message length
**
*******************************************************************************/
int sfs_tpm_sw_get_help_show(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "========================================================================================================================\n");
    off += sprintf(buf+off, "                                          Configuration Switch Commands\n");
    off += sprintf(buf+off, "                                          Insert all parameters in Decimal\n");
    off += sprintf(buf+off, "========================================================================================================================\n");
    off += sprintf(buf+off, "cat help_sw_get_info                                                  - show this help\n");
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_admin              - get port administration state\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_link_status        - get port current link status\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_autoneg            - get port auto negotiation state and mode\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_duplex_config      - get port duplex configuration\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_duplex_status      - get port duplex current status\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_fc_config          - get port flow control configuration\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_fc_status          - get port flow control current status\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_speed_config       - get port speed configuration\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_speed_status       - get port speed current status\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_isolate_vector     - get port isolation vector\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [mode]   > get_port_loopback           - get port loopback status, mode('0'-internal, '1'-external)\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id]                         > get_mac_age_time            - get the MAC aging time\n");
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_max_macs           - get port maximum number of allowed MAC address to be learned\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_mac_learn          - get port MAC learning state, disabled or enabled\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_flooding           - get Forward Unknown mode\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_mc_flooding        - get Forward Unknown mode\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id]                         > get_bc_flooding             - get Forward Unknown mode\n");
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_tagged             - get DiscardTagged bits\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_untagged           - get DiscardUntagged bit\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_vid_filter         - get the filtering mod VID\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_def_vlan           - get port default vlan\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_def_pri            - get port default priority\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [vid 0-4095]            > get_port_vid                - get port VID if it exist\n");
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_uni_ingr_police_rate    - get the port's ingress data limit by burst size\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_uni_tc_ingr_police_rate - get the port's ingress data limit for tc\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_uni_egr_rate_limit      - get the port's egress frame rate limit\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_port_vlan_ports         - get the port VLAN group port membership list\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]          > get_init_pri                - get initial priority\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [pri 0-7]               > get_tag_pri_mapping         - get tag pri mapping\n");
    off += sprintf(buf+off, "echo [owner_id] [dscp 0-63]             > get_ip_pri_mapping          - get ip pri mapping\n");
    off += sprintf(buf+off, "echo [owner_id] [gmac]                  > get_gmac_mtu                - get GMAC MTU ('0'-GMAC0, '1'-GMAC1, '2'-PMAC, '3'-switch)\n");
    off += sprintf(buf+off, "echo [owner_id] [sport][dport][mode]    > get_port_mirror             - get port mirro, src port/dst port(uni port 1-4), mode('0'-ingress, '1'-egress)\n");
    off += sprintf(buf+off, "========================================================================================================================\n");

    return(off);
}
#endif

/* ========================================================================== */
/*                            Info Group                                      */
/* ========================================================================== */
/******************************************************************************/

/* ========================================================================== */
/*                            Info Commands                                   */
/* ========================================================================== */
#ifdef CONFIG_MV_TPM_SYSFS_HELP
static ssize_t get_sw_show(struct device *dev,
                        struct device_attribute *attr,
                        char *buf)
{
  const char* name = attr->attr.name;

  if (!capable(CAP_NET_ADMIN))
      return -EPERM;

  if (!strcmp(name, "help_sw_get_info"))
      return (sfs_tpm_sw_get_help_show(buf));

  return 0;
}
#else
static ssize_t get_sw_show(struct device *dev,
                           struct device_attribute *attr,
                           char *buf)
{
	printk(KERN_WARNING "%s: sysfs help is not compiled (CONFIG_MV_TPM_SYSFS_HELP)\n", __FUNCTION__);
	return 0;
}
#endif

static ssize_t get_sw_store(struct device *dev,
                             struct device_attribute *attr,
                             const char *buf, size_t len)
{
    const char* name = attr->attr.name;
//     unsigned long flags;
    unsigned int  param1 = 0;
    unsigned int  param2 = 0;
    unsigned int  param3 = 0;
    unsigned int  param4 = 0;

    if (!capable(CAP_NET_ADMIN))
    return -EPERM;

    if (!strcmp(name, "get_port_mirror"))
    {
        sscanf(buf, "%d %d %d %d",
               &param1, &param2, &param3, &param4);
    }
    else
    {
        sscanf(buf, "%d %d %d", &param1, &param2, &param3);
    }

    printk(KERN_INFO "param1[%d],param2[%d],param3[%d]\n",param1,param2,param3);

//     raw_local_irq_save(flags);

    if (!strcmp(name, "get_port_admin"))
        sfs_tpm_sw_get_port_admin((MV_U32)param1, (MV_U32)param2);

    else if (!strcmp(name, "get_port_link_status"))
        sfs_tpm_sw_get_port_link_status((MV_U32)param1, (MV_U32)param2);

    else if (!strcmp(name, "get_port_autoneg"))
        sfs_tpm_sw_get_port_autoneg((MV_U32)param1, (MV_U32)param2);

    else if (!strcmp(name, "get_port_duplex_config"))
        sfs_tpm_sw_get_port_duplex_config((MV_U32)param1, (MV_U32)param2);

    else if (!strcmp(name, "get_port_duplex_status"))
        sfs_tpm_sw_get_port_duplex_status((MV_U32)param1, (MV_U32)param2);

    else if (!strcmp(name, "get_port_fc_config"))
        sfs_tpm_sw_get_port_fc_config((MV_U32)param1, (MV_U32)param2);

    else if (!strcmp(name, "get_port_fc_status"))
        sfs_tpm_sw_get_port_fc_status((MV_U32)param1, (MV_U32)param2);

    else if (!strcmp(name, "get_port_speed_config"))
        sfs_tpm_sw_get_port_speed_config((MV_U32)param1, (MV_U32)param2);

    else if (!strcmp(name, "get_port_speed_status"))
        sfs_tpm_sw_get_port_speed_status((MV_U32)param1, (MV_U32)param2);

    else if (!strcmp(name, "get_port_isolate_vector"))
        sfs_tpm_sw_get_port_isolate_vector((MV_U32)param1, (MV_U32)param2);

    else if (!strcmp(name, "get_port_loopback"))
        sfs_tpm_sw_get_port_loopback((MV_U32)param1, (MV_U32)param2, (MV_U32)param3);

    else if (!strcmp(name, "get_port_max_macs"))
        sfs_tpm_sw_get_port_max_macs((MV_U32)param1, (MV_U32)param2);

    else if (!strcmp(name, "get_port_mac_learn"))
        sfs_tpm_sw_get_port_mac_learn((MV_U32)param1, (MV_U32)param2);

    else if (!strcmp(name, "get_port_flooding"))
        sfs_tpm_sw_get_port_flooding((MV_U32)param1, (MV_U32)param2);

    else if (!strcmp(name, "get_port_mc_flooding"))
        sfs_tpm_sw_get_port_mc_flooding((MV_U32)param1, (MV_U32)param2);

    else if (!strcmp(name, "get_bc_flooding"))
        sfs_tpm_sw_get_bc_flooding((MV_U32)param1);

    else if (!strcmp(name, "get_port_tagged"))
        sfs_tpm_sw_get_port_tagged((MV_U32)param1 ,(MV_U32)param2);

    else if (!strcmp(name, "get_port_untagged"))
      sfs_tpm_sw_get_port_untagged((MV_U32)param1 ,(MV_U32)param2);

    else if (!strcmp(name, "get_port_def_vlan"))
      sfs_tpm_sw_get_port_def_vlan((MV_U32)param1 ,(MV_U32)param2);

    else if (!strcmp(name, "get_port_def_pri"))
        sfs_tpm_sw_get_port_def_pri((MV_U32)param1 ,(MV_U32)param2);

    else if (!strcmp(name, "get_port_vid"))
        sfs_tpm_sw_get_port_vid((MV_U32)param1 ,(MV_U32)param2);

    else if (!strcmp(name, "get_port_vid_filter"))
      sfs_tpm_sw_get_port_vid_filter((MV_U32)param1 ,(MV_U32)param2);

    else if (!strcmp(name, "get_uni_ingr_police_rate"))
        sfs_tpm_sw_get_uni_ingr_police_rate((MV_U32)param1, (MV_U32)param2);

    else if (!strcmp(name, "get_uni_tc_ingr_police_rate"))
        sfs_tpm_sw_get_uni_tc_ingr_police_rate((MV_U32)param1, (MV_U32)param2);

    else if (!strcmp(name, "get_uni_egr_rate_limit"))
        sfs_tpm_sw_get_uni_egr_rate_limit((MV_U32)param1, (MV_U32)param2);

    else if (!strcmp(name, "get_mac_age_time"))
        sfs_tpm_sw_get_mac_age_time((MV_U32)param1);

    else if (!strcmp(name, "get_port_vlan_ports"))
        sfs_tpm_sw_get_port_vlan_ports((MV_U32)param1,(MV_U32)param2);

    else if (!strcmp(name, "get_port_vlan_ports"))
        sfs_tpm_sw_get_port_vlan_ports((MV_U32)param1,(MV_U32)param2);

    else if (!strcmp(name, "get_ingress_limit_mode"))
        sfs_tpm_sw_get_ingress_limit_mode((MV_U32)param1,(MV_U32)param2);

    else if (!strcmp(name, "get_init_pri"))
        sfs_tpm_sw_get_priority_selection((MV_U32)param1,(MV_U8)param2);

    else if (!strcmp(name, "get_tag_pri_mapping"))
        sfs_tpm_sw_get_tag_pri_mapping((MV_U32)param1,(MV_U8)param2);

    else if (!strcmp(name, "get_ip_pri_mapping"))
        sfs_tpm_sw_get_ip_pri_mapping((MV_U32)param1,(MV_U8)param2);

    else if (!strcmp(name, "get_gmac_mtu"))
        sfs_tpm_sw_get_gmac_mtu((MV_U32)param1,(MV_U8)param2);

    else if (!strcmp(name, "get_port_mirror"))
        sfs_tpm_sw_get_port_mirror((MV_U32)param1, (MV_U32)param2, (MV_U32)param3, (MV_U32)param4);

    else
        printk("%s: illegal operation <%s>\n", __FUNCTION__, attr->attr.name);

//     raw_local_irq_restore(flags);

    return(len);
}

/* ========================================================================== */
/*                            Set SW Attributes                                 */
/* ========================================================================== */

/*******************************************************************************
* sfs_tpm_sw_set_port_admin
*
* DESCRIPTION:
*       This function set the port administration state
*
* INPUTS:
*       owner_id      - APP owner id - should be used for all API calls.
*       lport         - Packet origination.
*       state         - port administration state, 1:enable, 0:disable.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_port_admin
(
    MV_U32               owner_id,
    MV_U32               lport,
    MV_U32               state
)
{
    bool admin_state;

    admin_state = ((1 == state) ? true: false);
    tpm_phy_set_port_admin_state(owner_id, (tpm_src_port_type_t)lport, admin_state);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_port_autoneg
*
* DESCRIPTION:
*       This function set the port auto negotiation
*
* INPUTS:
*       owner_id      - APP owner id - should be used for all API calls.
*       lport         - Packet origination.
*       state         - port auto negotiation  state, 1:enable, 0:disable.
*       mode          - port auto negotiation  mode:
*                       Auto for both speed and duplex.
*                       Auto for speed only and Full duplex.
*                       Auto for speed only and Half duplex. (1000Mbps is not supported)
*                       Auto for duplex only and speed 1000Mbps.
*                       Auto for duplex only and speed 100Mbps.
*                       Auto for duplex only and speed 10Mbps.
*                       1000Mbps Full duplex.
*                       100Mbps Full duplex.
*                       100Mbps Half duplex.
*                       10Mbps Full duplex.
*                       10Mbps Half duplex.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_port_autoneg
(
    MV_U32               owner_id,
    MV_U32               lport,
    MV_U32               state,
    MV_U32               mode
)
{
    bool               autoneg_state;
    tpm_autoneg_mode_t autoneg_mode;

    autoneg_state = ((1 == state) ? true: false);

    autoneg_mode = ((mode > TPM_SPEED_10_DUPLEX_HALF ) ? TPM_SPEED_10_DUPLEX_HALF: (tpm_autoneg_mode_t)mode);
    tpm_phy_set_port_autoneg_mode(owner_id, (tpm_src_port_type_t)lport, autoneg_state, autoneg_mode);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_restart_autoneg
*
* DESCRIPTION:
*       This function restart the port auto negotiation
*
* INPUTS:
*       owner_id      - APP owner id - should be used for all API calls.
*       lport         - Packet origination.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_restart_autoneg
(
    MV_U32               owner_id,
    MV_U32               lport
)
{
    tpm_phy_restart_port_autoneg(owner_id, (tpm_src_port_type_t)lport);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_port_duplex
*
* DESCRIPTION:
*       This function set the port duplex mode
*
* INPUTS:
*       owner_id      - APP owner id - should be used for all API calls.
*       lport         - Packet origination.
*       mode          - port duplex mode, 1:enable, 0:disable.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_port_duplex
(
    MV_U32               owner_id,
    MV_U32               lport,
    MV_U32               mode
)
{
    bool duplex_mode;

    duplex_mode = ((1 == mode) ? true: false);
    tpm_phy_set_port_duplex_mode(owner_id, (tpm_src_port_type_t)lport, duplex_mode);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_port_fc
*
* DESCRIPTION:
*       This function set the port flow control
*
* INPUTS:
*       owner_id      - APP owner id - should be used for all API calls.
*       lport         - Packet origination.
*       state         - port flow control configuration, 1:enable, 0:disable.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_port_fc
(
    MV_U32      owner_id,
    MV_U32      lport,
    MV_U32      state
)
{
    bool fc_state;

    fc_state = ((1 == state) ? true: false);
    tpm_phy_set_port_flow_control_support(owner_id, (tpm_src_port_type_t)lport, fc_state);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_port_speed
*
* DESCRIPTION:
*       This function set the port speed
*
* INPUTS:
*       owner_id      - APP owner id - should be used for all API calls.
*       lport         - Packet origination.
*       speed_mode    - speed mode, '0'-10M, '1'-100M, '2'-1000M.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_port_speed
(
    MV_U32               owner_id,
    MV_U32               lport,
    MV_U32               speed_mode
)
{
    tpm_phy_speed_t speed;

    if((tpm_phy_speed_t)speed_mode > TPM_PHY_SPEED_1000_MBPS)
        speed = (tpm_phy_speed_t)TPM_PHY_SPEED_1000_MBPS;
    else
        speed = (tpm_phy_speed_t)speed_mode;

    tpm_phy_set_port_speed(owner_id, (tpm_src_port_type_t)lport, speed);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_port_isolate_vector
*
* DESCRIPTION:
*       This function set the port isolation vector
*
* INPUTS:
*       owner_id      - APP owner id - should be used for all API calls.
*       lport         - Packet origination.
*       vector        - '0'-isolate, '1'-enable, bit0-UNI0, bit1-UNI1...
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_port_isolate_vector
(
    MV_U32               owner_id,
    MV_U32               lport,
    MV_U32               vector
)
{
    tpm_sw_set_isolate_eth_port_vector(owner_id, (tpm_src_port_type_t)lport, vector);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_port_loopback
*
* DESCRIPTION:
*       This function set the port loopback
*
* INPUTS:
*       owner_id      - APP owner id - should be used for all API calls.
*       lport         - Packet origination.
*       mode          - '0'-internal loopback, '1'-external loopback
*       state         - '0'-disable, '1'-enable
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_port_loopback
(
    MV_U32               owner_id,
    MV_U32               lport,
    MV_U32               mode,
    MV_U32               state
)
{
    tpm_phy_loopback_mode_t loopback_mode;
    bool                    loopback_state;

    if(mode > TPM_PHY_EXTERNAL_LOOPBACK)
        loopback_mode = TPM_PHY_EXTERNAL_LOOPBACK;
    else
        loopback_mode = (tpm_phy_loopback_mode_t)mode;

    loopback_state = ((1 == state) ? true: false);

    tpm_phy_set_port_loopback(owner_id, (tpm_src_port_type_t)lport, loopback_mode, loopback_state);

    return;
}

/*Evan*/
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
void tpm_sw_add_static_mac_bounce(uint32_t owner_id,
				uint32_t lport,
				uint8_t static_mac[6])
{
	tpm_ioctl_sw_mac_security_t *tpm_sw_mac_set = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_sw_mac_set;

	tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_SW_SECURITY_SECTION;
	tpm_sw_mac_set->sw_security_cmd = MV_TPM_IOCTL_SW_ADD_STATIC_MAC;
	tpm_sw_mac_set->owner_id = owner_id;
	tpm_sw_mac_set->port = lport;
	memcpy(tpm_sw_mac_set->static_mac, (void*)static_mac, sizeof(uint8_t)*6);

	up(&tpm_sfs_2_ioctl_sem);
}
 #define _tpm_sw_add_static_mac tpm_sw_add_static_mac_bounce
#else
 #define _tpm_sw_add_static_mac tpm_sw_add_static_mac
#endif

/*******************************************************************************
* sfs_tpm_sw_add_static_mac
*
* DESCRIPTION:
*       This function creates a static MAC entry in the MAC address table for a
*       specific lport in the integrated switch
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       lport        - Packet origination.
*       static_mac    - 6byte network order MAC source address.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_add_static_mac
(
    MV_U32               owner_id,
    MV_U32               lport,
    MV_U8                static_mac[6]
)
{
    _tpm_sw_add_static_mac(owner_id,(tpm_src_port_type_t)lport,static_mac);

    return;
}

/*Evan*/
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
void tpm_sw_del_static_mac_bounce(uint32_t owner_id,
				uint8_t static_mac[6])
{
	tpm_ioctl_sw_mac_security_t *tpm_sw_mac_set = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_sw_mac_set;

	tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_SW_SECURITY_SECTION;
	tpm_sw_mac_set->sw_security_cmd = MV_TPM_IOCTL_SW_DEL_STATIC_MAC;
	tpm_sw_mac_set->owner_id = owner_id;
	memcpy(tpm_sw_mac_set->static_mac, (void*)static_mac, sizeof(uint8_t)*6);

	up(&tpm_sfs_2_ioctl_sem);
}
 #define _tpm_sw_del_static_mac tpm_sw_del_static_mac_bounce
#else
 #define _tpm_sw_del_static_mac tpm_sw_del_static_mac
#endif
/*******************************************************************************
* sfs_tpm_sw_del_static_mac
*
* DESCRIPTION:
*       This function removes an existing static MAC entry from the MAC address
*       table  in the integrated switch.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       static_mac    - 6byte network order MAC source address.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_del_static_mac
(
    MV_U32        owner_id,
    MV_U8         static_mac[6]
)
{
    _tpm_sw_del_static_mac(owner_id,static_mac);

     return;
}

/*******************************************************************************
* sfs_tpm_sw_clear_dynamic_mac
*
* DESCRIPTION:
*       This function removes all dynamic MAC address in the integrated switch.
*
* INPUTS:
*       owner_id    - APP owner id should be used for all API calls.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_clear_dynamic_mac
(
    MV_U32        owner_id
)
{
    tpm_sw_clear_dynamic_mac(owner_id);

     return;
}

/*******************************************************************************
* sfs_tpm_sw_add_static_mac
*
* DESCRIPTION:
*       This function creates a static MAC entry in the MAC address table for a
*       specific lport in the integrated switch
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       lport        - Packet origination.
*       static_mac    - 6byte network order MAC source address.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_static_mac_w_mask
(
    MV_U32               owner_id,
    MV_U32               ports_mask,
    MV_U8                static_mac[6]
)
{
    tpm_sw_set_static_mac_w_ports_mask(owner_id,ports_mask,static_mac);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_mac_age_time
*
* DESCRIPTION:
*       The API Configures the egress frame rate limit of an Ethernet UNI lport
* INPUTS:
*       owner_id            - APP owner id - should be used for all API calls.
*       time_out            - table size
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*      None
*
*******************************************************************************/
void sfs_tpm_sw_set_mac_age_time
(
    IN uint32_t                 owner_id,
    IN uint32_t                    time_out
)
{
    tpm_sw_set_mac_age_time(owner_id, time_out);

    return;
}

/*Evan*/
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
void tpm_sw_set_port_max_macs_bounce(uint32_t owner_id,
				uint32_t lport,
				uint8_t mac_per_port)
{
	tpm_ioctl_sw_mac_security_t *tpm_sw_mac_set = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_sw_mac_set;

	tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_SW_SECURITY_SECTION;
	tpm_sw_mac_set->sw_security_cmd = MV_TPM_IOCTL_SW_SET_PORT_MAC_LIMIT;
	tpm_sw_mac_set->owner_id = owner_id;
	tpm_sw_mac_set->port = lport;
	tpm_sw_mac_set->mac_per_port = mac_per_port;

	up(&tpm_sfs_2_ioctl_sem);
}
 #define _tpm_sw_set_port_max_macs tpm_sw_set_port_max_macs_bounce
#else
 #define _tpm_sw_set_port_max_macs tpm_sw_set_port_max_macs
#endif

/*******************************************************************************
* sfs_tpm_sw_set_port_max_macs
*
* DESCRIPTION:
*       This function limits the number of MAC addresses per lport.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       lport         - Packet origination.
*       mac_per_port - maximum number of MAC addresses per lport (1-255).
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_port_max_macs
(
    MV_U32                  owner_id,
    MV_U32                  lport,
    MV_U8                     mac_per_port
)
{
    tpm_sw_set_port_max_macs(owner_id,(tpm_src_port_type_t)lport,mac_per_port);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_port_mac_learn
*
* DESCRIPTION:
*       This function set MAC learning ability of given lport
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       lport        - Packet origination.
*       state        - MAC learning state. 0:disable, 1:enable.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_port_mac_learn
(
    MV_U32      owner_id,
    MV_U32      lport,
    MV_U8       state
)
{
    bool mac_state;

    mac_state = ((1 == state) ? true: false);

    tpm_sw_set_mac_learn(owner_id, (tpm_src_port_type_t)lport, mac_state);

    return;
}

/*Evan*/
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
void tpm_sw_set_port_flooding_bounce(uint32_t owner_id,
					tpm_src_port_type_t src_port,
					tpm_flood_type_t    flood_mode,
					uint8_t             allow_flood)
{
	tpm_ioctl_sw_mac_security_t *tpm_sw_mac_set = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_sw_mac_set;

	tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_SW_SECURITY_SECTION;
	tpm_sw_mac_set->sw_security_cmd = MV_TPM_IOCTL_SW_SET_PORT_FLOOD;
	tpm_sw_mac_set->owner_id = owner_id;
	tpm_sw_mac_set->port = src_port;
	tpm_sw_mac_set->flood_mode = flood_mode;
	tpm_sw_mac_set->allow_flood = allow_flood;

	up(&tpm_sfs_2_ioctl_sem);
}
 #define _tpm_sw_set_port_flooding tpm_sw_set_port_flooding_bounce
#else
 #define _tpm_sw_set_port_flooding tpm_sw_set_port_flooding
#endif

/*******************************************************************************
* sfs_tpm_sw_set_port_flooding
*
* DESCRIPTION:
*       This function control the flooding behavior (unknown Dest MAC address) per lport.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       egress_port     - Egress lport for applying the flood settings.
*       allow_flood  - permit flooding of unknown DA: set to 1,
*                      do not permit flooding of uknown DA: set to 0.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*      None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_port_flooding
(
    MV_U32           owner_id,
    MV_U32           lport,
    MV_U32           allow_flood
)
{
    _tpm_sw_set_port_flooding(owner_id,(tpm_src_port_type_t)lport, TPM_FLOOD_UNKNOWN_UNICAST, allow_flood);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_port_mc_flooding
*
* DESCRIPTION:
*       This function control the flooding behavior (unknown GDA) per lport.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       egress_port     - Egress lport for applying the flood settings.
*       allow_flood  - permit flooding of unknown multicast: set to 1,
*                      do not permit flooding of uknown multicast: set to 0.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*      None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_port_mc_flooding
(
    MV_U32           owner_id,
    MV_U32           lport,
    MV_U32           allow_flood
)
{
    tpm_sw_set_port_flooding (owner_id,(tpm_src_port_type_t)lport,TPM_FLOOD_UNKNOWN_MULTI_BROAD_CAST, allow_flood);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_bc_flooding
*
* DESCRIPTION:
*       This function control the flooding behavior (broadcast address) per lport.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       allow_flood  - permit flooding of BC: set to 1,
*                      do not permit flooding of BC: set to 0.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*      None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_bc_flooding
(
    MV_U32           owner_id,
    MV_U32           always_on
)
{
    bool lFloodOn;
    if(!always_on)
        lFloodOn = true;
    else
        lFloodOn = false;
    tpm_sw_set_broadcast_flood(owner_id,lFloodOn);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_port_tagged
*
* DESCRIPTION:
*       The API allows or drops tagged packets on a per lport basis.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       lport         -  lport for applying the filtering of tagged packets.
*       allow_tagged - set to 1 = allow tagged packets per lport
*                      set to 0 = drop tagged packets per lport.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_port_tagged
(
    MV_U32    owner_id,
    MV_U32    lport,
    MV_U8     allow_tagged
)
{
    tpm_sw_set_port_tagged(owner_id,(tpm_src_port_type_t)lport,allow_tagged);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_port_untagged
*
* DESCRIPTION:
*       The API allows or drops untagged packets on a per lport basis.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       lport         -  lport for applying the filtering of tagged packets.
*       allow_untagged - set to 1 = allow tagged packets per lport
*                      set to 0 = drop tagged packets per lport.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_port_untagged
(
    MV_U32    owner_id,
    MV_U32    lport,
    MV_U8     allow_untagged
)
{
    tpm_sw_set_port_untagged(owner_id,(tpm_src_port_type_t)lport,allow_untagged);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_port_def_vlan
*
* DESCRIPTION:
*       The API sets port default vlan id.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       lport       - lport for applying the filtering of tagged packets.
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
void sfs_tpm_sw_set_port_def_vlan
(
    MV_U32    owner_id,
    MV_U32    lport,
    MV_U16    vid
)
{
    tpm_sw_set_port_def_vlan(owner_id,(tpm_src_port_type_t)lport,vid);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_port_def_pri
*
* DESCRIPTION:
*       The API sets port default priority.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       lport       - lport for applying the filtering of tagged packets.
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
void sfs_tpm_sw_set_port_def_pri
(
    MV_U32    owner_id,
    MV_U32    lport,
    MV_U8     pri
)
{
    tpm_sw_set_port_def_pri(owner_id,(tpm_src_port_type_t)lport,pri);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_port_add_vid
*
* DESCRIPTION:
*       The API adds a VID to the list of allowed VIDs per lport.
*
* INPUTS:
*       owner_id - APP owner id - should be used for all API calls.
*       lport    -  lport for adding the vid.
*       vid      - VLAN id.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_port_add_vid
(
    MV_U32    owner_id,
    MV_U32    lport,
    MV_U16    vid
)
{
    tpm_sw_port_add_vid(owner_id,(tpm_src_port_type_t)lport,vid);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_del_port_vid
*
* DESCRIPTION:
*       The API delete the existed VID from the list of VIDs allowed per lport.
*
* INPUTS:
*       owner_id - APP owner id - should be used for all API calls.
*       lport    -  lport for deleting the VID.
*       vid      - VLAN id.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_del_port_vid
(
    MV_U32    owner_id,
    MV_U32    lport,
    MV_U16    vid
)
{
    tpm_sw_port_del_vid(owner_id,(tpm_src_port_type_t)lport,vid);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_port_add_gvid
*
* DESCRIPTION:
*       The API adds a group of VID to the list of allowed VIDs per lport.
*
* INPUTS:
*       owner_id  - APP owner id - should be used for all API calls.
*       lport     - lport for adding the vid.
*       egr_mode  - VLAN egress mode.
*       min_vid   - minimum VID.
*       max_vid   - maximum VID.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_port_add_gvid
(
    MV_U32    owner_id,
    MV_U32    lport,
    MV_U16    egr_mode,
    MV_U16    min_vid,
    MV_U16    max_vid
)
{
    MV_U16    mode;

    if(egr_mode > MEMBER_EGRESS_TAGGED)
        mode = MEMBER_EGRESS_UNMODIFIED;
    else
        mode = egr_mode;

    tpm_sw_port_add_vid_group(owner_id, (tpm_src_port_type_t)lport, (MV_U8)mode, min_vid, max_vid);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_del_port_gvid
*
* DESCRIPTION:
*       The API delete the existed VID from the list of VIDs allowed per lport.
*
* INPUTS:
*       owner_id  - APP owner id - should be used for all API calls.
*       lport     - lport for deleting the VID.
*       min_vid   - minimum VID.
*       max_vid   - maximum VID.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_del_port_gvid
(
    MV_U32    owner_id,
    MV_U32    lport,
    MV_U16    min_vid,
    MV_U16    max_vid
)
{
    tpm_sw_port_del_vid_group(owner_id, (tpm_src_port_type_t)lport, min_vid, max_vid);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_port_vid_egress_mode
*
* DESCRIPTION:
*       The API sets the egress mode for a member port of a vlan.
*
* INPUTS:
*       owner_id  - APP owner id - should be used for all API calls.
*       lport     - logic port id to set
*       vid       - vlan id
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
*       MEMBER_EGRESS_UNMODIFIED - 0
*       NOT_A_MEMBER             - 1
*       MEMBER_EGRESS_UNTAGGED   - 2
*       MEMBER_EGRESS_TAGGED     - 3
*
*******************************************************************************/
void sfs_tpm_sw_set_port_vid_egress_mode
(
    MV_U32    owner_id,
    MV_U32    lport,
    MV_U16    vid,
    MV_U8     eMode
)
{
    tpm_sw_set_port_vid_egress_mode(owner_id,(tpm_src_port_type_t)lport,vid,eMode);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_vid_filter_per_port
*
* DESCRIPTION:
*       The API sets the filtering mode of a certain lport.
*       If the lport is in filtering mode, only the VIDs added by the
*       tpm_sw_port_add_vid API will be allowed to ingress and egress the lport.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       lport        -  lport for setting the filtering mode.
*       vid_filter  - set to 1 - means the lport will DROP all packets which are NOT in
*                    the allowed VID list (built using API tpm_sw_port_add_vid).
*                    set to 0 - means that the list of VIDs allowed
*                    per lport has no significance (the list is not deleted).
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void  sfs_tpm_sw_set_vid_filter_per_port
(
    MV_U32    owner_id,
    MV_U32    lport,
    MV_U8     vid_filter
)
{
    tpm_sw_port_set_vid_filter(owner_id,(tpm_src_port_type_t)lport,vid_filter);

    return;
}

/*******************************************************************************
* tpm_sw_set_uni_q_weight
*
* DESCRIPTION:
*       The API configures the weight of a queue for all
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
*       None
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_uni_q_weight
(
    MV_U32        owner_id,
    MV_U8         queue_id,
    MV_U8         weight
)
{
    tpm_sw_set_uni_q_weight(owner_id,queue_id,weight);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_uni_ingr_police_rate
*
* DESCRIPTION:
*       The API Configures an ingress policing function for an Ethernet UNI lport.
*
* INPUTS:
*       owner_id - APP owner id - should be used for all API calls.
*       uni_port - uni lport for configuring the ingress policer function.
*       mode     - policing mode.
*       cir      - commited info rate.
*       cbs      - commited burst rate
*       ebs      - extra burst rate

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
void sfs_tpm_sw_set_uni_ingr_police_rate
(
    MV_U32      owner_id,
    MV_U32      uni_port,
    MV_U32      mode,
    MV_U32      cir,
    MV_U32      cbs,
    MV_U32      ebs
)
{
    MV_U32 count_mode;

    if(mode > GT_PIRL_ELIMIT_LAYER3)
        count_mode  = GT_PIRL_ELIMIT_LAYER2;
    else
        count_mode  = mode;

    tpm_sw_set_uni_ingr_police_rate(owner_id, (tpm_src_port_type_t)uni_port, count_mode, cir, cbs, ebs);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_uni_tc_ingr_police_rate
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
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_uni_tc_ingr_police_rate
(
    MV_U32      owner_id,
    MV_U32      uni_port,
    MV_U32      tc,
    MV_U32      cir,
    MV_U32      cbs
)
{
    tpm_sw_set_uni_tc_ingr_police_rate(owner_id,(tpm_src_port_type_t)uni_port,tc,cir,cbs);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_uni_egr_rate_limit
*
* DESCRIPTION:
*       The API Configures the egress frame rate limit of an Ethernet UNI lport
* INPUTS:
*       owner_id            - APP owner id - should be used for all API calls.
*       trg_lport            - uni lport for configuring the egress rate limit.
*       kb_rate_limit_val  - egress rate limit value.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*      None
*
*******************************************************************************/
void sfs_tpm_sw_set_uni_egr_rate_limit
(
    IN uint32_t     owner_id,
    IN uint32_t     trg_lport,
    IN uint32_t     set_uni_egr_rate_limit
)
{
    tpm_sw_set_uni_egr_rate_limit(owner_id,(tpm_src_port_type_t)trg_lport,TPM_SW_LIMIT_LAYER2,set_uni_egr_rate_limit);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_uni_egr_rate_limit
*
* DESCRIPTION:
*       The API Configures the egress frame rate limit of an Ethernet UNI lport
* INPUTS:
*       owner_id            - APP owner id - should be used for all API calls.
*       size            - table size
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*      None
*
*******************************************************************************/
void sfs_tpm_sw_set_atu_size
(
    IN uint32_t   owner_id,
    IN uint32_t   size
)
{
    tpm_sw_set_atu_size(owner_id, size);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_port_vlan_ports
*
* DESCRIPTION:
*       The API Configures the egress frame rate limit of an Ethernet UNI lport
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       lport       - logical port number to set.
*       memPorts    - array of logical ports in the same vlan.
*       memPortsLen - number of members in memPorts array
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*      None
*
*******************************************************************************/
void sfs_tpm_sw_set_port_vlan_ports
(
    IN uint32_t             owner_id,
    IN uint32_t             lport,
    IN uint32_t             memPorts[],
    IN uint8_t              memPortsLen
)
{
    tpm_sw_set_port_vlan_ports(owner_id,(tpm_src_port_type_t)lport,memPorts,memPortsLen);

    return;
}


/*******************************************************************************
* sfs_tpm_sw_set_ingress_limit_mode
*
* DESCRIPTION:
*      This routine sets the port's rate control ingress limit mode.
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       lport       - logical port number to set.
*       mode         - rate control ingress limit mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*      None
*
*******************************************************************************/
void sfs_tpm_sw_set_ingress_limit_mode
(
    IN uint32_t                 owner_id,
    IN uint32_t                 lport,
    IN GT_RATE_LIMIT_MODE       mode
)
{
    tpm_sw_set_ingress_limit_mode(owner_id,(tpm_src_port_type_t)lport,mode);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_set_priority_selection
*
* DESCRIPTION:
*       This function sets initial QPri and FPri selection.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       lport       - Port number to set init pri.
*       init_pri    - Initial QPri and FPri selection type.
*       tag_if_both - Use Tag information for the initial QPri assignment if the frame is both
*                     tagged and its also IPv4 or IPv6 and if InitialPri uses Tag & IP priority.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
* COMMENTS:
*       PRI_SEL_USE_PORTS_DEF  - 0
*       PRI_SEL_TAG_PRI_ONLY   - 1
*       PRI_SEL_IP_PRI_ONLY    - 2
*       PRI_SEL_TAG_AND_IP_PRI - 3
*
*******************************************************************************/
void sfs_tpm_sw_set_priority_selection
(
    IN uint32_t owner_id,
    IN uint8_t  lport,
    IN uint8_t  init_pri,
    IN GT_BOOL  tag_if_both
)
{
    bool lTag;
    if(GT_TRUE == tag_if_both)
        lTag = true;
    else
        lTag = false;
    tpm_sw_set_priority_selection(owner_id,(tpm_src_port_type_t)lport,init_pri,lTag);
}

/*******************************************************************************
* sfs_tpm_sw_set_tag_pri_mapping
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
*       None.
*
* COMMENTS:
*       tag_pri 0-7.
*       q_pri   0-3.
*
*******************************************************************************/
void sfs_tpm_sw_set_tag_pri_mapping
(
    IN  uint32_t owner_id,
    IN  uint8_t  tag_pri,
    IN  uint8_t  q_pri
)
{
    tpm_sw_set_tag_pri_mapping(owner_id,tag_pri,q_pri);
}

/*******************************************************************************
* sfs_tpm_sw_set_ip_pri_mapping
*
* DESCRIPTION:
*       This function maps a dscp value to a queue priority.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       dscp        - Source dscp value.
*       q_pri       - Target queue priority number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
* COMMENTS:
*       dscp    0-63.
*       q_pri   0-3.
*
*******************************************************************************/
void sfs_tpm_sw_set_ip_pri_mapping
(
    IN  uint32_t owner_id,
    IN  uint8_t  dscp,
    IN  uint8_t  q_pri
)
{
    tpm_sw_set_ip_pri_mapping(owner_id,dscp,q_pri);
}

/*******************************************************************************
* sfs_tpm_sw_set_gmac_mtu
*
* DESCRIPTION:
*       This function set the MTU of GMAC.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       gmac        - GMAC, '0'-GMAC0, '1'-GMAC1, '02'-PONMAC, '3'-switch.
*       mtu         - MTU size.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
*
*******************************************************************************/
void sfs_tpm_sw_set_gmac_mtu
(
    IN  uint32_t  owner_id,
    IN  uint32_t  gmac,
    IN  uint32_t  mtu
)
{

    tpm_mru_type_t type;
    if(gmac > TPM_NETA_MTU_SWITCH)
        type = TPM_NETA_MTU_SWITCH;
    else
        type = (tpm_mru_type_t)gmac;

    tpm_set_mtu_size(owner_id, type, mtu);
}

/*******************************************************************************
* sfs_tpm_sw_set_port_mirror
*
* DESCRIPTION:
*       This function set port mirror.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       src_port    - src UNI port(1-4).
*       dst_port    - dst UNI port(1-4).
*       mode        - mirror mode, '0'-ingress mirror, '1'-egress mirror.
*       state       - mirror state, '0'-disable, '1'-enable.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
*
*******************************************************************************/
void sfs_tpm_sw_set_port_mirror
(
    IN  uint32_t  owner_id,
    IN  uint32_t  src_port,
    IN  uint32_t  dst_port,
    IN  uint32_t  mode,
    IN  uint32_t  state
)
{
    tpm_sw_mirror_type_t mirror_mode;
    bool                 mirror_state;

    if(mode > TPM_SW_MIRROR_EGRESS)
        mirror_mode = TPM_SW_MIRROR_EGRESS;
    else
        mirror_mode = (tpm_sw_mirror_type_t)mode;

    mirror_state = ((1 == state) ? true: false);

    tpm_sw_set_port_mirror(owner_id, src_port, dst_port, mirror_mode, mirror_state);
}


/*******************************************************************************
* sfs_tpm_sw_set_trunk_ports
*
* DESCRIPTION:
*       This function set port mirror.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       trunk_id    - valid from 0x0 to 0xf
*       ports_mask  - mask for real switch port, not logical port like TPM_SRC_PORT_UNI_0.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
*
*******************************************************************************/
void sfs_tpm_sw_set_trunk_ports
(
    IN  uint32_t  owner_id,
    IN  uint32_t  trunk_id,
    IN  uint32_t  ports_mask
)
{
    tpm_sw_set_trunk_ports(owner_id, trunk_id, ports_mask);
}
/*******************************************************************************
* sfs_tpm_sw_set_trunk_mask
*
* DESCRIPTION:
*       This function set port mirror.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       mask_num    - trunk mask number, valid from 0 to 7.
*       trunk_mask  - mask for real switch port, not logical port like TPM_SRC_PORT_UNI_0.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
*
*******************************************************************************/
void sfs_tpm_sw_set_trunk_mask
(
    IN	uint32_t  owner_id,
    IN	uint32_t  mask_num,
    IN	uint32_t  trunk_mask
)
{
    tpm_sw_set_trunk_mask(owner_id, mask_num, trunk_mask);
}

/*******************************************************************************
* sfs_tpm_sw_set_port_speed_duplex
*
* DESCRIPTION:
*       This function set the port speed and duplex together
*
* INPUTS:
*       owner_id      - APP owner id - should be used for all API calls.
*       lport         - Packet origination.
*       speed_mode    - speed mode, '0'-10M, '1'-100M, '2'-1000M.
*       mode          - duplex mode, 1:enable, 0:disable.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_set_port_speed_duplex
(
    MV_U32               owner_id,
    MV_U32               lport,
    MV_U32               speed_mode,
    MV_U32               mode
)
{
    tpm_phy_speed_t speed;
    bool duplex_mode;

    if((tpm_phy_speed_t)speed_mode > TPM_PHY_SPEED_1000_MBPS)
        speed = (tpm_phy_speed_t)TPM_PHY_SPEED_1000_MBPS;
    else
        speed = (tpm_phy_speed_t)speed_mode;

    duplex_mode = ((1 == mode) ? true: false);

    tpm_phy_set_port_speed_duplex_mode(owner_id, (tpm_src_port_type_t)lport, speed, duplex_mode);

    return;
}

/*******************************************************************************
* sfs_tpm_sw_port_add_vid_set_egrs_mode
*
* DESCRIPTION:
*       The API adds a VID to the list of allowed VIDs per lport,
*       and set the port's egress mode.
*
* INPUTS:
*       owner_id - APP owner id - should be used for all API calls.
*       lport    -  lport for adding the vid.
*       vid      - VLAN id.
*       eMode    - port egress mode
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_sw_port_add_vid_set_egrs_mode
(
    MV_U32    owner_id,
    MV_U32    lport,
    MV_U16    vid,
    MV_U8     eMode
)
{
    tpm_sw_port_add_vid_set_egrs_mode(owner_id,(tpm_src_port_type_t)lport,vid,eMode);

    return;
}

#ifdef CONFIG_MV_TPM_SYSFS_HELP
/*******************************************************************************
**
**  sfs_tpm_sw_set_help_show
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print cfg help
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     message length
**
*******************************************************************************/
int sfs_tpm_sw_set_help_show_1(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "==============================================================================================================================================\n");
    off += sprintf(buf+off, "                                 Configuration Switch Commands  [DEC]                                                                         \n");
    off += sprintf(buf+off, "==============================================================================================================================================\n");
    off += sprintf(buf+off, "cat help_sw_set_cfg_1                                                                                           - show this help\n");
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [state('1'-enable,'0'-disable)]                    > set_port_admin              - set port administration state\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [state('1'-enable,'0'-disable)] [mode('0'~'11')]   > set_port_autoneg            - set port auto negotiation\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d]                                                    > restart_autoneg             - restart port auto negotiation\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [mode('1'-enable,'0'-disable)]                     > set_port_duplex             - set port duplex mode\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [state('1'-enable,'0'-disable)]                    > set_port_fc                 - set port flow control\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [mode('0'-10M,'1'-100M,'2'-1000M)]                 > set_port_speed              - set port speed mode\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [vector('0'-isolate,'1'-enable, bit0-UNI0...]      > set_port_isolate_vector     - set port isolation vector\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [mode('0'-internal,'1'-external)] [state]          > set_port_loopback           - set port loopback state, enable or disable it. \n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] mac[6] mac[5] mac[4] mac[3] mac[2] mac[1]          > add_static_mac              - add static mac to port\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] mac[6], mac[5] mac[4] mac[3]  mac[2], mac[1]                      > del_static_mac              - delete static mac\n");
    off += sprintf(buf+off, "echo [owner_id]                                                                   > clear_dynamic_mac           - clear all dynamic mac\n");
    off += sprintf(buf+off, "echo [owner_id] [port_mask] mac[6] mac[5] mac[4] mac[3] mac[2] mac[1]             > set_static_mac_w_mask       - set static mac with port vector\n");
    off += sprintf(buf+off, "echo [owner_id] [time_out(sec)]                                                   > set_mac_age_time            - set the MAC aging time\n");
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [mac_pe_port(0 - 255)]                             > set_port_max_macs           - set the maximum MAC address of given port\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [state('1'-enable,'0'-disable)]                    > set_port_mac_learn          - set port MAC learning state\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [size 0(256) - 5(8192)]                                           > set_atu_size                - set the FDB size \n");
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [allow_flood ('1'-permit,'0'-not)]                 > set_port_flooding           - permit flooding of unknown DA\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [allow_flood ('1'-permit,'0'-not)]                 > set_port_mc_flooding        - permit flooding of unknown multicast\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [always_on ('1'-permit,'0'-use mc rules)]                         > set_bc_flooding             - permit flooding of broadcast\n");
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [drop_tagged('1'-drop,  '0'-not)]                  > set_port_tagged             - allows or drops tagged packets\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [drop_untagged('1'-drop,'0'-not)]                  > set_port_untagged           - allows or drops untagged packets\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [vid_filter('1'-Drop,'0'-not)]                     > set_vid_filter_per_port     - set the filtering mod VID\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [speed('0'-10M,'1'-100M,'2'-1000M)] [duplex('1'-enable,'0'-disable)]  > set_port_speed_duplex - set port speed and duplex mode\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "====================================================================================================================================================\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_tpm_sw_set_help_show_2
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print cfg help
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     message length
**
*******************************************************************************/
int sfs_tpm_sw_set_help_show_2(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "==============================================================================================================================================\n");
    off += sprintf(buf+off, "                                 Configuration Switch Commands  [DEC]                                                                         \n");
    off += sprintf(buf+off, "==============================================================================================================================================\n");
    off += sprintf(buf+off, "cat help_sw_set_cfg_2                                                                                           - show this help\n");
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [vid(1-4094)]                                      > set_port_def_vlan           - set port default vid\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [pri(0-7)]                                         > set_port_def_pri            - set port default pri\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [vid(1-4094)]                                      > add_port_vid                - add a VID to lport\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [vid(1-4094)]                                      > del_port_vid                - delete the existed VID from given port\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [egr_mode] [min_vid(0-4095)] [max_vid(0-4095)]     > add_port_gvid               - add a group of VID to lport and set egress mode('0'-AS_IS, '2'-RM_TAG, '3'-ADD_TAG)\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [min_vid(0-4095)] [max_vid(0-4095)]                > del_port_gvid               - delete a group of VID from lport\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [vid(1-4094)] [egr_mode]                           > set_port_vid_egr_mode       - set the egress modee('0'-AS_IS, '2'-RM_TAG, '3'-ADD_TAG)\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [queue_id 1-3]  [weight 1-8]                                      > set_uni_q_weight            - configure the weight of a queues\n");
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] rate [egr_rate_limit_menu]                         > set_uni_egr_rate_limit      - set egress frame rate limit\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] mode[rate_limit_menu] [cir] [cbs] [ebs]            > set_uni_ingr_police_rate    - set ingress policing\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] tc[1-4] cir[rate_limit_menu] cbs[0 ~ 0xFFFF)       > set_uni_tc_ingr_police_rate - set ingress policing\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] mode [rate_limit_menu]                             > set_ingress_limit_mode      - set ingress policing mode\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] Port[6]Port[5]Port[4]Port[3]Port[2]Port[1]len[1-6] > set_port_vlan_ports         - add lport to VLAN group membership\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [init_pri_menu] [tag_if_both 0-1]                  > set_init_pri                - set initial priority\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "echo [owner_id] [pri 0-7] [q 0-3]                                                 > set_tag_pri_mapping         - map tag pri to queue\n");
    off += sprintf(buf+off, "echo [owner_id] [dscp 0-63] [q 0-3]                                               > set_ip_pri_mapping          - map ip pri to queue\n");
    off += sprintf(buf+off, "echo [owner_id] [gmac('0'-GMAC0, '1'-GMAC1, '2'-PMAC, '3'-switch)] [mtu]          > set_gmac_mtu                - set GMAC MTU\n");
    off += sprintf(buf+off, "echo [owner_id] [src port] [dst port] [mode('0'-ingress), '1'-egress] [state]     > set_port_mirror             - set port mirror, src/dst port is UNI port(1-4), state('1'-enable,'0'-disable)\n");
    off += sprintf(buf+off, "echo [owner_id] [trunk_id] [port mask]                                            > set_trunk_ports             - set trunk ID and port mask\n");
    off += sprintf(buf+off, "echo [owner_id] [mask num] [trunk mask]                                           > set_trunk_mask              - set trunk mask\n");
    off += sprintf(buf+off, "echo [owner_id] [UNI port %.1d-%.1d] [vid(1-4094)] [egr_mode]                           > add_port_vid_set_egr_mode   - add a VID to lport and set the egress mode('0'-AS_IS, '2'-RM_TAG, '3'-ADD_TAG)\n", TPM_SRC_PORT_UNI_0, TPM_SRC_PORT_UNI_3);
    off += sprintf(buf+off, "====================================================================================================================================================\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_tpm_sw_rate_limit_menu
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print cfg help
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     message length
**
*******************************************************************************/
int sfs_tpm_sw_rate_limit_menu(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "==================================================================================================\n");
    off += sprintf(buf+off, "                                 rate limit menu                                                  \n");
    off += sprintf(buf+off, "==================================================================================================\n");
    off += sprintf(buf+off, "cir allowed values are: 64, 128, 192, 256, 320,,..., 960,1000, 2000..., 100000,110000, 120000,..,200000\n");
    off += sprintf(buf+off, "mode enumeration values are: '0'-All,'1'-LIMIT_FLOOD,'2'-LIMIT_BRDCST_MLTCST,'3'-LIMIT_BRDCST\n");
    off += sprintf(buf+off, "==================================================================================================\n");

    return(off);
}
/*******************************************************************************
**
**  sfs_tpm_sw_egr_rate_limit_menu
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print cfg help
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     message length
**
*******************************************************************************/
int sfs_tpm_sw_egr_rate_limit_menu(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "==================================================================================================\n");
    off += sprintf(buf+off, "                                egress rate limit menu \n");
    off += sprintf(buf+off, "==================================================================================================\n");
    off += sprintf(buf+off, "rate allowed values are: 0(MAX) 7600,..,7600+n*64,9600, 100000 110000, 120000,..., 1000000\n");
    off += sprintf(buf+off, "==================================================================================================\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_tpm_sw_init_pri_menu
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print cfg help
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     message length
**
*******************************************************************************/
int sfs_tpm_sw_init_pri_menu(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "==================================================================================================\n");
    off += sprintf(buf+off, "                                 initial priority menu           \n");
    off += sprintf(buf+off, "==================================================================================================\n");
    off += sprintf(buf+off, "mode enumeration values are: '0'-USE_PORTS_DEF,'1'-TAG_PRI_ONLY,'2'-IP_PRI_ONLY,'3'-IP_AND_TAG_PRI\n");
    off += sprintf(buf+off, "==================================================================================================\n");

    return(off);
}
#endif /* CONFIG_MV_TPM_SYSFS_HELP */
/* ========================================================================== */
/******************************************************************************/

/* ========================================================================== */
/*                            Cfg Commands                                    */
/* ========================================================================== */
/* ========================================================================== */
/*                            Cfg Commands                                    */
/* ========================================================================== */
#ifdef CONFIG_MV_TPM_SYSFS_HELP
static ssize_t set_sw_show(struct device *dev,
                        struct device_attribute *attr,
                        char *buf)
{
    const char* name = attr->attr.name;

    if (!capable(CAP_NET_ADMIN))
        return -EPERM;

    if (!strcmp(name, "help_sw_set_cfg_1"))
        return (sfs_tpm_sw_set_help_show_1(buf));
    if (!strcmp(name, "help_sw_set_cfg_2"))
        return (sfs_tpm_sw_set_help_show_2(buf));
    else if (!strcmp(name, "rate_limit_menu"))
        return (sfs_tpm_sw_rate_limit_menu(buf));
    else if (!strcmp(name, "egr_rate_limit_menu"))
        return (sfs_tpm_sw_egr_rate_limit_menu(buf));
    else if (!strcmp(name, "init_pri_menu"))
        return (sfs_tpm_sw_init_pri_menu(buf));

    return 0;
}
#else
static ssize_t set_sw_show(struct device *dev,
                           struct device_attribute *attr,
                           char *buf)
{
	printk(KERN_WARNING "%s: sysfs help is not compiled (CONFIG_MV_TPM_SYSFS_HELP)\n", __FUNCTION__);
    return 0;
}
#endif /* CONFIG_MV_TPM_SYSFS_HELP */


static ssize_t set_sw_store(struct device *dev,
                         struct device_attribute *attr,
                         const char *buf, size_t len)
{
    const char* name = attr->attr.name;
//     unsigned long flags  = 0;
    int  param1 = 0;
    int  param2 = 0;
    int  param3 = 0;
    int  param4 = 0;
    int  param5 = 0;
    int  param6 = 0;
    int  param7 = 0;
    int  param8 = 0;
    int  param9 = 0;
    MV_U8         static_mac[6] = {0,0,0,0,0,0};
    MV_U32        memPorts[6];

    if (!capable(CAP_NET_ADMIN))
    return -EPERM;

    if (!strcmp(name, "add_static_mac"))
    {
        sscanf(buf, "%d %d %d %d %d %d %d %d",
               &param1, &param2,&param3,&param4,
               &param5, &param6,&param7,&param8);

        static_mac[0] = param3;
        static_mac[1] = param4;
        static_mac[2] = param5;
        static_mac[3] = param6;
        static_mac[4] = param7;
        static_mac[5] = param8;

    }
    else if (!strcmp(name, "del_static_mac"))
    {
        sscanf(buf, "%d %d %d %d %d %d %d ",
               &param1, &param2,&param3,
               &param4,&param5, &param6,&param7);

        static_mac[0] = param2;
        static_mac[1] = param3;
        static_mac[2] = param4;
        static_mac[3] = param5;
        static_mac[4] = param6;
        static_mac[5] = param7;
    }
    else if (!strcmp(name, "clear_dynamic_mac"))
    {
        sscanf(buf, "%d ",
               &param1);

    }
    else if (!strcmp(name, "set_static_mac_w_mask"))
    {
        sscanf(buf, "%d %d %d %d %d %d %d %d",
               &param1, &param2,&param3,&param4,
               &param5, &param6,&param7,&param8);

        static_mac[0] = param3;
        static_mac[1] = param4;
        static_mac[2] = param5;
        static_mac[3] = param6;
        static_mac[4] = param7;
        static_mac[5] = param8;

    }
    else if (!strcmp(name, "add_port_gvid"))
    {
        sscanf(buf, "%d %d %d %d %d",
               &param1, &param2,&param3,&param4,&param5);
    }
    else if (!strcmp(name, "set_port_mirror"))
    {
        sscanf(buf, "%d %d %d %d %d",
               &param1, &param2,&param3,&param4,&param5);
    }
    else if (!strcmp(name, "set_uni_ingr_police_rate"))
    {
        sscanf(buf, "%d %d %d %d %d %d",
               &param1, &param2, &param3,
               &param4, &param5, &param6);
    }
    else if (!strcmp(name, "set_uni_tc_ingr_police_rate"))
    {
        sscanf(buf, "%d %d %d %d %d ",
               &param1, &param2,&param3,
               &param4,&param5);
    }
    else if (!strcmp(name, "set_port_vlan_ports"))
    {
        sscanf(buf, "%d %d %d %d %d %d %d %d %d",
               &param1, &param2,&param3,&param4,
               &param5, &param6,&param7,&param8,&param9);

        memPorts[0] = param3;
        memPorts[1] = param4;
        memPorts[2] = param5;
        memPorts[3] = param6;
        memPorts[4] = param7;
        memPorts[5] = param8;
    }
    else if (!strcmp(name, "set_init_pri"))
    {
        sscanf(buf, "%d %d %d %d",
               &param1,&param2,&param3,&param4);
    }
    else if (!strcmp(name, "set_tag_pri_mapping"))
    {
        sscanf(buf, "%d %d %d",
               &param1,&param2,&param3);
    }
    else if (!strcmp(name, "set_ip_pri_mapping"))
    {
        sscanf(buf, "%d %d %d",
               &param1,&param2,&param3);
    }
    else
    {
        sscanf(buf, "%d %d %d %d",
               &param1, &param2,&param3,&param4);
    }

    printk(KERN_INFO "param1[%d],param2[%d],param3[%d],param4[%d],param5[%d],param6[%d],param7[%d],param8[%d] param9[%d] \n",
          param1,param2,param3,param4,
          param5,param6,param7,param8,param9);
    if (param1 < 0 ||
        param2 < 0 ||
        param3 < 0 ||
        param4 < 0 ||
        param5 < 0 ||
        param6 < 0 ||
        param7 < 0 ||
        param8 < 0 ||
        param9 < 0) {
            printk("ERROR %s: illegal negative parameter <%s>\n", __FUNCTION__, attr->attr.name);
            return (len);
    }

//     raw_local_irq_save(flags);

    if (!strcmp(name, "set_port_admin"))
        sfs_tpm_sw_set_port_admin((MV_U32)param1,(MV_U32)param2, (MV_U32)param3);

    else if (!strcmp(name, "set_port_autoneg"))
        sfs_tpm_sw_set_port_autoneg((MV_U32)param1,(MV_U32)param2, (MV_U32)param3, (MV_U32)param4);

    else if (!strcmp(name, "restart_autoneg"))
        sfs_tpm_sw_restart_autoneg((MV_U32)param1,(MV_U32)param2);

    else if (!strcmp(name, "set_port_duplex"))
        sfs_tpm_sw_set_port_duplex((MV_U32)param1,(MV_U32)param2, (MV_U32)param3);

    else if (!strcmp(name, "set_port_fc"))
        sfs_tpm_sw_set_port_fc((MV_U32)param1,(MV_U32)param2, (MV_U32)param3);

    else if (!strcmp(name, "set_port_speed"))
        sfs_tpm_sw_set_port_speed((MV_U32)param1,(MV_U32)param2, (MV_U32)param3);

    else if (!strcmp(name, "set_port_isolate_vector"))
        sfs_tpm_sw_set_port_isolate_vector((MV_U32)param1,(MV_U32)param2, (MV_U32)param3);

    else if (!strcmp(name, "set_port_loopback"))
        sfs_tpm_sw_set_port_loopback((MV_U32)param1,(MV_U32)param2, (MV_U32)param3, (MV_U32)param4);

    else if (!strcmp(name, "add_static_mac"))
        sfs_tpm_sw_add_static_mac((MV_U32)param1,(MV_U32)param2, static_mac);

    else if (!strcmp(name, "del_static_mac"))
        sfs_tpm_sw_del_static_mac((MV_U32)param1, static_mac);

    else if (!strcmp(name, "clear_dynamic_mac"))
        sfs_tpm_sw_clear_dynamic_mac((MV_U32)param1);

    else if (!strcmp(name, "set_static_mac_w_mask"))
        sfs_tpm_sw_set_static_mac_w_mask((MV_U32)param1,(MV_U32)param2, static_mac);

    else if (!strcmp(name, "set_port_max_macs"))
        sfs_tpm_sw_set_port_max_macs((MV_U32)param1 ,(MV_U32)param2 ,(MV_U8)param3);

    else if (!strcmp(name, "set_port_mac_learn"))
        sfs_tpm_sw_set_port_mac_learn((MV_U32)param1 ,(MV_U32)param2 ,(MV_U8)param3);

    else if
        (!strcmp(name, "set_atu_size"))
                sfs_tpm_sw_set_atu_size((MV_U32)param1,(MV_U32)param2);
    else if
        (!strcmp(name, "set_mac_age_time"))
                sfs_tpm_sw_set_mac_age_time((MV_U32)param1,(MV_U32)param2);

    else if (!strcmp(name, "set_port_flooding"))
        sfs_tpm_sw_set_port_flooding((MV_U32)param1 ,(MV_U32)param2 ,(MV_U32)param3);

    else if (!strcmp(name, "set_port_mc_flooding"))
        sfs_tpm_sw_set_port_mc_flooding((MV_U32)param1 ,(MV_U32)param2 ,(MV_U32)param3);

    else if (!strcmp(name, "set_bc_flooding"))
        sfs_tpm_sw_set_bc_flooding((MV_U32)param1 ,(MV_U32)param2);

    else if (!strcmp(name, "set_port_tagged"))
        sfs_tpm_sw_set_port_tagged((MV_U32)param1 ,(MV_U32)param2 ,(MV_U8)param3);

    else if (!strcmp(name, "set_port_untagged"))
        sfs_tpm_sw_set_port_untagged((MV_U32)param1 ,(MV_U32)param2 ,(MV_U8)param3);

    else if (!strcmp(name, "set_port_def_vlan"))
        sfs_tpm_sw_set_port_def_vlan((MV_U32)param1 ,(MV_U32)param2 ,(MV_U16)param3);

    else if (!strcmp(name, "set_port_def_pri"))
        sfs_tpm_sw_set_port_def_pri((MV_U32)param1 ,(MV_U32)param2 ,(MV_U8)param3);

    else if (!strcmp(name, "add_port_vid"))
        sfs_tpm_sw_port_add_vid((MV_U32)param1 ,(MV_U32)param2 ,(MV_U16)param3);

    else if (!strcmp(name, "del_port_vid"))
        sfs_tpm_sw_del_port_vid((MV_U32)param1 ,(MV_U32)param2 ,(MV_U16)param3);

    else if (!strcmp(name, "add_port_gvid"))
        sfs_tpm_sw_port_add_gvid((MV_U32)param1 ,(MV_U32)param2 ,(MV_U16)param3, (MV_U16)param4, (MV_U16)param5);

    else if (!strcmp(name, "del_port_gvid"))
        sfs_tpm_sw_del_port_gvid((MV_U32)param1 ,(MV_U32)param2 ,(MV_U16)param3, (MV_U16)param4);

    else if (!strcmp(name, "set_port_vid_egr_mode"))
        sfs_tpm_sw_set_port_vid_egress_mode((MV_U32)param1 ,(MV_U32)param2 ,(MV_U16)param3 ,(MV_U8)param4);

    else if (!strcmp(name, "set_uni_q_weight"))
        sfs_tpm_sw_set_uni_q_weight((MV_U32)param1 ,(MV_U32)param2,(MV_U32)param3);

    else if (!strcmp(name, "set_vid_filter_per_port"))
        sfs_tpm_sw_set_vid_filter_per_port((MV_U32)param1 ,(MV_U32)param2 ,(MV_U8)param3);

    else if (!strcmp(name, "set_uni_egr_rate_limit"))
        sfs_tpm_sw_set_uni_egr_rate_limit((MV_U32)param1, (MV_U32)param2,
                                        (MV_U32)param3);

    else if (!strcmp(name, "set_uni_ingr_police_rate"))
        sfs_tpm_sw_set_uni_ingr_police_rate((MV_U32)param1, (MV_U32)param2,
                                            (MV_U32)param3, (MV_U32)param4,
                                            (MV_U32)param5, (MV_U32)param6);
    else if (!strcmp(name, "set_uni_tc_ingr_police_rate"))
        sfs_tpm_sw_set_uni_tc_ingr_police_rate((MV_U32)param1, (MV_U32)param2,
                                           (MV_U32)param3,(MV_U32)param4,
                                           (MV_U32)param5);
    else if
        (!strcmp(name, "set_ingress_limit_mode"))
                sfs_tpm_sw_set_ingress_limit_mode((MV_U32)param1,(MV_U32)param2,(MV_U32)param3);

    else if
        (!strcmp(name, "set_port_vlan_ports"))
                sfs_tpm_sw_set_port_vlan_ports((MV_U32)param1,(MV_U32)param2, memPorts,(MV_U32)param9);

    else if
        (!strcmp(name, "set_init_pri"))
                sfs_tpm_sw_set_priority_selection((MV_U32)param1,(MV_U8)param2,(MV_U8)param3,(MV_U8)param4);
    else if
        (!strcmp(name, "set_tag_pri_mapping"))
                sfs_tpm_sw_set_tag_pri_mapping((MV_U32)param1,(MV_U8)param2,(MV_U8)param3);
    else if
        (!strcmp(name, "set_ip_pri_mapping"))
                sfs_tpm_sw_set_ip_pri_mapping((MV_U32)param1,(MV_U8)param2,(MV_U8)param3);
    else if
        (!strcmp(name, "set_gmac_mtu"))
                sfs_tpm_sw_set_gmac_mtu((MV_U32)param1,(MV_U32)param2,(MV_U32)param3);

    else if
        (!strcmp(name, "set_port_mirror"))
                sfs_tpm_sw_set_port_mirror((MV_U32)param1,(MV_U32)param2,(MV_U32)param3,(MV_U32)param4,(MV_U32)param5);
    else if
        (!strcmp(name, "set_trunk_mask"))
                sfs_tpm_sw_set_trunk_mask((MV_U32)param1,(MV_U32)param2,(MV_U32)param3);
    else if
        (!strcmp(name, "set_trunk_ports"))
                sfs_tpm_sw_set_trunk_ports((MV_U32)param1,(MV_U32)param2,(MV_U32)param3);
    else if
        (!strcmp(name, "set_port_speed_duplex"))
                sfs_tpm_sw_set_port_speed_duplex((MV_U32)param1,(MV_U32)param2, (MV_U32)param3, (MV_U32)param4);
    else if
        (!strcmp(name, "add_port_vid_set_egr_mode"))
                sfs_tpm_sw_port_add_vid_set_egrs_mode((MV_U32)param1,(MV_U32)param2, (MV_U16)param3, (MV_U8)param4);
    else
        printk("%s: illegal operation <%s>\n", __FUNCTION__, attr->attr.name);

//     raw_local_irq_restore(flags);

    return(len);
}


/* ========================================================================== */
/*                            Debug SW Attributes                             */
/* ========================================================================== */
static DEVICE_ATTR(set_global_reg,          S_IWUSR, dbg_sw_show, dbg_sw_store);
static DEVICE_ATTR(get_global_reg,          S_IWUSR, dbg_sw_show, dbg_sw_store);
static DEVICE_ATTR(set_port_reg,            S_IWUSR, dbg_sw_show, dbg_sw_store);
static DEVICE_ATTR(get_port_reg,            S_IWUSR, dbg_sw_show, dbg_sw_store);
static DEVICE_ATTR(get_port_cntrs,          S_IWUSR, dbg_sw_show, dbg_sw_store);
static DEVICE_ATTR(get_fdb,                 S_IWUSR, dbg_sw_show, dbg_sw_store);
static DEVICE_ATTR(sw_init,                 S_IWUSR, dbg_sw_show, dbg_sw_store);
static DEVICE_ATTR(debug_trace,             S_IWUSR, dbg_sw_show, dbg_sw_store);
#ifdef CONFIG_MV_TPM_SYSFS_HELP
static DEVICE_ATTR(help_sw_dbg,             S_IRUSR, dbg_sw_show, dbg_sw_store);
#endif


static struct attribute *dbg_sw_attrs[] =
{
    &dev_attr_set_global_reg.attr,
    &dev_attr_get_global_reg.attr,
    &dev_attr_set_port_reg.attr,
    &dev_attr_get_port_reg.attr,
    &dev_attr_get_port_cntrs.attr,
    &dev_attr_get_fdb.attr,
    &dev_attr_sw_init.attr,
    &dev_attr_debug_trace.attr,
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_sw_dbg.attr,
#endif
    NULL
};

static struct attribute_group dbg_sw_group =
{
    .name = "debug",
    .attrs = dbg_sw_attrs
};



/* ========================================================================== */
/*                            Info SW Attributes                              */
/* ========================================================================== */

static DEVICE_ATTR(get_port_admin,                  S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_link_status,            S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_autoneg,                S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_duplex_config,          S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_duplex_status,          S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_fc_config,              S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_fc_status,              S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_speed_config,           S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_speed_status,           S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_isolate_vector,         S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_loopback,               S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_mac_age_time,                S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_max_macs,               S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_mac_learn,              S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_flooding,               S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_mc_flooding,            S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_bc_flooding,                 S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_tagged,                 S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_untagged,               S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_def_vlan,               S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_def_pri,                S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_vid,                    S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_vid_filter,             S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_uni_ingr_police_rate,        S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_uni_tc_ingr_police_rate,     S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_uni_egr_rate_limit,          S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_vlan_ports,             S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_ingress_limit_mode,          S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_init_pri,                    S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_tag_pri_mapping,             S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_ip_pri_mapping,              S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_gmac_mtu,                    S_IWUSR, get_sw_show, get_sw_store);
static DEVICE_ATTR(get_port_mirror,                 S_IWUSR, get_sw_show, get_sw_store);
#ifdef CONFIG_MV_TPM_SYSFS_HELP
static DEVICE_ATTR(help_sw_get_info,                S_IRUSR, get_sw_show, get_sw_store);
#endif


static struct attribute *get_sw_attrs[] =
{
    &dev_attr_get_port_admin.attr,
    &dev_attr_get_port_link_status.attr,
    &dev_attr_get_port_autoneg.attr,
    &dev_attr_get_port_duplex_config.attr,
    &dev_attr_get_port_duplex_status.attr,
    &dev_attr_get_port_fc_config.attr,
    &dev_attr_get_port_fc_status.attr,
    &dev_attr_get_port_speed_config.attr,
    &dev_attr_get_port_speed_status.attr,
    &dev_attr_get_port_isolate_vector.attr,
    &dev_attr_get_port_loopback.attr,
    &dev_attr_get_mac_age_time.attr,
    &dev_attr_get_port_max_macs.attr,
    &dev_attr_get_port_mac_learn.attr,
    &dev_attr_get_port_flooding.attr,
    &dev_attr_get_port_mc_flooding.attr,
    &dev_attr_get_bc_flooding.attr,
    &dev_attr_get_port_tagged.attr,
    &dev_attr_get_port_untagged.attr,
    &dev_attr_get_port_def_vlan.attr,
    &dev_attr_get_port_def_pri.attr,
    &dev_attr_get_port_vid.attr,
    &dev_attr_get_port_vid_filter.attr,
    &dev_attr_get_uni_ingr_police_rate.attr,
    &dev_attr_get_uni_tc_ingr_police_rate.attr,
    &dev_attr_get_uni_egr_rate_limit.attr,
    &dev_attr_get_port_vlan_ports.attr,
    &dev_attr_get_ingress_limit_mode.attr,
    &dev_attr_get_init_pri.attr,
    &dev_attr_get_tag_pri_mapping.attr,
    &dev_attr_get_ip_pri_mapping.attr,
    &dev_attr_get_gmac_mtu.attr,
    &dev_attr_get_port_mirror.attr,
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_sw_get_info.attr,
#endif
    NULL
};

static struct attribute_group get_sw_group =
{
    .name = "info",
    .attrs = get_sw_attrs
};


/* ========================================================================== */
/*                            Cfg SW Attributes                               */
/* ========================================================================== */
static DEVICE_ATTR(set_port_admin,                  S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_port_autoneg,                S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(restart_autoneg,                 S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_port_duplex,                 S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_port_fc,                     S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_port_speed,                  S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_port_isolate_vector,         S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_port_loopback,               S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(add_static_mac,                  S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(del_static_mac,                  S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(clear_dynamic_mac,               S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_static_mac_w_mask,           S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_mac_age_time,                S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_port_max_macs,               S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_port_mac_learn,              S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_port_flooding,               S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_port_mc_flooding,            S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_bc_flooding,                 S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_port_tagged,                 S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_port_untagged,               S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_port_def_vlan,               S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_port_def_pri,                S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(add_port_vid,                    S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(del_port_vid,                    S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(add_port_gvid,                   S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(del_port_gvid,                   S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_port_vid_egr_mode,           S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_vid_filter_per_port,         S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_uni_q_weight,                S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_uni_ingr_police_rate,        S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_uni_tc_ingr_police_rate,     S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_uni_egr_rate_limit,          S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_atu_size,                    S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_port_vlan_ports,             S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_ingress_limit_mode,          S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_init_pri,                    S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_tag_pri_mapping,             S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_ip_pri_mapping,              S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_gmac_mtu,                    S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_port_mirror,                 S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_trunk_mask,                  S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_trunk_ports,                 S_IWUSR, set_sw_show, set_sw_store);
#ifdef CONFIG_MV_TPM_SYSFS_HELP
static DEVICE_ATTR(help_sw_set_cfg_1,               S_IRUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(help_sw_set_cfg_2,               S_IRUSR, set_sw_show, set_sw_store);
#endif
static DEVICE_ATTR(rate_limit_menu,                 S_IRUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(egr_rate_limit_menu,             S_IRUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(init_pri_menu,                   S_IRUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(set_port_speed_duplex,           S_IWUSR, set_sw_show, set_sw_store);
static DEVICE_ATTR(add_port_vid_set_egr_mode,       S_IWUSR, set_sw_show, set_sw_store);

static struct attribute *set_sw_attrs[] = {
    &dev_attr_set_port_admin.attr,
    &dev_attr_set_port_autoneg.attr,
    &dev_attr_restart_autoneg.attr,
    &dev_attr_set_port_duplex.attr,
    &dev_attr_set_port_fc.attr,
    &dev_attr_set_port_speed.attr,
    &dev_attr_set_port_isolate_vector.attr,
    &dev_attr_set_port_loopback.attr,
    &dev_attr_add_static_mac.attr,
    &dev_attr_del_static_mac.attr,
    &dev_attr_clear_dynamic_mac.attr,
    &dev_attr_set_static_mac_w_mask.attr,
    &dev_attr_set_mac_age_time.attr,
    &dev_attr_set_port_max_macs.attr,
    &dev_attr_set_port_mac_learn.attr,
    &dev_attr_set_port_flooding.attr,
    &dev_attr_set_port_mc_flooding.attr,
    &dev_attr_set_bc_flooding.attr,
    &dev_attr_set_port_tagged.attr,
    &dev_attr_set_port_untagged.attr,
    &dev_attr_set_port_def_vlan.attr,
    &dev_attr_set_port_def_pri.attr,
    &dev_attr_add_port_vid.attr,
    &dev_attr_del_port_vid.attr,
    &dev_attr_add_port_gvid.attr,
    &dev_attr_del_port_gvid.attr,
    &dev_attr_set_port_vid_egr_mode.attr,
    &dev_attr_set_vid_filter_per_port.attr,
    &dev_attr_set_uni_q_weight.attr,
    &dev_attr_set_uni_ingr_police_rate.attr,
    &dev_attr_set_uni_tc_ingr_police_rate.attr,
    &dev_attr_set_uni_egr_rate_limit.attr,
    &dev_attr_set_atu_size.attr,
    &dev_attr_set_port_vlan_ports.attr,
    &dev_attr_set_ingress_limit_mode.attr,
    &dev_attr_set_init_pri.attr,
    &dev_attr_set_tag_pri_mapping.attr,
    &dev_attr_set_ip_pri_mapping.attr,
    &dev_attr_set_gmac_mtu.attr,
    &dev_attr_set_port_mirror.attr,
    &dev_attr_set_trunk_mask.attr,
    &dev_attr_set_trunk_ports.attr,
    &dev_attr_set_port_speed_duplex.attr,
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_sw_set_cfg_1.attr,
    &dev_attr_help_sw_set_cfg_2.attr,
#endif
    &dev_attr_rate_limit_menu.attr,
    &dev_attr_egr_rate_limit_menu.attr,
    &dev_attr_init_pri_menu.attr,
    &dev_attr_add_port_vid_set_egr_mode.attr,
    NULL
};

static struct attribute_group set_sw_group =
{
  .name = "cfg",
  .attrs = set_sw_attrs
};
/******************************************************************************/
/* ========================================================================== */
/*                            SW SYS FS                                       */
/* ========================================================================== */
/******************************************************************************/
int sw_sysfs_init(void)
{
    int err;
    struct device *pd;

    pd = bus_find_device_by_name(&platform_bus_type, NULL, "tpm_sw");
    if (!pd)
    {
        platform_device_register_simple("tpm_sw", -1, NULL, 0);
        pd = bus_find_device_by_name(&platform_bus_type, NULL, "tpm_sw");
    }

    if (!pd)
    {
        printk(KERN_ERR"%s: cannot find tpm_sw device\n", __FUNCTION__);
        pd = &platform_bus;
    }

    err = sysfs_create_group(&pd->kobj, &set_sw_group);
    if (err)
    {
        printk(KERN_INFO "sysfs group failed %d\n", err);
        goto out;
    }

    err = sysfs_create_group(&pd->kobj, &get_sw_group);
    if (err)
    {
        printk(KERN_INFO "sysfs group failed %d\n", err);
        goto out;
    }

    err = sysfs_create_group(&pd->kobj, &dbg_sw_group);
    if (err)
    {
        printk(KERN_INFO "sysfs group failed %d\n", err);
        goto out;
    }

    printk(KERN_INFO "= SW Module SYS FS Init ended successfully =\n");

    out:
    return err;
}

/*******************************************************************************
**
**  tpm_module_usr_if_create
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function create user interface - sys fs
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR
**
*******************************************************************************/
MV_STATUS tpm_module_usr_if_create(void)
{
  if (sw_sysfs_init() != 0)
    return(MV_ERROR);

  if (tpm_sysfs_init() != 0)
    return(MV_ERROR);

  if (tpm_sysfs_print_init() != 0)
    return(MV_ERROR);

  if (tpm_sysfs_trace_init() != 0)
    return(MV_ERROR);
#if 0 //See  tpm_sysfs_config_init module for explanation
  if (tpm_sysfs_config_init() != 0)
    return(MV_ERROR);
#endif

  return(MV_OK);
}


