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
* mv_tpm_sw_config.c
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
*       $Revision: 1.47 $
*******************************************************************************/

#include "tpm_common.h"
#include "tpm_header.h"
#include "../../mv_mac_learn/mv_mac_learn_header.h"

uint32_t trace_sw_dbg_flag = 0;

static tpm_swport_pm_1_t g_mc_tpm_swport_pm_1;
static tpm_swport_pm_3_all_t g_mc_tpm_swport_pm_3;

#define IF_ERROR(ret)\
    if (ret != TPM_DB_OK) {\
        printk(KERN_ERR "ERROR: (%s) call not allowed, switch not initialized\n", __FUNCTION__);\
        return(ret);\
    }

#define SWITCH_INIT_CHECK()\
    int32_t ret_init_check;\
    ret_init_check = tpm_sw_init_check();\
    IF_ERROR(ret_init_check)

/*******************************************************************************
* tpm_sw_init_check
*
* DESCRIPTION:
*       This function is to check switch is allowed to init or not
*
* INPUTS:
*       NONE
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
tpm_error_code_t tpm_sw_init_check(void)
{
    uint32_t switch_init;
    int32_t ret;

    ret = tpm_db_switch_init_get(&switch_init);
    IF_ERROR(ret);
    if (!switch_init)
        return ERR_SW_NOT_INIT;

    return TPM_RC_OK;
}

/*******************************************************************************
* tpm_phy_access_check
*
* DESCRIPTION:
*       This function check the PHY access path, direct or through switch.
*
* INPUTS:
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       phy_ctrl    - the PHY SMI master indication
*       phy_direct_addr - PHY address if PHY accessed directly
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
tpm_error_code_t tpm_phy_access_check(tpm_src_port_type_t src_port,
                                      tpm_phy_ctrl_t *phy_access_way,
                                      uint32_t *phy_direct_addr)
{
    tpm_init_gmac_conn_conf_t gmac_conn_info;
    uint32_t i;
    tpm_db_chip_conn_t chip_con;
    tpm_db_int_conn_t int_con;
    uint32_t switch_port;

    /* Para check*/
    if ((NULL == phy_access_way) || (NULL == phy_direct_addr)) {
        printk(KERN_ERR "ERROR: Invalid pointer\n");
        return ERR_GENERAL;
    }

    /* Check gmac port connection info */
    for (i = 0; i < TPM_MAX_NUM_GMACS; i++)
    {
        if (TPM_DB_OK != tpm_db_gmac_conn_conf_get(i, &gmac_conn_info))
        {
            printk(KERN_ERR "ERROR: (%s:%d) Gmac port(%d) connection info get failed\n", __FUNCTION__, __LINE__, i);
            return ERR_PHY_SRC_PORT_CONN_INVALID;
        }
        if (TPM_TRUE == gmac_conn_info.valid && src_port == gmac_conn_info.port_src)
        {
        	if(gmac_conn_info.conn == TPM_GMAC_CON_SGMII)
        	{
				/* No PHY available */
				*phy_access_way = PHY_SGMII;

				if (trace_sw_dbg_flag)
					printk(KERN_INFO "Port%d SGMII connection, no PHY\n", src_port, *phy_direct_addr);

				return TPM_RC_OK;
        	}
        	else // We keep the original code for the rest of cases
        	{
				/* PHY access directly */
				*phy_access_way = PHY_SMI_MASTER_CPU;
				/*get PHY addr on GMAC*/
				*phy_direct_addr = mvBoardPhyAddrGet(i);

				if (trace_sw_dbg_flag)
					printk(KERN_INFO "Port%d PHY access directly, phyaddr %d\n", src_port, *phy_direct_addr);

				return TPM_RC_OK;
        	}
        }
    }

    /* Check eth port connection info */
    if (TPM_DB_OK != tpm_db_eth_port_conf_get(src_port, &chip_con, &int_con, &switch_port)) {
        printk(KERN_ERR "ERROR: (%s:%d) Eth port(%d) connection info get failed\n", __FUNCTION__, __LINE__, src_port);
        return ERR_PHY_SRC_PORT_CONN_INVALID;
    }
    /* Check QSGMII */
    if (TPM_CONN_QSGMII == chip_con && TPM_INTCON_SWITCH == int_con) {
        /* Check MPP register, if value of MPP29 and MPP30 are both 4, then access PHY directly */
        if ((mvBoardMppGet(3) & DB_88F6535_MPP24_31) == DB_88F6535_MPP24_31) {
            /* PHY access directly */
            *phy_access_way = PHY_SMI_MASTER_CPU;
            /*get PHY addr on GMAC*/
            *phy_direct_addr = (uint32_t)src_port;
            if (trace_sw_dbg_flag)
                printk(KERN_INFO "Port%d QSGMII PHY access directly, phyaddr %d\n", src_port, *phy_direct_addr);
            return TPM_RC_OK;
        }
    }

    *phy_access_way = PHY_SMI_MASTER_SWT;
    if (trace_sw_dbg_flag)
        printk(KERN_INFO "Port%d PHY access through switch\n", src_port);

    return TPM_RC_OK;
}

tpm_error_code_t tpm_src_port_mac_map(tpm_src_port_type_t src_port,
                                      tpm_gmacs_enum_t *gmac)
{
    tpm_init_gmac_conn_conf_t gmac_conn_info;
    uint32_t i;

    /* Check gmac port connection info */
    for (i = 0; i < TPM_MAX_NUM_GMACS; i++) {
        if (TPM_DB_OK != tpm_db_gmac_conn_conf_get(i, &gmac_conn_info)) {
	    printk(KERN_ERR "ERROR: (%s:%d) Gmac port(%d) connection info get failed\n", __FUNCTION__, __LINE__, i);
	    return ERR_GENERAL;
        }
        if (TPM_TRUE == gmac_conn_info.valid && src_port == gmac_conn_info.port_src) {
            *gmac = i;
            return TPM_RC_OK;
        }
    }
    if (i == TPM_MAX_NUM_GMACS) {
        printk(KERN_ERR "ERROR: (%s:%d) src port(%d) map to MAC failed\n", __FUNCTION__, __LINE__, src_port);
	return ERR_SRC_PORT_INVALID;
    }

    return TPM_RC_OK;
}

/*******************************************************************************
* tpm_sw_set_gmac_speed_duplex_fc
*
* DESCRIPTION:
*       This function will update GMAC Port Auto-Negotiation Configuration Register if PHY is
*        connected to GMAC directly. Maily for Media Convert.
*
* INPUTS:
*       src_port         - Source port in UNI port index, UNI0, UNI1...UNI4.
*       auto_nego_en - Auto-negotiation enable status
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
*******************************************************************************/
static tpm_error_code_t tpm_sw_set_gmac_speed_duplex_fc(uint32_t owner_id,
                                                        tpm_src_port_type_t src_port,
                                                        bool auto_nego_en)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t switch_init;
    tpm_gmacs_enum_t gmac_port;
    uint32_t phy_direct_addr;
    tpm_phy_speed_t cfg_speed;
    bool  cfg_duplex, cfg_fc;

    retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
    if (retVal != TPM_RC_OK) {
        printk(KERN_ERR
               "Port%d PHY access way check failed\n", src_port);
        return retVal;
    }
    /* check swicth init or not */
    if (tpm_db_switch_init_get(&switch_init) != TPM_DB_OK) {
        printk(KERN_ERR "ERROR: (%s:%d) tpm_db_switch_init_get Failed\n", __FUNCTION__, __LINE__);
        retVal = ERR_GENERAL;
        return retVal;
    }
    /* The routine only apply PHY connected to GMAC */
    if ((PHY_SMI_MASTER_CPU == phy_access_way) &&
         (!switch_init)) {
        for (gmac_port = TPM_ENUM_GMAC_0; gmac_port < TPM_MAX_NUM_GMACS; gmac_port++) {
            if (phy_direct_addr == mvBoardPhyAddrGet(gmac_port))
            break;
        }
        if (gmac_port == TPM_MAX_NUM_GMACS) {
            printk(KERN_ERR "ERROR: (%s:%d) Can not find gmac port\n", __FUNCTION__, __LINE__);
            retVal = ERR_GENERAL;
            return retVal;
        }

        if (auto_nego_en) {/* auto-nego enable */
            if (mvNetaSpeedDuplexSet((int)gmac_port, MV_ETH_SPEED_AN, MV_ETH_DUPLEX_AN) != MV_OK) {
                printk(KERN_ERR "ERROR: (%s:%d) mvNetaSpeedDuplexSet Failed\n", __FUNCTION__, __LINE__);
                retVal = ERR_GENERAL;
                return retVal;
            }
            if (mvNetaFlowCtrlSet((int)gmac_port, MV_ETH_FC_AN_NO)  != MV_OK){
                printk(KERN_ERR "ERROR: (%s:%d) mvNetaFlowCtrlSet Failed\n", __FUNCTION__, __LINE__);
                retVal = ERR_GENERAL;
                return retVal;
            }
        } else {/* auto-nego disable */
            retVal = tpm_phy_get_port_speed(owner_id, src_port, &cfg_speed);
            if (retVal) {
                printk(KERN_ERR "ERROR: (%s:%d) PHY speed config get failed\n", __FUNCTION__, __LINE__);
                return retVal;
            }
            retVal = tpm_phy_get_port_duplex_mode(owner_id, src_port, &cfg_duplex);
            if (retVal) {
                printk(KERN_ERR "ERROR: (%s:%d) PHY duplex config get failed\n", __FUNCTION__, __LINE__);
                return retVal;
            }
            retVal = tpm_phy_get_port_flow_control_support(owner_id, src_port, &cfg_fc);
            if (retVal) {
                printk(KERN_ERR "ERROR: (%s:%d) PHY flow control config get failed\n", __FUNCTION__, __LINE__);
                return retVal;
            }
            if (mvNetaSpeedDuplexSet((int)gmac_port,
                                     (cfg_speed + MV_ETH_SPEED_10),
                                     (cfg_duplex ? MV_ETH_DUPLEX_FULL : MV_ETH_DUPLEX_HALF)) != MV_OK) {
                printk(KERN_ERR "ERROR: (%s:%d) mvNetaSpeedDuplexSet Failed\n", __FUNCTION__, __LINE__);
                retVal = ERR_GENERAL;
                return retVal;
            }
            if (mvNetaFlowCtrlSet((int)gmac_port, (cfg_fc ? MV_ETH_FC_ENABLE : MV_ETH_FC_DISABLE)) != MV_OK) {
                printk(KERN_ERR "ERROR: (%s:%d) mvNetaFlowCtrlSet Failed\n", __FUNCTION__, __LINE__);
                retVal = ERR_GENERAL;
                return retVal;
            }
        }
    }
    else if (PHY_SGMII != phy_access_way)
    {
        printk(KERN_ERR "PHY not connected to GMAC\n");
        retVal = ERR_GENERAL;
        return retVal;
    }

    return retVal;
}

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
     uint32_t enDis
)
{
    trace_sw_dbg_flag = enDis;
    return TPM_RC_OK;
}

/*******************************************************************************
* tpm_sw_add_static_mac
*
* DESCRIPTION:
*       This function creates a static MAC entry in the MAC address table for a
*       specific UNI port in the integrated switch
*
* INPUTS:
*       owner_id    - APP owner id, should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*       static_mac  - 6 byte network order MAC source address.
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
tpm_error_code_t tpm_sw_add_static_mac
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint8_t             static_mac[6]
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    tpm_gmacs_enum_t gmac_i;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d]\n\r \
               static_mac[%02x:%02x:%02x:%02x:%02x:%02x]\n\r",
              __FUNCTION__,owner_id,src_port, static_mac[0],static_mac[1],static_mac[2],static_mac[3],static_mac[4],static_mac[5]);
    }

    if (tpm_sw_init_check()) {
        if (tpm_src_port_mac_map(src_port, &gmac_i)) {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) map to mac failed\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }
        /* Check GMAC1 lpk status, if no lpk, no way to add static MAC */
#ifdef CONFIG_MV_MAC_LEARN
        if (tpm_db_gmac1_lpbk_en_get())
            retVal = mv_mac_learn_static_entry_add(&(static_mac[0]));
	else {
            printk(KERN_ERR "ERROR: (%s:%d) src port(%d) MAC learn not supported\n", __FUNCTION__, __LINE__, src_port);
            return ERR_GENERAL;
        }
#endif
    } else {
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_add_static_mac(lPort, &(static_mac[0]));
    }
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_del_static_mac
*
* DESCRIPTION:
*       This function removes an existing static MAC entry from the MAC address
*       table  in the integrated switch.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       static_mac  - 6byte network order MAC source address.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success  - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_del_static_mac
(
    uint32_t owner_id,
    uint8_t  static_mac[6]
)
{
    tpm_error_code_t retVal = TPM_RC_OK;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                 "==ENTER==%s: owner_id[%d]\n\r, \
                 static_mac[%02x:%02x:%02x:%02x:%02x:%02x]\n\r",
                 __FUNCTION__,owner_id,static_mac[0],static_mac[1],static_mac[2],static_mac[3],static_mac[4],static_mac[5]);
    }

    if (tpm_sw_init_check()) {
        /* Check GMAC1 lpk status, if no lpk, no way to add static MAC */
#ifdef CONFIG_MV_MAC_LEARN
        if (tpm_db_gmac1_lpbk_en_get())
            retVal = mv_mac_learn_static_entry_del(&(static_mac[0]));
        else {
            printk(KERN_ERR "ERROR: (%s:%d) MAC learn not supported\n", __FUNCTION__, __LINE__);
            return ERR_GENERAL;
        }
#endif
    } else {
        retVal = mv_switch_del_static_mac(&(static_mac[0]));
    }
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

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
*       On success  - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_static_mac_w_ports_mask
(
    uint32_t owner_id,
    uint32_t ports_mask,
    uint8_t  static_mac[6]
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
#ifdef CONFIG_MV_MAC_LEARN
    static uint32_t mask_check = 1;
#endif

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],ports_mask[0x%x]\n\r"\
               "static_mac[%02x:%02x:%02x:%02x:%02x:%02x]\n\r",
               __FUNCTION__,owner_id,
               ports_mask,
               static_mac[0],
               static_mac[1],
               static_mac[2],
               static_mac[3],
               static_mac[4],
               static_mac[5]);
    }

    if (tpm_sw_init_check()) {
        /* Check GMAC1 lpk status, if no lpk, no way to add static MAC */
#ifdef CONFIG_MV_MAC_LEARN
	if (tpm_db_gmac1_lpbk_en_get()) {
            if (ports_mask & mask_check)
                retVal = mv_mac_learn_static_entry_add(&(static_mac[0]));
        } else {
            printk(KERN_ERR "ERROR: (%s:%d) MAC learn not supported\n", __FUNCTION__, __LINE__);
            return ERR_GENERAL;
        }
#endif
    } else {
        retVal = mv_switch_mac_addr_set(&(static_mac[0]), 0, ports_mask, 1);
    }
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_set_trunk_ports
*
* DESCRIPTION:
*       This function creates trunk ports and trunk id
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
*       On success  - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_trunk_ports
(
    uint32_t owner_id,
    uint32_t trunk_id,
    uint32_t ports_mask
)
{
    tpm_error_code_t ret;
    uint16_t  reg_tmp = 0;
    uint32_t  switch_port = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d], trunk_id[0x%x], ports_mask[0x%x]\n\r",
               __FUNCTION__, owner_id, trunk_id, ports_mask);
    }

    if (trunk_id > SW_TRUNK_ID_MAX)
    {
       printk(KERN_INFO
               "==ENTER==%s: trunk_id[0x%x] is bigger than [0x%x]\n\r",
               __FUNCTION__,trunk_id, SW_TRUNK_ID_MAX);
    }

    for (switch_port = 0; switch_port <= TPM_SWITCH_NUM_PORTS; switch_port++)
    {
        if(0 == (ports_mask & (1 << switch_port)))
        {
	    continue;
        }

	/* get trunk reg from HW */
	ret = mv_switch_reg_read(switch_port, SW_TRUNK_ID_REG, MV_SWITCH_PORT_ACCESS, &reg_tmp);
	if (0 != ret) {
		TPM_OS_ERROR(TPM_INIT_MOD, "Fail to get trunk reg, ret(%d)\n", ret);
		return (TPM_FAIL);
	}

	/* set bits 8, 9 to 11, as trunk_id */
	SW_CLEAR_REG_BIT(reg_tmp, SW_TRUNK_ID_BIT_OFF, SW_TRUNK_ID_BIT_LEN);
	reg_tmp |= (trunk_id << SW_TRUNK_ID_BIT_OFF);
	/* set bit 14, as trunk bit */
	reg_tmp |= (1 << SW_TRUNK_BIT_OFF);
	ret = mv_switch_reg_write(switch_port, SW_TRUNK_ID_REG, MV_SWITCH_PORT_ACCESS, reg_tmp);
	if (0 != ret) {
		TPM_OS_ERROR(TPM_INIT_MOD, "Fail to set trunk reg, ret(%d)\n", ret);
		return (TPM_FAIL);
	}
    }

    /* get trunk mapping reg from HW */
    ret = mv_switch_reg_read(0, SW_TRUNK_MAPPING_REG, MV_SWITCH_GLOBAL2_ACCESS, &reg_tmp);
    if (0 != ret) {
	    TPM_OS_ERROR(TPM_INIT_MOD, "Fail to get trunk mapping reg, ret(%d)\n", ret);
	    return (TPM_FAIL);
    }

    /* set bits 11 to 14, as trunk_id */
    SW_CLEAR_REG_BIT(reg_tmp, SW_TRUNK_MAPPING_ID_BIT_OFF, SW_TRUNK_ID_BIT_LEN);
    reg_tmp |= (trunk_id << SW_TRUNK_MAPPING_ID_BIT_OFF);

    /* set bits 0 to 6, as trunk map */
    SW_CLEAR_REG_BIT(reg_tmp, SW_TRUNK_MAPPING_BIT_OFF, SW_TRUNK_MAPPING_BIT_LEN);
    reg_tmp |= (ports_mask << SW_TRUNK_MAPPING_BIT_OFF);

    /* set bits 15, as udpate bit */
    reg_tmp |= (1 << SW_REG_UPDATE_BIT_OFF);

    ret = mv_switch_reg_write(0, SW_TRUNK_MAPPING_REG, MV_SWITCH_GLOBAL2_ACCESS, reg_tmp);
    if (0 != ret) {
	    TPM_OS_ERROR(TPM_INIT_MOD, "Fail to set trunk mapping reg, ret(%d)\n", ret);
	    return (TPM_FAIL);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return ret;
}


/*******************************************************************************
* tpm_sw_set_trunk_mask
*
* DESCRIPTION:
*       This function sets trunk mask
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
*       On success  - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_trunk_mask
(
    uint32_t owner_id,
    uint32_t mask_num,
    uint32_t trunk_mask
)
{
    tpm_error_code_t ret;
    uint16_t  reg_tmp = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],mask_num[0x%x],trunk_mask[0x%x]\n\r",
               __FUNCTION__, owner_id, mask_num, trunk_mask);
    }

    if (mask_num > SW_TRUNK_MASK_NUM_MAX)
    {
       printk(KERN_INFO
               "==ENTER==%s: mask_num[0x%x] is bigger than [0x%x]\n\r",
               __FUNCTION__,mask_num, SW_TRUNK_MASK_NUM_MAX);
    }

    if (trunk_mask > SW_TRUNK_MASK_MAX)
    {
       printk(KERN_INFO
               "==ENTER==%s: trunk_mask[0x%x] is bigger than [0x%x]\n\r",
               __FUNCTION__,trunk_mask, SW_TRUNK_MASK_MAX);
    }

    /* get trunk mask reg from HW */
    ret = mv_switch_reg_read(0, SW_TRUNK_MASK_REG, MV_SWITCH_GLOBAL2_ACCESS, &reg_tmp);
    if (0 != ret) {
	    TPM_OS_ERROR(TPM_INIT_MOD, "Fail to get trunk mask reg, ret(%d)\n", ret);
	    return (TPM_FAIL);
    }

    /* set bits 11 to 14, as mask_num */
    SW_CLEAR_REG_BIT(reg_tmp, SW_TRUNK_MASK_NUM_BIT_OFF, SW_TRUNK_MASK_NUM_BIT_LEN);
    reg_tmp |= (mask_num << SW_TRUNK_MASK_NUM_BIT_OFF);

    /* set bits 0 to 6, as trunk_mask */
    SW_CLEAR_REG_BIT(reg_tmp, SW_TRUNK_MASK_BIT_OFF, SW_TRUNK_MASK_BIT_LEN);
    reg_tmp |= (trunk_mask << SW_TRUNK_MASK_BIT_OFF);

    /* set bits 15, as udpate bit */
    reg_tmp |= (1 << SW_REG_UPDATE_BIT_OFF);

    ret = mv_switch_reg_write(0, SW_TRUNK_MASK_REG, MV_SWITCH_GLOBAL2_ACCESS, reg_tmp);
    if (0 != ret) {
	    TPM_OS_ERROR(TPM_INIT_MOD, "Fail to set trunk mask reg, ret(%d)\n", ret);
	    return (TPM_FAIL);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return ret;
}

/*******************************************************************************
* tpm_sw_clear_dynamic_mac
*
* DESCRIPTION:
*       Clear all dynamic MAC.
*
* INPUTS:
*       owner_id   - APP owner id , should be used for all API calls.
*
* OUTPUTS:
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case , see tpm_error_code_t.
*
* COMMENTS:
*       None
*
*******************************************************************************/
tpm_error_code_t tpm_sw_clear_dynamic_mac(    uint32_t owner_id)
{
    tpm_error_code_t retVal;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d]\n\r", __FUNCTION__, owner_id);
    }

    retVal = mv_switch_clear_dynamic_mac();
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_set_port_mirror
*
* DESCRIPTION:
*       Set port mirror.
*
* INPUTS:
*       owner_id   - APP owner id , should be used for all API calls.
*       sport      - Source port.
*       dport      - Destination port.
*       mode       - mirror mode.
*       enable     - enable/disable mirror.
*
* OUTPUTS:
*       None
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case , see tpm_error_code_t.
*
* COMMENTS:
*       None
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_port_mirror
(
    uint32_t             owner_id,
    uint32_t             sport,
    uint32_t             dport,
    tpm_sw_mirror_type_t mode,
    bool                 enable
)
{
    tpm_error_code_t retVal;
    GT_BOOL  state;
    uint32_t  sw_sport;
    uint32_t  sw_dport;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d] sport[%d] dport[%d] mode[%d] enable[%d]\n\r",
               __FUNCTION__, owner_id, tpm_db_eth_port_switch_port_get(sport),
               tpm_db_eth_port_switch_port_get(dport), mode, enable);
    }

    if(enable == true)
        state = GT_TRUE;
    else
        state = GT_FALSE;

    sw_sport = tpm_db_eth_port_switch_port_get(sport);
    sw_dport = tpm_db_eth_port_switch_port_get(dport);

    if (TPM_DB_ERR_PORT_NUM == sw_sport || TPM_DB_ERR_PORT_NUM == sw_dport){
            TPM_OS_ERROR(TPM_INIT_MOD, "invalid port (sw_sport %d, sw_dport %d)\n", sw_sport, sw_dport);
	     return GT_FAIL;
    }

    retVal = mv_switch_set_mirror(sw_sport, sw_dport, (GT_MIRROR_MODE)mode, state);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_get_port_mirror
*
* DESCRIPTION:
*       Get port mirror status.
*
* INPUTS:
*       owner_id    - APP owner id , should be used for all API calls.
*       sport       - Source port.
*       dport       - Destination port.
*       mode        - mirror mode.
*
* OUTPUTS:
*       enable      - enable/disable mirror.
*
* RETURNS:
*       On success  - TPM_RC_OK.
*       On error different types are returned according to the case , see tpm_error_code_t.
*
* COMMENTS:
*       None
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_port_mirror
(
    uint32_t             owner_id,
    uint32_t             sport,
    uint32_t             dport,
    tpm_sw_mirror_type_t mode,
    bool                *enable
)
{
    tpm_error_code_t retVal;
    GT_BOOL  state;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d] sport[%d] dport[%d] mode[%d] \n\r",
               __FUNCTION__, owner_id, tpm_db_eth_port_switch_port_get(sport),
               tpm_db_eth_port_switch_port_get(dport), mode);
    }

    retVal = mv_switch_get_mirror(tpm_db_eth_port_switch_port_get(sport),
                                  tpm_db_eth_port_switch_port_get(dport),
                                  (GT_MIRROR_MODE)mode, &state);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if(state == GT_FALSE)
        *enable = false;
    else
        *enable = true;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s: enable[%d]\n\r",__FUNCTION__, *enable);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_set_isolate_eth_port_vector()
*
* DESCRIPTION:      Isolate port vector.
*
* INPUTS:
*       owner_id    - APP owner id  should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*       port_vector - port vector.
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_isolate_eth_port_vector
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint32_t            port_vector
)
{
#define MEM_PORTS_NR  7
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    uint32_t memPorts[MEM_PORTS_NR] = {1};
    uint32_t i = 0;
    uint8_t  memPortsLen = MEM_PORTS_NR;
    uint32_t sw_port_num;

    SWITCH_INIT_CHECK();

    memset(memPorts, 1, sizeof(memPorts));

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d] src_port[%d] port_vector[%d]\n\r",
               __FUNCTION__, owner_id, src_port, port_vector);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    /*Init port list*/
    for(i=0; i<MEM_PORTS_NR; i++)
    {
        memPorts[i] = i;
    }

    for (i=TPM_SRC_PORT_UNI_0; i<TPM_MAX_NUM_UNI_PORTS; i++)
    {
        sw_port_num = tpm_db_eth_port_switch_port_get(i);

        if (TPM_DB_ERR_PORT_NUM == sw_port_num)
            continue;

        if (sw_port_num >= MEM_PORTS_NR) {
            TPM_OS_ERROR(TPM_INIT_MOD, "sw_port_num too big! (%d >= MEM_PORTS_NR) \n", sw_port_num);
            return ERR_SRC_PORT_INVALID;
        }

        if(port_vector & (1 << i))
        {
            memPorts[sw_port_num] = sw_port_num;
        }
        else
        {
            memPorts[sw_port_num] = 0x80;
        }
    }

    /*set VLAN member*/
    retVal = mv_switch_set_port_vlan_ports(lPort, memPorts, memPortsLen);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

     if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s \n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_get_isolate_eth_port_vector()
*
* DESCRIPTION:      Isolate port vector.
*
* INPUTS:
*       owner_id    - APP owner id  should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       port_vector - port vector.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_isolate_eth_port_vector
(
    uint32_t             owner_id,
    tpm_src_port_type_t  src_port,
    uint32_t            *port_vector
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    uint32_t memPorts[7] = {0x80};
    uint32_t i = 0;
    uint8_t  memPortsLen = 7;
    uint32_t sw_port_num;
    uint32_t vector = 0;

    SWITCH_INIT_CHECK();

    memset(memPorts, 1, sizeof(memPorts));

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d] src_port[%d] \n\r",
               __FUNCTION__, owner_id, src_port);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    /*get VLAN member*/
    retVal = mv_switch_get_port_vlan_ports(lPort, memPorts, &memPortsLen);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    for(i=TPM_SRC_PORT_UNI_0; i<TPM_MAX_NUM_ETH_PORTS; i++)
    {
        sw_port_num = tpm_db_eth_port_switch_port_get(i);
	 if (TPM_DB_ERR_PORT_NUM == sw_port_num)
            continue;

        if(memPorts[sw_port_num] == sw_port_num)
        {
            vector |= (1 << (i-1));
        }
    }

    *port_vector = vector & 0x0000000F;

     if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s: port_vector[%d]\n\r",__FUNCTION__, *port_vector);
    }

    return retVal;
}

/*******************************************************************************
* tpm_set_mtu_size
*
* DESCRIPTION:
*       Set switch MTU size.
*
* INPUTS:
*       owner_id  - APP owner id , should be used for all API calls.
*       type      - MRU type:GMAC0, GMAC1, PONMAC, switch
*       mtu       - MTU size.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case , see tpm_error_code_t.
*
* COMMENTS:
*       None
*
*******************************************************************************/
tpm_error_code_t tpm_set_mtu_size
(
    uint32_t       owner_id,
    tpm_mru_type_t type,
    uint32_t       mtu
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    uint32_t switch_init = 0;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],type[%d],mtu[%d]\r\n",
               __FUNCTION__, owner_id, type, mtu);

    }

    switch(type)
    {
        case TPM_NETA_MTU_GMAC0:
              /*GMAC0 port number: 0*/
            retVal = mv_eth_set_mtu(0, mtu);
            break;
        case TPM_NETA_MTU_GMAC1:
            /*GMAC1 port number: 1*/
            retVal = mv_eth_set_mtu(1, mtu);
            break;
        case TPM_NETA_MTU_PONMAC:
            /*PON GMAC port number: 2*/
            retVal = mv_eth_set_mtu(2, mtu);
            break;
        case TPM_NETA_MTU_SWITCH:
            /*Set switch MTU if exist*/
            tpm_db_switch_init_get(&switch_init);
	    if (switch_init)
                retVal = mv_switch_set_mtu(mtu);
            break;
        default:
            break;
    }

    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_get_mtu_size
*
* DESCRIPTION:
*       Get switch MTU size.
*
* INPUTS:
*       owner_id   - APP owner id , should be used for all API calls.
*       type       - MRU type:GMAC0, GMAC1, PONMAC, switch
*
* OUTPUTS:
*       mtu        - MTU size.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case , see tpm_error_code_t.
*
* COMMENTS:
*       None
*
*******************************************************************************/
tpm_error_code_t tpm_get_mtu_size
(
    uint32_t        owner_id,
    tpm_mru_type_t  type,
    uint32_t       *mtu
)
{
    tpm_error_code_t retVal = TPM_RC_OK;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
              "==ENTER==%s: owner_id[%d],type[%d],",
              __FUNCTION__,owner_id, type);

    }

    switch(type)
    {
        case TPM_NETA_MTU_GMAC0:
              /*GMAC0 port number: 0*/
            retVal = mv_eth_get_mtu(0, mtu);
            break;
        case TPM_NETA_MTU_GMAC1:
            /*GMAC1 port number: 1*/
            retVal = mv_eth_get_mtu(1, mtu);
            break;
        case TPM_NETA_MTU_PONMAC:
            /*PON GMAC port number: 2*/
            retVal = mv_eth_get_mtu(2, mtu);
            break;
        case TPM_NETA_MTU_SWITCH:
            /*Get switch MTU if exist*/
            if (!tpm_sw_init_check())
                retVal = mv_switch_get_mtu(mtu);
            else
                retVal = ERR_GENERAL;
            break;
        default:
            break;
    }

    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:mtu[%d]\n\r",__FUNCTION__, *mtu);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_set_port_max_macs
*
* DESCRIPTION:
*       This function limits the number of MAC addresses per src_port.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       src_port     - Source port in UNI port index, UNI0, UNI1...UNI4.
*       mac_per_port - maximum number of MAC addresses per port (1-255).
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       The following care is needed when enabling this feature:
*            1) disable learning on the ports
*            2) flush all non-static addresses in the ATU
*            3) define the desired limit for the ports
*            4) re-enable learing on the ports
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_port_max_macs
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint8_t             mac_per_port
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    tpm_gmacs_enum_t gmac_i;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d],mac_per_port[%d]\r\n",
                __FUNCTION__, owner_id, src_port, mac_per_port);
    }

    if (tpm_sw_init_check()) {
        if (tpm_src_port_mac_map(src_port, &gmac_i)) {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) map to mac failed\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }
#ifdef CONFIG_MV_MAC_LEARN
        /* Check GMAC1 lpk status, if no lpk, no way to add static MAC */
        if (tpm_db_gmac1_lpbk_en_get())
            retVal = mv_mac_learn_max_count_set(mac_per_port);
        else {
            printk(KERN_ERR "ERROR: (%s:%d) src port(%d) MAC learn not supported\n", __FUNCTION__, __LINE__, src_port);
            return ERR_GENERAL;
        }
#endif
    } else {
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_set_port_max_macs(lPort, mac_per_port);
    }
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_get_port_max_macs
*
* DESCRIPTION:
*        Port's auto learning limit. When the limit is non-zero value, the number
*        of MAC addresses that can be learned on this UNI port are limited to the value
*        specified in this API. When the learn limit has been reached any frame
*        that ingresses this UNI port with a source MAC address not already in the
*        address database that is associated with this UNI port will be discarded.
*        Normal auto-learning will resume on the UNI port as soon as the number of
*        active unicast MAC addresses associated to this UNI port is less than the
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
*       owner_id   - APP owner id - should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*
*
* OUTPUTS:
*       limit      - maximum number of MAC addresses per UNI port (1-255).
*
* RETURNS:
*       On success - TPM_RC_OK.
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
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    tpm_gmacs_enum_t gmac_i;
    tpm_db_pnc_range_conf_t range_conf;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d]\r\n",
                __FUNCTION__,owner_id,src_port);
    }

    if (tpm_sw_init_check()) {
        if (tpm_src_port_mac_map(src_port, &gmac_i)) {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) map to mac failed\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }
        /* Check GMAC1 lpk status */
        if (tpm_db_gmac1_lpbk_en_get()) {
            if (tpm_db_pnc_rng_conf_get(TPM_PNC_MAC_LEARN, &range_conf)) {
                printk(KERN_ERR "ERROR: (%s:%d) tpm_db_pnc_rng_conf_get failed\n", __FUNCTION__, __LINE__);
                return ERR_GENERAL;
            }
            *limit = range_conf.api_end - range_conf.api_start + 1;
            if (*limit > MAC_LEARN_FDB_MAX_COUNT)
                *limit = MAC_LEARN_FDB_MAX_COUNT;
        } else {
            printk(KERN_ERR "ERROR: (%s:%d) src port(%d) MAC learn not supported\n", __FUNCTION__, __LINE__, src_port);
            return ERR_GENERAL;
        }
    } else {
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_get_port_max_macs(lPort, limit);
    }
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s: limit[%d]\n\r",
                __FUNCTION__,*limit);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_set_port_tagged
*
* DESCRIPTION:
*       The API allows or drops tagged packets on a per UNI port basis.
*
* INPUTS:
*       owner_id     - APP owner id - should be used for all API calls.
*       src_port     - Source port in UNI port index, UNI0, UNI1...UNI4.
*       drop_tagged - set to 1 = drop tagged packets per UNI port
*                      set to 0 = allow tagged packets per UNI port.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success   - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_port_tagged
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint8_t             drop_tagged
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL          mode;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
              "==ENTER==%s: owner_id[%d],src_port[%d],drop_tagged[%d]\r\n",
              __FUNCTION__,owner_id,src_port,drop_tagged);

    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    if (drop_tagged == 1)
    {
        mode = 1/*GT_TRUE*/;
    }
    else
    {
        mode = MV_FALSE;
    }

    retVal = mv_switch_set_port_tagged(lPort, mode);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_get_port_tagged
*
* DESCRIPTION:
*       This routine gets DiscardTagged bit for the given UNI port.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       mode       - MV_TRUE if DiscardTagged bit is set, MV_FALSE otherwise
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_port_tagged
(
    uint32_t             owner_id,
    tpm_src_port_type_t  src_port,
    uint32_t            *mode
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d]\r\n",
               __FUNCTION__,owner_id,src_port);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_get_port_tagged(lPort, mode);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:mode[%d]\n\r",__FUNCTION__,*mode);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_set_port_untagged
*
* DESCRIPTION:
*       The API allows or drops untagged packets on a per UNI port basis.
*
* INPUTS:
*       owner_id       - APP owner id - should be used for all API calls.
*       src_port       - Source port in UNI port index, UNI0, UNI1...UNI4.
*       drop_untagged - set to 1 = drop untagged packets per UNI port
*                        set to 0 = alow untagged packets per UNI port.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success     - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_port_untagged
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint8_t             drop_untagged
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL          mode;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d],drop_untagged[%d]\r\n",
                __FUNCTION__,owner_id,src_port,drop_untagged);
    }

    if (drop_untagged == 1)
    {
        mode = 1/*GT_TRUE*/;
    }
    else
    {
        mode = MV_FALSE;
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_set_port_untagged(lPort, mode);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_get_port_untagged
*
* DESCRIPTION:
*       This routine gets DiscardUntagged bit for the given UNI port.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       mode        - MV_TRUE if DiscardUntagged bit is set, MV_FALSE otherwise
*
* RETURNS:
*       On success  - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_port_untagged
(
    uint32_t             owner_id,
    tpm_src_port_type_t  src_port,
    uint32_t            *mode
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d]\r\n",
                __FUNCTION__,owner_id,src_port);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_get_port_untagged(lPort, mode);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:mode[%d]\n\r",
                __FUNCTION__,*mode);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_set_port_def_vlan
*
* DESCRIPTION:
*       The API sets port default vlan id.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*       vid        - the port vlan id.
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
tpm_error_code_t tpm_sw_set_port_def_vlan
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint16_t            vid
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d],vid[%d]\r\n",
                __FUNCTION__,owner_id,src_port,vid);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    if (vid > 4095)
    {
        printk(KERN_ERR
            "%s:illegal VID[%d]\r\n", __FUNCTION__, vid);
        return ERR_SW_VID_INVALID;
    }

    retVal = mv_switch_set_port_def_vlan(lPort, vid);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_get_port_def_vlan
*
* DESCRIPTION:
*       The API gets port default vlan id.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       vid        - the port vlan id
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_port_def_vlan
(
    uint32_t             owner_id,
    tpm_src_port_type_t  src_port,
    uint16_t            *vid
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d]\r\n",
                __FUNCTION__,owner_id,src_port);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_get_port_def_vlan(lPort, vid);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:vid[%d]\n\r",
                __FUNCTION__,*vid);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_set_port_def_pri
*
* DESCRIPTION:
*       The API sets port default priority.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*       pri        - the port priority.
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
tpm_error_code_t tpm_sw_set_port_def_pri
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint8_t             pri
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d],pri[%d]\r\n",
                __FUNCTION__,owner_id,src_port,pri);
    }

    if (pri>7)
    {
        printk(KERN_ERR
           "%s:illegal pri[%d]\r\n", __FUNCTION__, pri);
        return ERR_GENERAL;
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_set_port_def_pri(lPort, pri);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_get_port_def_pri
*
* DESCRIPTION:
*       The API gets port default priority.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       pri        - the port priority.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_port_def_pri
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint8_t            *pri
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d]\r\n",
                __FUNCTION__,owner_id,src_port);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_get_port_def_pri(lPort, pri);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:pri[%d]\n\r",
                __FUNCTION__,*pri);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_port_add_vid
*
* DESCRIPTION:
*       The API adds a VID to the list of the allowed VIDs per UNI port.
*
* INPUTS:
*       owner_id - APP owner id - should be used for all API calls.
*       src_port - Source port in UNI port index, UNI0, UNI1...UNI4.
*       vid      - VLAN id.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       see the example sample802_1qSetup().
*
*******************************************************************************/
tpm_error_code_t tpm_sw_port_add_vid
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint16_t            vid
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d],vid[%d]\n",
                __FUNCTION__,owner_id,src_port,vid);
    }

    if (vid >= TPM_MAX_VID)
    {
        printk(KERN_INFO
               "%s:%d:==ERROR== invalid VID[%d]\r\n", __FUNCTION__,__LINE__,vid);
        return ERR_SW_VID_INVALID;
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_port_add_vid(lPort, vid, TPM_GMAC0_AMBER_PORT_NUM);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_clear_vid_per_port
*
* DESCRIPTION:
*       The API delete all VID from the list of VIDs allowed per UNI port.
*
* INPUTS:
*       owner_id   - APP owner id should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_clear_vid_per_port
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    uint16_t         vid;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d]\r\n",
                __FUNCTION__,owner_id,src_port);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    for(vid=1; vid < TPM_MAX_VID-1; vid++)
    {
        retVal = mv_switch_del_vid_per_port(lPort, vid);
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        }
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_add_all_vid_per_port
*
* DESCRIPTION:
*       The API adds all allowed VIDs from 1 to 4095 per UNI port.
*
* INPUTS:
*       owner_id   - APP owner id should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case , see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_add_all_vid_per_port
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    uint16_t         vid;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d]\r\n",
                __FUNCTION__,owner_id,src_port);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    for(vid=1; vid < TPM_MAX_VID-1; vid++)
    {
        retVal = mv_switch_port_add_vid(lPort, vid, TPM_GMAC0_AMBER_PORT_NUM);
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        }
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_port_del_vid
*
* DESCRIPTION:
*       The API delete and existing VID from the list of VIDs allowed per UNI port.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*       vid        - VLAN id.
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
tpm_error_code_t tpm_sw_port_del_vid
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint16_t            vid
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d],vid[%d]\r\n",
                __FUNCTION__, owner_id, src_port, vid);
    }

    if (vid >= TPM_MAX_VID)
    {
        printk(KERN_INFO
               "%s:%d:==ERROR== invalid VID[%d]\r\n", __FUNCTION__,__LINE__,vid);
        return ERR_SW_VID_INVALID;
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_del_vid_per_port(lPort, vid);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_set_port_vid_egress_mode
*
* DESCRIPTION:
*       The API sets the egress mode for a member port of a vlan.
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
tpm_error_code_t tpm_sw_set_port_vid_egress_mode
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint16_t            vid,
    uint8_t             eMode
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d],vid[%d],eMode[%d]\r\n",
                __FUNCTION__,owner_id,src_port,vid,eMode);
    }

    if (vid >= TPM_MAX_VID)
    {
        printk(KERN_INFO
               "%s:%d:==ERROR== invalid VID[%d]\r\n", __FUNCTION__,__LINE__,vid);
        return ERR_SW_VID_INVALID;
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_set_port_vid_egress_mode(lPort, vid, eMode);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n",__FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_port_get_vid
*
* DESCRIPTION:
*       The API return VID to the list of the allowed VIDs per UNI port.
*
* INPUTS:
*       owner_id   - APP owner id should be used for all API calls.
*       vid        -  searching VID.
*
* OUTPUTS:
*       found      - MV_TRUE, if the appropriate entry exists.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case see tpm_error_code_t.
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
)
{
    tpm_error_code_t retVal;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],vid[%d]\r\n",
                __FUNCTION__,owner_id,vid);
    }

    if (vid >= TPM_MAX_VID)
    {
        printk(KERN_INFO
               "%s:%d:==ERROR== invalid VID[%d]\r\n", __FUNCTION__,__LINE__,vid);
        return ERR_SW_VID_INVALID;
    }

    retVal = mv_switch_port_print_vid(vid, found);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s: found[%d]\n\r",__FUNCTION__,*found);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_port_add_vid_group()
*
* DESCRIPTION:      Add a group of VID to the list of the allowed VIDs per port,
*                    and set the egress mode correspondingly.
*
* INPUTS:
*       owner_id - APP owner id  should be used for all API calls.
*       src_port - Source port in UNI port index, UNI0, UNI1...UNI4.
*       mode     - VLAN egress mode.
*       min_vid  - min VLAN ID.
*       max_vid  - max VLAN ID.
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_port_add_vid_group
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint8_t             mode,
    uint16_t            min_vid,
    uint16_t            max_vid
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    uint16_t         vid;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d], src_port[%d], mode[%d], min_vid[%d] max_vid[%d]\r\n",
                __FUNCTION__,owner_id, src_port, mode, min_vid, max_vid);
    }

    /*check VID*/
    if (max_vid >= TPM_MAX_VID)
    {
        printk(KERN_INFO
               "%s:%d:==ERROR== invalid max_vid[%d]\r\n", __FUNCTION__,__LINE__,max_vid);
        return ERR_SW_VID_INVALID;
    }

    if(min_vid > max_vid)
    {
        printk(KERN_INFO
               "%s:%d:==ERROR==  param error: min_vid[%d] > max_vid[%d] \r\n", __FUNCTION__,__LINE__, min_vid, max_vid);
        return ERR_SW_VID_INVALID;
    }

    for(vid=min_vid; vid<=max_vid; vid++)
    {
        /*set the VID*/
        retVal = tpm_sw_port_add_vid(owner_id, src_port, vid);
        if(retVal !=TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
            break;
        }

        /*set VLAN egress mode*/
        retVal = tpm_sw_set_port_vid_egress_mode(owner_id, src_port, vid, mode);
        if(retVal !=TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
            break;
        }
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_port_del_vid_group()
*
* DESCRIPTION:      Delete a group of VID to the list of the allowed VIDs per port,
*                    and set the egress mode correspondingly.
*
* INPUTS:
*       owner_id - APP owner id  should be used for all API calls.
*       src_port - Source port in UNI port index, UNI0, UNI1...UNI4.
*       min_vid  - min VLAN ID.
*       max_vid  - max VLAN ID.
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_port_del_vid_group
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint16_t            min_vid,
    uint16_t            max_vid
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    uint16_t         vid;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d], src_port[%d], min_vid[%d] max_vid[%d]\r\n",
                __FUNCTION__,owner_id, src_port, min_vid, max_vid);
    }

    /*check VID*/
    if (max_vid >= TPM_MAX_VID)
    {
        printk(KERN_INFO
               "%s:%d:==ERROR== invalid max_vid[%d]\r\n", __FUNCTION__,__LINE__,max_vid);
        return ERR_SW_VID_INVALID;
    }

    if(min_vid > max_vid)
    {
        printk(KERN_INFO
               "%s:%d:==ERROR==  param error: min_vid[%d] > max_vid[%d] \r\n", __FUNCTION__,__LINE__, min_vid, max_vid);
        return ERR_SW_VID_INVALID;
    }

    for(vid=min_vid; vid<=max_vid; vid++)
    {
        /*Delete the VID*/
        retVal = tpm_sw_port_del_vid(owner_id, src_port, vid);
        if(retVal !=TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
            break;
        }
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_prv_set_secure_mode
*
* DESCRIPTION:
*       Change a port mode in the SW data base and remove it from all VLANs
*
* INPUTS:
*       owner_id   -  APP owner id - should be used for all API calls.
*       port       -  secure port number
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
tpm_error_code_t tpm_sw_prv_set_secure_mode
(
    uint32_t owner_id,
    uint32_t port
)
{
    tpm_error_code_t retVal;

    SWITCH_INIT_CHECK();

    retVal = mv_switch_prv_set_secure_mode(port);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
              "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_prv_set_fallback_mode
*
* DESCRIPTION:
*       Change a port mode in the SW data base and add it to all VLANs
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       port       - secure port number
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
tpm_error_code_t tpm_sw_prv_set_fallback_mode
(
    uint32_t owner_id,
    uint32_t port
)
{
    tpm_error_code_t retVal;

    SWITCH_INIT_CHECK();

    retVal = mv_switch_prv_set_fallback_mode(port);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
              "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_port_set_vid_filter
*
* DESCRIPTION:
*       The API sets the filtering mode of a certain UNI port.
*       If the UNI port is in filtering mode, only the VIDs added by the
*       tpm_sw_port_add_vid API will be allowed to ingress and egress the UNI port.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*       vid_filter - set to 1 - means the UNI port will DROP all packets which are NOT in
*                    the allowed VID list (built using API tpm_sw_port_add_vid).
*                    set to 0 - means that the list of VIDs allowed
*                    per UNI port has no significance (the list is not deleted).
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
tpm_error_code_t tpm_sw_port_set_vid_filter
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    uint8_t             vid_filter
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                 "==ENTER==%s: owner_id[%d],src_port[%d],vid_filter[%d]\r\n",
                 __FUNCTION__,owner_id,src_port, vid_filter);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_set_vid_filter_per_port(lPort, vid_filter);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
              "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_get_vid_filter_per_port
*
* DESCRIPTION:
*        This routine gets protected mode of a UNI port port.
*        When this mode is set to GT_TRUE, frames are allowed to egress UNI port
*        defined by the 802.1Q VLAN membership for the frame's VID 'AND'
*        by the UNI port's VLANTable if 802.1Q is enabled on the UNI port. Both must
*        allow the frame to Egress.
*
* INPUTS:
*       owner_id    - APP owner id - should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*         mode - GT_TRUE: header mode enabled
*                   GT_FALSE otherwise
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
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    uint8_t          vid_filter;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                 "==ENTER==%s: owner_id[%d],src_port[%d]\r\n",
                 __FUNCTION__,owner_id,src_port);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_get_vid_filter_per_port(lPort, &vid_filter);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    *mode = (uint32_t)vid_filter;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s: mode[%d] \n\r",__FUNCTION__, *mode);
    }

    return TPM_RC_OK;
}

/*******************************************************************************
* tpm_sw_set_uni_sched()
*
* DESCRIPTION:      Configures the scheduling mode per Ethernet port.
*
* INPUTS:
*       owner_id    - APP owner id  should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*       sched_mode  - scheduler mode per port
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*DDD
*******************************************************************************/
tpm_error_code_t tpm_sw_set_uni_sched
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    tpm_sw_sched_type_t sched_mode
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    uint32_t         mode;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d],sched_mode[%d]\n\r",
               __FUNCTION__,owner_id,src_port,sched_mode);
    }

    switch(sched_mode)
    {
        case  TPM_PP_SCHED_STRICT:
            mode = PORT_SCHED_MODE_SPRI;
            break;

        case TPM_PP_SCHED_WRR:
            mode = PORT_SCHED_MODE_WRRB;
            break;

       default:
            printk(KERN_INFO "==Error== %s: sched_mode[1 or 2]\n\r",__FUNCTION__);
            return ERR_GENERAL;
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_set_port_sched_mode(lPort, mode);
    if (retVal != GT_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_set_uni_q_weight
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
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.  DDD
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_uni_q_weight
(
    uint32_t owner_id,
    uint8_t  queue_id,
    uint8_t  weight
)
{
    tpm_error_code_t retVal = GT_OK;
    tpm_gmacs_enum_t gmac_i;
    tpm_db_gmac_func_t gmac_func;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],queue_id[%d] weight[%d]\n\r",
                __FUNCTION__,owner_id,queue_id,weight);
    }

    if (tpm_sw_init_check()) {
        for (gmac_i = TPM_ENUM_GMAC_0; gmac_i < TPM_MAX_NUM_GMACS; gmac_i++) {
            tpm_db_gmac_func_get(gmac_i, &gmac_func);
            if (TPM_GMAC_FUNC_LAN_UNI == gmac_func ||
                TPM_GMAC_FUNC_US_MAC_LEARN_DS_LAN_UNI == gmac_func)
                retVal |= mvNetaTxqWrrPrioSet(gmac_i, 0, queue_id, weight);
        }
    } else {
        if ((queue_id == 0) ||
            (queue_id > SW_QOS_NUM_OF_QUEUES))
        {
            printk(KERN_INFO
                   "%s:%d:==ERROR== invalid queue[%d]\r\n", __FUNCTION__,__LINE__,queue_id);
            return ERR_SW_TM_QUEUE_INVALID;
        }

        retVal = mv_switch_set_uni_q_weight(queue_id, weight);
    }
    if (retVal != GT_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_set_uni_ingr_police_rate
*
* DESCRIPTION:
*       The API Configures an ingress policing function for an Ethernet UNI port.
*
* INPUTS:
*       owner_id   - APP owner id, should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*       count_mode - count mode:
*                               TPM_SW_LIMIT_FRAME
*                               TPM_SW_LIMIT_LAYER1
*                               TPM_SW_LIMIT_LAYER2
*                               TPM_SW_LIMIT_LAYER3
*       cir        - comited info rate.
*       cbs        - Committed Burst Size limit (expected to be 2kBytes)
*       ebs        - Excess Burst Size limit ( 0 ~ 0xFFFFFF)
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
tpm_error_code_t tpm_sw_set_uni_ingr_police_rate
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    tpm_limit_mode_t    count_mode,
    uint32_t            cir,
    uint32_t            cbs,
    uint32_t            ebs
)
{
    tpm_error_code_t    retVal = TPM_RC_OK;
    int32_t             lPort  = 0;
    GT_PIRL2_COUNT_MODE mode;
    tpm_gmacs_enum_t    gmac_i;
    tpm_db_gmac_lpk_uni_ingr_rate_limit_t rate_limit;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d],count_mode[%d], cir[%d], cbs[%d], ebs[%d]\r\n",
                __FUNCTION__,owner_id,src_port,count_mode, cir, cbs,ebs);
    }

    if (tpm_sw_init_check()) {
        if (tpm_src_port_mac_map(src_port, &gmac_i)) {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) map to mac failed\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }
        /* Check GMAC1 lpk status, if no lpk, no ingress rate for ingress */
        if (tpm_db_gmac1_lpbk_en_get()) {
            retVal = tpm_tm_set_gmac0_ingr_rate_lim(owner_id, cir, cbs);
            if (retVal == TPM_RC_OK) {
                rate_limit.count_mode = count_mode;
                rate_limit.cir = cir;
                rate_limit.cbs = cbs;
                rate_limit.ebs = ebs;
                retVal = tpm_db_gmac_lpk_uni_ingr_rate_limit_set(src_port, rate_limit);
            }
        } else {
            printk(KERN_ERR "ERROR: (%s:%d) src port(%d) ingr rate limit not support\n", __FUNCTION__, __LINE__, src_port);
            return ERR_GENERAL;
        }
    } else {

        mode = (GT_PIRL2_COUNT_MODE)count_mode;

        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_set_uni_ingr_police_rate(lPort, (GT_PIRL2_COUNT_MODE)mode, cir, cbs, ebs);
    }
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s \n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_get_uni_ingr_police_rate
*
* DESCRIPTION:
*       The API gets an ingress policing function for an Ethernet UNI port.
*
* INPUTS:
*       owner_id - APP owner id, should be used for all API calls.
*       src_port - Source port in UNI port index, UNI0, UNI1...UNI4.
*
*
* OUTPUTS:
*       count_mode - count mode:
*                               TPM_SW_LIMIT_FRAME
*                               TPM_SW_LIMIT_LAYER1
*                               TPM_SW_LIMIT_LAYER2
*                               TPM_SW_LIMIT_LAYER3
*       cir       - comited info rate.
*       cbs       - Committed Burst Size limit (expected to be 2kBytes)
*       ebs       - Excess Burst Size limit ( 0 ~ 0xFFFFFF)
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_uni_ingr_police_rate
(
    uint32_t             owner_id,
    tpm_src_port_type_t  src_port,
    tpm_limit_mode_t    *count_mode,
    uint32_t            *cir,
    uint32_t            *cbs,
    uint32_t            *ebs
)
{
    tpm_error_code_t    retVal = TPM_RC_OK;
    int32_t             lPort  = 0;
    GT_PIRL2_COUNT_MODE mode;
    tpm_gmacs_enum_t    gmac_i;
    tpm_db_gmac_lpk_uni_ingr_rate_limit_t rate_limit;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d]\r\n",
                __FUNCTION__,owner_id,src_port);
    }

    if (tpm_sw_init_check()) {
        if (tpm_src_port_mac_map(src_port, &gmac_i)) {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) map to mac failed\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }
        /* Check GMAC1 lpk status, if no lpk, no ingress rate for ingress */
        if (tpm_db_gmac1_lpbk_en_get()) {
            retVal = tpm_db_gmac_lpk_uni_ingr_rate_limit_get(src_port, &rate_limit);
            if (retVal == TPM_RC_OK) {
                mode = (GT_PIRL2_COUNT_MODE)rate_limit.count_mode;
                *cir = rate_limit.cir;
                *cbs = rate_limit.cbs;
                *ebs = rate_limit.ebs;
            }
        } else {
            printk(KERN_ERR "ERROR: (%s:%d) src port(%d) ingr rate limit not support\n", __FUNCTION__, __LINE__, src_port);
            return ERR_GENERAL;
        }
    } else {
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_get_uni_ingr_police_rate(lPort, &mode, cir, cbs, ebs);
    }
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    *count_mode = (tpm_limit_mode_t)mode;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:,count_mode[%d], cir[%d], cbs[%d], ebs[%d]\n\r",
                __FUNCTION__, *count_mode, *cir, *cbs, *ebs);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_set_uni_tc_ingr_police_rate
*
* DESCRIPTION:
*       The API Configures a policer function for a traffic class for an Ethernet UNI port.
*       There are 4 globally defined traffic classes in the integrated switch.
*
* INPUTS:
*       owner_id - APP owner id - should be used for all API calls.
*       src_port - Source port in UNI port index, UNI0, UNI1...UNI4.
*       tc       - traffic class ( a combination of p-bits and DSCP values).
*       cir      - comited info rate.
*       cbs      - comited burst rate.
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
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_uni_tc_ingr_police_rate
(
    uint32_t             owner_id,
    tpm_src_port_type_t  src_port,
    uint32_t             tc,
    uint32_t             cir,
    uint32_t             cbs
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d],tc[%d],cir[%d],cbs[%d]\r\n",
                __FUNCTION__,owner_id,src_port,tc, cir, cbs);
    }

    if ((tc == 0) || (tc > SW_QOS_NUM_OF_QUEUES))
    {
        printk(KERN_INFO
               "%s:%d:==ERROR== invalid tc[%d]\r\n", __FUNCTION__,__LINE__,tc);
        return ERR_SW_TM_QUEUE_INVALID;
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_set_uni_tc_ingr_police_rate(lPort, tc, cir, cbs);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_get_uni_tc_ingr_police_rate
*
* DESCRIPTION:
*       This routine gets the UNI port's ingress data limit for priority 0 or 1 or 2 or 3 frames.
*
* INPUTS:
*       owner_id - APP owner id - should be used for all API calls.
*       src_port - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*
*       tc       - traffic class ( a combination of p-bits and DSCP values).
*       cir      - comited info rate.
*       cbs      - comited burst rate
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
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d]\r\n",
                __FUNCTION__,owner_id,src_port);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_get_uni_tc_ingr_police_rate(lPort, tc, cir, cbs);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:tc[0x%x] cir[%d]\n\r",
               __FUNCTION__,*tc,*cir);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_set_uni_egr_rate_limit
*
* DESCRIPTION:
*       The API Configures the egress frame rate limit of an Ethernet UNI port
* INPUTS:
*       owner_id             - APP owner id - should be used for all API calls.
*       src_port             - Source port in UNI port index, UNI0, UNI1...UNI4.
*       mode                 - frame/rate limit mode
*       frame_rate_limit_val - egress rate limit value.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       GT_ERATE_TYPE used kbRate - frame rate valid values are:
*                                    7600,..., 9600,
*                                    10000, 20000, 30000, 40000, ..., 100000,
*                                    110000, 120000, 130000, ..., 1000000.
*
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_uni_egr_rate_limit
(
    uint32_t             owner_id,
    tpm_src_port_type_t  src_port,
    tpm_limit_mode_t     mode,
    uint32_t             frame_rate_limit_val
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    tpm_gmacs_enum_t gmac_i;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d],mode[%d],frame_rate_limit_val[%d]\r\n",
                __FUNCTION__,owner_id,src_port,mode,frame_rate_limit_val);
    }

    if (tpm_sw_init_check()) {
        if (tpm_src_port_mac_map(src_port, &gmac_i)) {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) map to mac failed\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }
        /* Set tx port rate limit on gmac_i */
        retVal = tpm_tm_set_tx_port_rate_lim(owner_id, gmac_i, frame_rate_limit_val, 0);
        if (retVal == TPM_RC_OK) {
            retVal = tpm_db_gmac_uni_egr_rate_limit_set(src_port, frame_rate_limit_val);
        }
    } else {

        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_set_uni_egr_rate_limit(lPort, (GT_PIRL_ELIMIT_MODE)mode, frame_rate_limit_val);
    }
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_get_uni_egr_rate_limit
*
* DESCRIPTION:
*       The API return the egress frame rate limit of an Ethernet UNI port
* INPUTS:
*       owner_id        - APP owner id - should be used for all API calls.
*       src_port        - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       mode            - frame/rate limit mode
*       rate_limit_val  - egress rate limit value..
*
* RETURNS:
*       On success - TPM_RC_OK.
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
)
{
    tpm_error_code_t    retVal = TPM_RC_OK;
    int32_t             lPort  = 0;
    GT_PIRL_ELIMIT_MODE limit_mode;
    tpm_gmacs_enum_t    gmac_i;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d],src_port[%d]\r\n",
                __FUNCTION__,owner_id,src_port);
    }

    if (tpm_sw_init_check()) {
        if (tpm_src_port_mac_map(src_port, &gmac_i)) {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) map to mac failed\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }
        /* Set tx port rate limit on gmac_i */
        retVal = tpm_db_gmac_uni_egr_rate_limit_get(src_port, frame_rate_limit_val);
        limit_mode = GT_PIRL_ELIMIT_FRAME;
    } else {
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_get_uni_egr_rate_limit(lPort, &limit_mode, frame_rate_limit_val);
    }
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }
    *mode = (tpm_limit_mode_t)limit_mode;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s: mode[%d], frame_rate_limit_val[%d]\n\r",
                __FUNCTION__, *mode, *frame_rate_limit_val);
    }

    return retVal;
}

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
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        The device GT_88E6351 have fixed ATU size 8192.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_atu_size
(
    uint32_t owner_id,
    uint32_t size
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    uint32_t         atu_size;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],size[%d]\n\r",
               __FUNCTION__,owner_id,size);
    }

    switch(size)
    {
        case 0: atu_size = 0; /*ATU_SIZE_256*/  break;
        case 1: atu_size = 1; /*ATU_SIZE_512*/  break;
        case 2: atu_size = 2; /*ATU_SIZE_1024*/ break;
        case 3: atu_size = 3; /*ATU_SIZE_2048*/ break;
        case 4: atu_size = 4; /*ATU_SIZE_4096*/ break;
        case 5:
            printk(KERN_INFO "8192 entries not supported by driver\n");break;

        default:
            printk(KERN_INFO "==Error== %s: size[0-4]\n\r",__FUNCTION__);
            return ERR_GENERAL;
    }

    /* The driver support only 8192 entry as constant value */
    printk(" The device GT_88E6351 have fixed ATU size 8192\r\n");

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_set_mac_age_time
*
* DESCRIPTION:
*       This function Sets the Mac address table size.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       time_out   - Aging Time value
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
tpm_error_code_t tpm_sw_set_mac_age_time
(
    uint32_t owner_id,
    uint32_t time_out
)
{
    tpm_error_code_t retVal = TPM_RC_OK;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],time_out[%d]\n\r",
               __FUNCTION__,owner_id,time_out);
    }

    if (tpm_sw_init_check()) {
        /* Check GMAC1 lpk status, if no lpk, no way to add static MAC */
#ifdef CONFIG_MV_MAC_LEARN
        if (tpm_db_gmac1_lpbk_en_get())
            retVal = mv_mac_learn_expire_time_set(time_out);
        else {
            printk(KERN_ERR "ERROR: (%s:%d) MAC learn not supported\n", __FUNCTION__, __LINE__);
            return ERR_GENERAL;
        }
#endif
    } else {
        retVal = mv_switch_set_age_time(time_out);
    }
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_get_mac_age_time
*
* DESCRIPTION:
*       This function Sets the Mac address table size.
*
* INPUTS:
*       owner_id   - APP owner id , should be used for all API calls.
*
* OUTPUTS:
*       time_out   - time out value.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case , see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_mac_age_time
(
     uint32_t  owner_id,
     uint32_t *time_out
)
{
    tpm_error_code_t retVal = TPM_RC_OK;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d]\n\r",
               __FUNCTION__,owner_id);
    }

    if (tpm_sw_init_check()) {
        /* Check GMAC1 lpk status, if no lpk, no way to add static MAC */
#ifdef CONFIG_MV_MAC_LEARN
        if (tpm_db_gmac1_lpbk_en_get())
            retVal = mv_mac_learn_expire_time_get(time_out);
        else {
            printk(KERN_ERR "ERROR: (%s:%d) MAC learn not supported\n", __FUNCTION__, __LINE__);
            return ERR_GENERAL;
        }
#endif
    } else {
        retVal = mv_switch_get_age_time(time_out);
    }
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:,time_out[%d]\n\r",__FUNCTION__,*time_out);
    }

    return retVal;
}

/*******************************************************************************
*  tpm_sw_set_mac_learn
*
* DESCRIPTION:
*       Enable/disable automatic learning of new source MAC addresses on port
*       ingress.
*
* INPUTS:
*       owner_id  - APP owner id , should be used for all API calls.
*       src_port  - Source port in UNI port index, UNI0, UNI1...UNI4.
*       enable    - GT_TRUE for enable  or GT_FALSE otherwise
*
* OUTPUTS:
*       None
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case, see tpm_error_code_t.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
tpm_error_code_t  tpm_sw_set_mac_learn
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    bool                enable
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL          state;
    tpm_gmacs_enum_t gmac_i;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d], src_port[%d] enable[%d]\n\r",
               __FUNCTION__,owner_id, src_port, enable);
    }

    if(enable == true)
        state = GT_TRUE;
    else
        state = GT_FALSE;

    if (tpm_sw_init_check()) {
        if (tpm_src_port_mac_map(src_port, &gmac_i)) {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) map to mac failed\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }
#ifdef CONFIG_MV_MAC_LEARN
        /* Check GMAC1 lpk status, if no lpk, no way to add static MAC */
        if (tpm_db_gmac1_lpbk_en_get())
            retVal = mv_mac_learn_enable_set(state);
        else {
            printk(KERN_ERR "ERROR: (%s:%d) src port(%d) MAC learn not supported\n", __FUNCTION__, __LINE__, src_port);
            return ERR_GENERAL;
        }
#endif
    } else {
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_set_mac_learn(lPort, state);
    }
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
*  tpm_sw_get_mac_learn
*
* DESCRIPTION:
*       Enable/disable automatic learning of new source MAC addresses on port
*       ingress.
*
* INPUTS:
*       owner_id  - APP owner id , should be used for all API calls.
*       src_port  - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       enable    - GT_TRUE for enable  or GT_FALSE otherwise
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case, see tpm_error_code_t.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
tpm_error_code_t  tpm_sw_get_mac_learn
(
    uint32_t             owner_id,
    tpm_src_port_type_t  src_port,
    bool               *enable
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL          state;
    tpm_gmacs_enum_t gmac_i;
    bool             mac_learn_enable = false;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d], src_port[%d] \n\r",
               __FUNCTION__,owner_id, src_port);
    }

    if (tpm_sw_init_check()) {
        if (tpm_src_port_mac_map(src_port, &gmac_i)) {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) map to mac failed\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }
        /* Check GMAC1 lpk status, if no lpk, no way to add static MAC */
#ifdef CONFIG_MV_MAC_LEARN
        if (tpm_db_gmac1_lpbk_en_get())
            retVal = mv_mac_learn_enable_get(&mac_learn_enable);
        else {
            printk(KERN_ERR "ERROR: (%s:%d) src port(%d) MAC learn not supported\n", __FUNCTION__, __LINE__, src_port);
            return ERR_GENERAL;
        }
        if(mac_learn_enable)
            state = GT_TRUE;
        else
            state = GT_FALSE;
#endif
    } else {
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_get_mac_learn(lPort, &state);
    }
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if(state == GT_TRUE)
        *enable = true;
    else
        *enable = false;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:enable[%d]\n\r",__FUNCTION__, *enable);
    }

    return retVal;
}

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
*       On success - TPM_RC_OK.
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
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d] memPorts[%02d %02d %02d %02d %02d %02d] \
               memPortsLen[%d]\n\r",
               __FUNCTION__,
              owner_id,
              src_port,
              memPorts[0],
              memPorts[1],
              memPorts[2],
              memPorts[3],
              memPorts[4],
              memPorts[5],
              memPortsLen);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_set_port_vlan_ports(lPort, memPorts, memPortsLen);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

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
*       On success - TPM_RC_OK.
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
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d]\n\r",
               __FUNCTION__,owner_id,src_port);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_get_port_vlan_ports(lPort, memPorts, memPortsLen);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT==%s: memPorts[%02d %02d %02d %02d %02d %02d] memPortsLen[%d]\n\r",
               __FUNCTION__,
               memPorts[0],
               memPorts[1],
               memPorts[2],
               memPorts[3],
               memPorts[4],
               memPorts[5],
               *memPortsLen);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_set_ingress_limit_mode
*
* DESCRIPTION:
*       This routine sets the port's rate control ingress limit mode.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*       mode       - rate control ingress limit mode.
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
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
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d],mode[%d]\n\r",
               __FUNCTION__,owner_id,src_port,mode);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_set_ingress_limit_mode(lPort, mode);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

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
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d]\n\r",
               __FUNCTION__,owner_id,src_port);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_get_ingress_limit_mode(lPort, mode);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s: mode[%d]\n\r",__FUNCTION__,*mode);
    }

    return retVal;
}

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
*       On success  - TPM_RC_OK.
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
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL          lTag;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==ENTER==%s: owner_id[%d] src_port[%d] init_pri[%d]\n\r",
               __FUNCTION__,owner_id,src_port,init_pri);
    }

    if (init_pri >= PRI_SEL_TYPE_MAX)
    {
        printk(KERN_INFO
               "%s:%d: invalid initial priority\r\n", __FUNCTION__,__LINE__);
        return ERR_GENERAL;
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    if(true == tag_if_both)
        lTag = GT_TRUE;
    else
        lTag = GT_FALSE;

    retVal = mv_switch_set_priority_selection(lPort,init_pri,lTag);

    if (retVal != TPM_RC_OK)
    {
        printk(KERN_INFO
               "%s:%d: function failed\r\n",__FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

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
*       On success  - TPM_RC_OK.
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
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL          lTag;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==ENTER==%s: owner_id[%d] src_port[%d]\n\r",
               __FUNCTION__,owner_id,src_port);
    }

    if (NULL == init_pri)
    {
        printk(KERN_INFO
               "%s:%d: null pointer\r\n", __FUNCTION__,__LINE__);
        return ERR_GENERAL;
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_get_priority_selection(lPort,init_pri,&lTag);

    if (retVal != TPM_RC_OK)
    {
        printk(KERN_INFO
               "%s:%d: function failed\r\n",__FUNCTION__,__LINE__);
    }

    if(GT_TRUE == lTag)
       *tag_if_both = true;
    else
       *tag_if_both = false;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==EXIT== %s: init_pri[%d]\n\r",__FUNCTION__,*init_pri);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_set_tag_pri_mapping
*
* DESCRIPTION:
*       This function maps a tag priority to a queue priority.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       tag_pri    - Source tag priority number.
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
*       tag_pri 0-7.
*       q_pri   0-3.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_tag_pri_mapping
(
     uint32_t owner_id,
     uint8_t  tag_pri,
     uint8_t  q_pri
)
{
    tpm_error_code_t retVal;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==ENTER==%s: owner_id[%d] tag_pri[%d] q_pri[%d]\n\r",
               __FUNCTION__,owner_id,tag_pri,q_pri);
    }

    if (tag_pri > SW_QOS_NUM_OF_TAG_PRI)
    {
        printk(KERN_INFO
               "%s:%d: invalid tag priority\r\n", __FUNCTION__,__LINE__);
        return ERR_GENERAL;
    }

    if (q_pri > SW_QOS_NUM_OF_QUEUES)
    {
        printk(KERN_INFO
               "%s:%d: invalid queue priority\r\n", __FUNCTION__,__LINE__);
        return ERR_GENERAL;
    }

    retVal = mv_switch_set_tag_pri_mapping(tag_pri,q_pri);

    if (retVal != TPM_RC_OK)
    {
        printk(KERN_INFO
               "%s:%d: function failed\r\n",__FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

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
)
{
    tpm_error_code_t retVal;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==ENTER==%s: owner_id[%d] tag_pri[%d]\n\r",
               __FUNCTION__,owner_id,tag_pri);
    }

    if (NULL == q_pri)
    {
        printk(KERN_INFO
               "%s:%d: null pointer\r\n", __FUNCTION__,__LINE__);
        return ERR_GENERAL;
    }

    if (tag_pri > SW_QOS_NUM_OF_TAG_PRI)
    {
        printk(KERN_INFO
               "%s:%d: invalid tag priority\r\n", __FUNCTION__,__LINE__);
        return ERR_GENERAL;
    }

    retVal = mv_switch_get_tag_pri_mapping(tag_pri,q_pri);

    if (retVal != TPM_RC_OK)
    {
        printk(KERN_INFO
               "%s:%d: function failed\r\n",__FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==EXIT== %s: q_pri[%d]\n\r",__FUNCTION__,*q_pri);
    }

    return retVal;
}

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
)
{
    tpm_error_code_t retVal;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==ENTER==%s: owner_id[%d] dscp[0x%x] q_pri[%d]\n\r",
               __FUNCTION__,owner_id,dscp,q_pri);
    }

    if (dscp > SW_QOS_DSCP_MAX)
    {
        printk(KERN_INFO
               "%s:%d: invalid dscp number\r\n", __FUNCTION__,__LINE__);
        return ERR_GENERAL;
    }

    if (q_pri > SW_QOS_NUM_OF_QUEUES)
    {
        printk(KERN_INFO
               "%s:%d: invalid queue priority\r\n", __FUNCTION__,__LINE__);
        return ERR_GENERAL;
    }

    retVal = mv_switch_set_ip_pri_mapping(dscp,q_pri);

    if (retVal != TPM_RC_OK)
    {
        printk(KERN_INFO
               "%s:%d: function failed\r\n",__FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

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
)
{
    tpm_error_code_t retVal;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==ENTER==%s: owner_id[%d] dscp[%d]\n\r",
               __FUNCTION__,owner_id,dscp);
    }

    if (NULL == q_pri)
    {
        printk(KERN_INFO
               "%s:%d: null pointer\r\n", __FUNCTION__,__LINE__);
        return ERR_GENERAL;
    }

    if (dscp > SW_QOS_DSCP_MAX)
    {
        printk(KERN_INFO
               "%s:%d: invalid tag priority\r\n", __FUNCTION__,__LINE__);
        return ERR_GENERAL;
    }

    retVal = mv_switch_get_ip_pri_mapping(dscp,q_pri);

    if (retVal != TPM_RC_OK)
    {
        printk(KERN_INFO
               "%s:%d: function failed\r\n",__FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==EXIT== %s: q_pri[%d]\n\r",__FUNCTION__,*q_pri);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_set_broadcast_flood
*
* DESCRIPTION:
*       This function decides whether the switch always floods the broadcast
*       frames to all portsr or uses the multicast egress mode (per port).
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       always_on  - always floods the broadcast regardless the multicast egress mode.
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
)
{
    tpm_error_code_t retVal;
    GT_BOOL          lFloodOn;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==ENTER==%s: owner_id[%d] always_on[%d]\n\r",
               __FUNCTION__,owner_id,always_on);
    }

    if(true == always_on)
        lFloodOn = GT_TRUE;
    else
        lFloodOn = GT_FALSE;

    retVal = mv_switch_set_broadcast_flood(lFloodOn);

    if (retVal != TPM_RC_OK)
    {
        printk(KERN_INFO
               "%s:%d: function failed\r\n",__FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}


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
*       always_on  - always floods the broadcast regardless the multicast egress mode.
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
)
{
    tpm_error_code_t retVal;
    GT_BOOL          lFloodOn;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==ENTER==%s: owner_id[%d]\n\r",
               __FUNCTION__,owner_id);
    }

    retVal = mv_switch_get_broadcast_flood(&lFloodOn);

    if (retVal != TPM_RC_OK)
    {
        printk(KERN_INFO
               "%s:%d: function failed\r\n",__FUNCTION__,__LINE__);
    }

    if(GT_TRUE == lFloodOn)
        *always_on = true;
    else
        *always_on = false;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==EXIT== %s: always_on[%d]\n\r",__FUNCTION__,*always_on);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_set_port_flooding()
*
* DESCRIPTION:      permit or not the flooding per port
*
* INPUTS:
*   owner_id    - APP owner id  should be used for all API calls.
*   src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*   flood_mode  - flooding mode.
*   allow_flood - set to 1 = permit flooding of unknown DA.
*
* OUTPUTS:
*   None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_port_flooding
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    tpm_flood_type_t    flood_mode,
    uint8_t             allow_flood
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL          enable;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==ENTER==%s: owner_id[%d] src_port[%d] flood_mode[%d], allow_flood[%d]\n\r",
               __FUNCTION__, owner_id, src_port, flood_mode, allow_flood);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    if(allow_flood == 1)
    {
        enable = GT_TRUE;
    }
    else
    {
        enable = GT_FALSE;
    }

    switch(flood_mode)
    {
        case TPM_FLOOD_UNKNOWN_UNICAST:
            retVal = mv_switch_set_unknown_unicast_flood(lPort, enable);
            break;
        case TPM_FLOOD_UNKNOWN_MULTI_BROAD_CAST:
            retVal = mv_switch_set_unknown_multicast_flood(lPort, enable);
            break;
        default:
            break;
    }
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_INFO
               "%s:%d: function failed\r\n",__FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_get_port_flooding()
*
* DESCRIPTION:      permit or not the flooding per port
*
* INPUTS:
*   owner_id    - APP owner id  should be used for all API calls.
*   src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*   flood_mode  - flooding mode
*
* OUTPUTS:
*   allow_flood - set to 1 = permit flooding .
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_port_flooding
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    tpm_flood_type_t    flood_mode,
    uint8_t            *allow_flood
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL          enable;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==ENTER==%s: owner_id[%d] src_port[%d] flood_mode[%d]\n\r",
               __FUNCTION__, owner_id, src_port, flood_mode);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    switch(flood_mode)
    {
        case TPM_FLOOD_UNKNOWN_UNICAST:
            retVal = mv_switch_get_unknown_unicast_flood(lPort, &enable);
            break;
        case TPM_FLOOD_UNKNOWN_MULTI_BROAD_CAST:
            retVal = mv_switch_get_unknown_multicast_flood(lPort, &enable);
            break;
        default:
            break;
    }

    if (retVal != TPM_RC_OK)
    {
        printk(KERN_INFO
               "%s:%d: function failed\r\n",__FUNCTION__,__LINE__);
    }

    if(enable == GT_TRUE)
    {
        *allow_flood = 1;
    }
    else
    {
        *allow_flood = 0;
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==EXIT== %s: allow_flood[%d]\n\r",__FUNCTION__, *allow_flood);
    }

    return retVal;
}

/*******************************************************************************
* tpm_phy_convert_port_index()
*
* DESCRIPTION: convert switch port index to external port index.
*
* INPUTS:
* owner_id       - APP owner id  should be used for all API calls.
* switch_port    - switch port index
*
* OUTPUTS:
* extern_port    - external port index
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_phy_convert_port_index (uint32_t             owner_id,
                                             uint32_t             switch_port,
                                             tpm_src_port_type_t *extern_port)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    tpm_init_gmac_conn_conf_t gmac_conn_info;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==ENTER==%s: owner_id[%d] switch_port[%d]\n\r",
               __FUNCTION__, owner_id, switch_port);
    }

    if (tpm_sw_init_check()) {
        if (tpm_db_gmac_conn_conf_get((tpm_gmacs_enum_t)switch_port, &gmac_conn_info)) {
            printk(KERN_ERR "ERROR: (%s:%d) switch_port(%d) invalid\n", __FUNCTION__, __LINE__, switch_port);
            return ERR_GENERAL;
        }
        if (TPM_TRUE == gmac_conn_info.valid)
            *extern_port = gmac_conn_info.port_src;
        else
            *extern_port = TPM_DB_ERR_PORT_NUM;
    } else {
        *extern_port = tpm_db_phy_convert_port_index(switch_port);
    }
    if (*extern_port == (tpm_src_port_type_t)TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) switch_port(%d) is invalid\n", __FUNCTION__, __LINE__, switch_port);
        return ERR_SRC_PORT_INVALID;
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==EXIT== %s: extern_port[%d]\n\r",__FUNCTION__, *extern_port);
    }

    return retVal;
}

/*******************************************************************************
* tpm_phy_set_port_autoneg_mode
*
* DESCRIPTION:
*       The API Configures the auto negotiation state of an Ethernet UNI port.
* INPUTS:
*       owner_id       - APP owner id  should be used for all API calls.
*       src_port       - Source port in UNI port index, UNI0, UNI1...UNI4.
*       autoneg_state  - autonegotiation state, enabled or disabled.
*       autoneg_mode   - enum:
*                        TPM_SPEED_AUTO_DUPLEX_AUTO: Auto for both speed and duplex
*                        TPM_SPEED_1000_DUPLEX_AUTO: 1000Mbps and auto duplex
*                        TPM_SPEED_100_DUPLEX_AUTO:  100Mbps and auto duplex
*                        TPM_SPEED_10_DUPLEX_AUTO:   10Mbps and auto duplex
*                        TPM_SPEED_AUTO_DUPLEX_FULL: Auto for speed only and Full duplex
*                        TPM_SPEED_AUTO_DUPLEX_HALF: Auto for speed only and Half duplex. (1000Mbps is not supported)
*                        TPM_SPEED_1000_DUPLEX_FULL: 1000Mbps Full duplex.
*                        TPM_SPEED_1000_DUPLEX_HALF: 1000Mbps half duplex.
*                        TPM_SPEED_100_DUPLEX_FULL:  100Mbps Full duplex.
*                        TPM_SPEED_100_DUPLEX_HALF:  100Mbps half duplex.
*                        TPM_SPEED_10_DUPLEX_FULL:   10Mbps Full duplex.
*                        TPM_SPEED_10_DUPLEX_HALF:   10Mbps half duplex.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case, see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
tpm_error_code_t tpm_phy_set_port_autoneg_mode
(
    uint32_t              owner_id,
    tpm_src_port_type_t   src_port,
    bool                  autoneg_state,
    tpm_autoneg_mode_t    autoneg_mode
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL  state, prev_state;
    GT_PHY_AUTO_MODE prev_mode;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;
    uint16_t         lsp_mode = PHY_AUTO_NEGO_MODE_HALF_10 |
                                PHY_AUTO_NEGO_MODE_FULL_10 |
                                PHY_AUTO_NEGO_MODE_HALF_100 |
                                PHY_AUTO_NEGO_MODE_FULL_100;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
              "==ENTER==%s: owner_id[%d],src_port[%d],autoneg_state[%d],autoneg_mode[%d]\n\r",
              __FUNCTION__, owner_id, src_port, autoneg_state, autoneg_mode);
    }

    if(true == autoneg_state)
        state = GT_TRUE;
    else
        state = GT_FALSE;

    switch (autoneg_mode) {
    case TPM_SPEED_AUTO_DUPLEX_AUTO:
        lsp_mode = PHY_AUTO_NEGO_MODE_HALF_10 |
                   PHY_AUTO_NEGO_MODE_FULL_10 |
                   PHY_AUTO_NEGO_MODE_HALF_100 |
                   PHY_AUTO_NEGO_MODE_FULL_100 |
                   PHY_AUTO_NEGO_MODE_HALF_1000 |
                   PHY_AUTO_NEGO_MODE_FULL_1000;
        break;
    case TPM_SPEED_1000_DUPLEX_AUTO:
        lsp_mode = PHY_AUTO_NEGO_MODE_HALF_1000 |
                   PHY_AUTO_NEGO_MODE_FULL_1000;
        break;
    case TPM_SPEED_100_DUPLEX_AUTO:
        lsp_mode = PHY_AUTO_NEGO_MODE_FULL_100 |
                   PHY_AUTO_NEGO_MODE_HALF_100;
        break;
    case TPM_SPEED_10_DUPLEX_AUTO:
        lsp_mode = PHY_AUTO_NEGO_MODE_FULL_10 |
                   PHY_AUTO_NEGO_MODE_HALF_10;
        break;
    case TPM_SPEED_AUTO_DUPLEX_FULL:
        lsp_mode = PHY_AUTO_NEGO_MODE_FULL_10 |
		   PHY_AUTO_NEGO_MODE_FULL_100 |
		    PHY_AUTO_NEGO_MODE_FULL_1000;
        break;
    case TPM_SPEED_AUTO_DUPLEX_HALF:
        lsp_mode = PHY_AUTO_NEGO_MODE_HALF_1000 |
                   PHY_AUTO_NEGO_MODE_HALF_100 |
                   PHY_AUTO_NEGO_MODE_HALF_10;
        break;
    case TPM_SPEED_1000_DUPLEX_FULL:
        lsp_mode = PHY_AUTO_NEGO_MODE_FULL_1000;
        break;
    case TPM_SPEED_1000_DUPLEX_HALF:
        lsp_mode = PHY_AUTO_NEGO_MODE_HALF_1000;
        break;
    case TPM_SPEED_100_DUPLEX_FULL:
        lsp_mode = PHY_AUTO_NEGO_MODE_FULL_100;
        break;
    case TPM_SPEED_100_DUPLEX_HALF:
        lsp_mode = PHY_AUTO_NEGO_MODE_HALF_100;
        break;
    case TPM_SPEED_10_DUPLEX_FULL:
        lsp_mode = PHY_AUTO_NEGO_MODE_FULL_10;
        break;
    case TPM_SPEED_10_DUPLEX_HALF:
        lsp_mode = PHY_AUTO_NEGO_MODE_HALF_10;
        break;
    default:
        printk(KERN_ERR "ERROR: (%s:%d) invalid autoneg_mode\n", __FUNCTION__, __LINE__);
        return ERR_GENERAL;
    }
    /* check PHY access way */
    retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
    if (retVal != TPM_RC_OK) {
        printk(KERN_ERR
               "Port%d PHY access way check failed\n", src_port);
        return retVal;
    }
    // Possible connection ways:
    //  -RGMII phy accessed directly
    //  -RGMII phy accessed through a switch
    //  -SGMII - No autoneg available
    /* PHY accessed directly, call lsp API */
    if (PHY_SMI_MASTER_CPU == phy_access_way) {
        if (mvEthPhyAutoNegoSet(phy_direct_addr, state)) {
            printk(KERN_ERR
                   "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
            retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
        }

        if(retVal == TPM_RC_OK )
        {
        	// Enable always advertisement
			if (mvEthPhyAdvertiseSet(phy_direct_addr, lsp_mode)) {
				printk(KERN_ERR
					   "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
				retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
			}
        }
        /* Update GMAC speed, duplex and flow control */
        retVal = tpm_sw_set_gmac_speed_duplex_fc(owner_id, src_port, (state ? true : false));
        if (retVal != TPM_RC_OK) {
            printk(KERN_ERR "Failed to update GMAC config with port%d\n", src_port);
            return retVal;
        }
    }
    else if (PHY_SMI_MASTER_SWT == phy_access_way)
    {
        /* PHY accessed through switch, as original do */
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        if (TPM_RC_OK == (retVal = mv_switch_get_port_autoneg_mode(lPort, &prev_state, &prev_mode)))
        {
            if (prev_state != state || prev_mode != (GT_PHY_AUTO_MODE)autoneg_mode)
            {
                retVal = mv_switch_set_port_autoneg_mode(lPort, state, (GT_PHY_AUTO_MODE)autoneg_mode);
                if (retVal != TPM_RC_OK)
                {
                    printk(KERN_ERR
                           "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
                }
            }
        }
    }
    else if (PHY_SGMII == phy_access_way)
    {  /* nothing to do*/ }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_phy_get_port_autoneg_mode
*
* DESCRIPTION:
*       The API return the the auto negotiation state of an Ethernet  UNI port.
* INPUTS:
*       owner_id       - APP owner id  should be used for all API calls.
*       src_port       - Source port in UNI port index, UNI0, UNI1...UNI4.
*
*
* OUTPUTS:
*       autoneg_state  - autonegotiation state, enabled or disabled.
*       autoneg_mode   - enum:
*                        TPM_SPEED_AUTO_DUPLEX_AUTO: Auto for both speed and duplex
*                        TPM_SPEED_1000_DUPLEX_AUTO: 1000Mbps and auto duplex
*                        TPM_SPEED_100_DUPLEX_AUTO:  100Mbps and auto duplex
*                        TPM_SPEED_10_DUPLEX_AUTO:   10Mbps and auto duplex
*                        TPM_SPEED_AUTO_DUPLEX_FULL: Auto for speed only and Full duplex
*                        TPM_SPEED_AUTO_DUPLEX_HALF: Auto for speed only and Half duplex. (1000Mbps is not supported)
*                        TPM_SPEED_1000_DUPLEX_FULL: 1000Mbps Full duplex.
*                        TPM_SPEED_1000_DUPLEX_HALF: 1000Mbps half duplex.
*                        TPM_SPEED_100_DUPLEX_FULL:  100Mbps Full duplex.
*                        TPM_SPEED_100_DUPLEX_HALF:  100Mbps half duplex.
*                        TPM_SPEED_10_DUPLEX_FULL:   10Mbps Full duplex.
*                        TPM_SPEED_10_DUPLEX_HALF:   10Mbps half duplex.

*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case, see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
tpm_error_code_t tpm_phy_get_port_autoneg_mode
(
    uint32_t             owner_id,
    tpm_src_port_type_t  src_port,
    bool                *autoneg_state,
    tpm_autoneg_mode_t  *autoneg_mode
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL          state;
    GT_PHY_AUTO_MODE mode;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;
    uint16_t         lsp_mode;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d]\n\r",
               __FUNCTION__,owner_id,src_port);
    }

    /* check PHY access way */
    retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
    if (retVal != TPM_RC_OK) {
        printk(KERN_ERR
               "Port%d PHY access way check failed\n", src_port);
        return retVal;
    }
    /* PHY accessed directly, call lsp API */
    if (PHY_SMI_MASTER_CPU == phy_access_way) {
        if (mvEthPhyAutoNegoGet(phy_direct_addr, (int *)&state)) {
            printk(KERN_ERR
                   "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
            retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
        }
        if (state && (retVal == TPM_RC_OK)) {
            if (mvEthPhyAdvertiseGet(phy_direct_addr, &lsp_mode)) {
                printk(KERN_ERR
                       "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
                retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
            }
            switch (lsp_mode) {
            case (PHY_AUTO_NEGO_MODE_HALF_10 |
                  PHY_AUTO_NEGO_MODE_FULL_10 |
                  PHY_AUTO_NEGO_MODE_HALF_100 |
                  PHY_AUTO_NEGO_MODE_FULL_100 |
                  PHY_AUTO_NEGO_MODE_HALF_1000 |
                  PHY_AUTO_NEGO_MODE_FULL_1000):
                mode = TPM_SPEED_AUTO_DUPLEX_AUTO;
                break;
            case (PHY_AUTO_NEGO_MODE_HALF_1000 |
                  PHY_AUTO_NEGO_MODE_FULL_1000):
                mode = TPM_SPEED_1000_DUPLEX_AUTO;
                break;
            case (PHY_AUTO_NEGO_MODE_FULL_100 |
                  PHY_AUTO_NEGO_MODE_HALF_100):
                mode = TPM_SPEED_100_DUPLEX_AUTO;
                break;
            case (PHY_AUTO_NEGO_MODE_FULL_10 |
                  PHY_AUTO_NEGO_MODE_HALF_10):
                mode = TPM_SPEED_10_DUPLEX_AUTO;
                break;
            case (PHY_AUTO_NEGO_MODE_FULL_10 |
                  PHY_AUTO_NEGO_MODE_FULL_100 |
                  PHY_AUTO_NEGO_MODE_FULL_1000):
                mode = TPM_SPEED_AUTO_DUPLEX_FULL;
                break;
            case (PHY_AUTO_NEGO_MODE_HALF_1000 |
                  PHY_AUTO_NEGO_MODE_HALF_100 |
                  PHY_AUTO_NEGO_MODE_HALF_10):
                mode = TPM_SPEED_AUTO_DUPLEX_HALF;
                break;
            case PHY_AUTO_NEGO_MODE_FULL_1000:
                mode = TPM_SPEED_1000_DUPLEX_FULL;
                break;
            case PHY_AUTO_NEGO_MODE_HALF_1000:
                mode = TPM_SPEED_1000_DUPLEX_HALF;
                break;
            case PHY_AUTO_NEGO_MODE_FULL_100:
                mode = TPM_SPEED_100_DUPLEX_FULL;
                break;
            case PHY_AUTO_NEGO_MODE_HALF_100:
                mode = TPM_SPEED_100_DUPLEX_HALF;
                break;
            case PHY_AUTO_NEGO_MODE_FULL_10:
                mode = TPM_SPEED_10_DUPLEX_FULL;
                break;
            case PHY_AUTO_NEGO_MODE_HALF_10:
                mode = TPM_SPEED_10_DUPLEX_HALF;
                break;
            default:
                mode = TPM_SPEED_AUTO_DUPLEX_AUTO;
                break;
            }
        } else {
            mode = TPM_SPEED_AUTO_DUPLEX_AUTO;
        }
    } else if (PHY_SMI_MASTER_SWT == phy_access_way){
        /* PHY accessed through switch, as original do */
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_get_port_autoneg_mode(lPort, &state, &mode);
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        }
    }
    else if (PHY_SGMII == phy_access_way)
    {  /* nothing to do*/ }

    if(GT_TRUE == state)
        *autoneg_state = true;
    else
        *autoneg_state = false;

    *autoneg_mode = (tpm_autoneg_mode_t)mode;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==EXIT== %s: autoneg_state[%d], autoneg_mode[%d]\n\r", __FUNCTION__, *autoneg_state, *autoneg_mode);
    }

    return retVal;
}

/*******************************************************************************
* tpm_phy_restart_port_autoneg
*
* DESCRIPTION:
*       The API restart the auto negotiation of an Ethernet  UNI port.
*	If AutoNegotiation is not enabled, it'll enable it.
*	Loopback and Power Down will be disabled by this routine.
* INPUTS:
*       owner_id    - APP owner id  should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
*
* OUTPUTS:
*        NONE.
*                                .
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
tpm_error_code_t tpm_phy_restart_port_autoneg
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;
    

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d]\n\r",
               __FUNCTION__,owner_id,src_port);
    }

    /* check PHY access way */
    retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
    if (retVal != TPM_RC_OK) {
        printk(KERN_ERR
               "Port%d PHY access way check failed\n", src_port);
        return retVal;
    }
    /* PHY accessed directly, call lsp API */
    if (PHY_SMI_MASTER_CPU == phy_access_way)
    {
        if (mvEthPhyRestartAN(phy_direct_addr, 0))
        {
            printk(KERN_ERR
                   "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
            retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
        }
    }
    else if (PHY_SMI_MASTER_SWT == phy_access_way)
    {
        /* PHY accessed through switch, as original do */
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_restart_port_autoneg(lPort);
        if (retVal != TPM_RC_OK)
        {
             printk(KERN_ERR
                    "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        }
    }
    else if (PHY_SGMII == phy_access_way)
    {  /* nothing to do*/ }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_phy_set_port_admin_state
*
* DESCRIPTION:
*       The API Configures the PHY port  state of an Ethernet  UNI port.
* INPUTS:
*       owner_id        - APP owner id  should be used for all API calls.
*       src_port        - Source port in UNI port index, UNI0, UNI1...UNI4.
*       phy_port_state  - PHY port  state to set.
*                         0:normal state
*                         1:power down
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success      -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
tpm_error_code_t tpm_phy_set_port_admin_state
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    bool                phy_port_state
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL  state, prev_state;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d],phy_port_state[%d]\n\r",
               __FUNCTION__,owner_id,src_port,phy_port_state);
    }

    if(phy_port_state == true)
        state = GT_TRUE;
    else
        state = GT_FALSE;

    /* check PHY access way */
    retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
    if (retVal != TPM_RC_OK) {
        printk(KERN_ERR
               "Port%d PHY access way check failed\n", src_port);
        return retVal;
    }
    /* PHY accessed directly, call lsp API */
    if (PHY_SMI_MASTER_CPU == phy_access_way)
    {
        if (mvEthPhySetAdminState(phy_direct_addr, (int)state)) {
            printk(KERN_ERR
                   "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
            retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
        }
    }
    else if (PHY_SMI_MASTER_SWT == phy_access_way)
    {
        /* PHY accessed through switch, as original do */
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        if (TPM_RC_OK == (retVal = mv_switch_get_phy_port_state(lPort, &prev_state)))
        {
            if (prev_state != state)
            {
                retVal = mv_switch_set_phy_port_state(lPort, state);
                if (retVal != TPM_RC_OK)
                {
                    printk(KERN_ERR
                           "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
                }
            }
        }
    }
    else if (PHY_SGMII == phy_access_way)
    {  /* nothing to do*/ }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_phy_get_port_admin_state
*
* DESCRIPTION:
*       The API return the PHY port  state of an Ethernet  UNI port.
* INPUTS:
*       owner_id         - APP owner id  should be used for all API calls.
*       src_port         - Source port in UNI port index, UNI0, UNI1...UNI4.
*
*
* OUTPUTS:
*       phy_port_state   -  0:normal state
*                           1:power down                                    .
*
* RETURNS:
*       On success       -  TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
tpm_error_code_t tpm_phy_get_port_admin_state
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    bool               *phy_port_state
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL          state;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d]\n\r",
               __FUNCTION__,owner_id,src_port);
    }
    /* check PHY access way */
    retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
    if (retVal != TPM_RC_OK) {
        printk(KERN_ERR
               "Port%d PHY access way check failed\n", src_port);
        return retVal;
    }
    /* PHY accessed directly, call lsp API */
    if (PHY_SMI_MASTER_CPU == phy_access_way)
    {
        if (mvEthPhyGetAdminState(phy_direct_addr, (int *)&state)) {
            printk(KERN_ERR
                   "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
            retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
        }
    }
    else if (PHY_SMI_MASTER_SWT == phy_access_way)
    {
        /* PHY accessed through switch, as original do */
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_get_phy_port_state(lPort, &state);
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        }
    }
    else if (PHY_SGMII == phy_access_way)
    {  /* nothing to do*/ }

    if(state == GT_TRUE)
        *phy_port_state = true;
    else
        *phy_port_state = false;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s: phy_port_state[%d]\n\r",__FUNCTION__,*phy_port_state);
    }

    return retVal;
}

/*******************************************************************************
* tpm_phy_get_port_link_status
*
* DESCRIPTION:
*       The API return realtime port link status of an Ethernet  UNI port.
* INPUTS:
*       owner_id    - APP owner id  should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
*
* OUTPUTS:
*       port_link_status  -  0:port link is ON
*                            1:port link is DOWN                                    .
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
tpm_error_code_t tpm_phy_get_port_link_status
(
    uint32_t             owner_id,
    tpm_src_port_type_t  src_port,
    bool                *port_link_status
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL          state;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d]\n\r",
               __FUNCTION__,owner_id,src_port);
    }
    /* check PHY access way */
    retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
    if (retVal != TPM_RC_OK) {
        printk(KERN_ERR
               "Port%d PHY access way check failed\n", src_port);
        return retVal;
    }
    /* PHY accessed directly, call lsp API */
    if (PHY_SMI_MASTER_CPU == phy_access_way)
    {
        state = (GT_BOOL)mvEthPhyCheckLink(phy_direct_addr);
    }
    else if (PHY_SMI_MASTER_SWT == phy_access_way)
    {
        /* PHY accessed through switch, as original do */
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_get_port_link_status(lPort, &state);
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        }
    }
    else if (PHY_SGMII == phy_access_way)
    {  /* nothing to do*/ }

    if(state == GT_TRUE)
        *port_link_status = true;
    else
        *port_link_status = false;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==EXIT== %s: port_link_status[%d]\n\r",__FUNCTION__,*port_link_status);
    }

    return retVal;
}

/*******************************************************************************
* tpm_phy_get_port_duplex_status
*
* DESCRIPTION:
*       The API return realtime port duplex status of an Ethernet  UNI port.
* INPUTS:
*       owner_id    - APP owner id  should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
*
* OUTPUTS:
*       port_duplex_status  -  0:half deplex mode
*                              1:full deplex mode                    .
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case, see tpm_error_code_t.
*
* COMMENTS:
* This function is different from tpm_phy_get_port_duplex_mode, it reflects the real duplex status happening
* in PHY now. If there is a switch, through PPU function, switch can get the duplex info from PHY and
* store it in switch port register, so duplex info can be got from switch port register. It is a
* "get_xxx_oper" function. If there is no switch, this info can be read from PHY spec status register,
* offset 17
*
*******************************************************************************/
tpm_error_code_t tpm_phy_get_port_duplex_status
(
    uint32_t             owner_id,
    tpm_src_port_type_t  src_port,
    bool                *port_duplex_status
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL          state, status_valid;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d]\n\r",
               __FUNCTION__,owner_id,src_port);
    }
    /* check PHY access way */
    retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
    if (retVal != TPM_RC_OK) {
        printk(KERN_ERR
               "Port%d PHY access way check failed\n", src_port);
        return retVal;
    }
    /* PHY accessed directly, call lsp API */
    if (PHY_SMI_MASTER_CPU == phy_access_way)
    {
        if (mvEthPhyDuplexOperGet(phy_direct_addr, (int *)&status_valid, (int *)&state)) {
            printk(KERN_ERR
                   "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
            retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
        }

        if (status_valid == GT_FALSE) {
            printk(KERN_ERR
                   "ERROR: (%s:%d) PHY duplex status is not valid on port(%d)\n", __FUNCTION__, __LINE__, src_port);
            retVal = ERR_PHY_STATUS_UNKNOWN;
        }
    }
    else if (PHY_SMI_MASTER_SWT == phy_access_way)
    {
        /* PHY accessed through switch, as original do */
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_get_port_duplex_status(lPort, &state);
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        }
    }
    else if (PHY_SGMII == phy_access_way)
    {  /* nothing to do*/ }

    if(state == GT_TRUE)
        *port_duplex_status = true;
    else
        *port_duplex_status = false;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s: port_duplex_status[%d]\n\r",__FUNCTION__,*port_duplex_status);
    }

    return retVal;
}

/*******************************************************************************
* tpm_phy_get_port_speed_mode
*
* DESCRIPTION:
*       The API return realtime port speed mode of an Ethernet  UNI port.
* INPUTS:
*       owner_id    - APP owner id  should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
*
* OUTPUTS:
*       port_duplex_status  -  0:10M
*                              1:100M
*                              2:1000M
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case, see tpm_error_code_t.
*
* COMMENTS:
* This function is different from "tpm_phy_get_port_speed ", it reflects what really happens
* in PHY. If there is a switch, this function will get the stats info from switch register,
* because the PPU function will make the switch get the PHY status info and store it in port
* register. If there is no switch, speed mode info can be got from PHY register spec status
* register, offset is 17.
*******************************************************************************/
tpm_error_code_t tpm_phy_get_port_speed_mode
(
    uint32_t             owner_id,
    tpm_src_port_type_t  src_port,
    uint32_t            *speed
)
{
    tpm_error_code_t   retVal = TPM_RC_OK;
    int32_t            lPort  = 0;
    GT_PORT_SPEED_MODE tmpSpeed;
    tpm_phy_ctrl_t     phy_access_way;
    uint32_t           phy_direct_addr;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d]\n\r",
               __FUNCTION__,owner_id,src_port);
    }
    /* check PHY access way */
    retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
    if (retVal != TPM_RC_OK) {
        printk(KERN_ERR
               "Port%d PHY access way check failed\n", src_port);
        return retVal;
    }
    /* PHY accessed directly, call lsp API */
    if (PHY_SMI_MASTER_CPU == phy_access_way)
    {
        if (mvEthPhySpeedOperGet(phy_direct_addr, (uint32_t *)&tmpSpeed)) {
            printk(KERN_ERR
                   "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
            retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
        }

        if (tmpSpeed == PORT_SPEED_UNKNOWN) {
            printk(KERN_ERR
                   "ERROR: (%s:%d) PHY speed status is not valid on port(%d)\n", __FUNCTION__, __LINE__, src_port);
            retVal = ERR_PHY_STATUS_UNKNOWN;
        }
    }
    else if (PHY_SMI_MASTER_SWT == phy_access_way)
    {
        /* PHY accessed through switch, as original do */
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_get_port_speed_mode(lPort, &tmpSpeed);
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        }
    }
    else if (PHY_SGMII == phy_access_way)
    {  /* nothing to do*/ }

    *speed = (uint32_t)tmpSpeed;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s: speed[%d]\n\r",__FUNCTION__,*speed);
    }

    return retVal;
}

/*******************************************************************************
* tpm_phy_set_port_flow_control_support
*
* DESCRIPTION:
*       This routine will set the pause bit in Autonegotiation Advertisement
*        Register. And restart the autonegotiation.
*
* INPUTS:
*       owner_id   - APP owner id  should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*       state      - false:port pause is off.
*                    true:port pause is on.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
* COMMENTS:
* This function reflect flow control configuration info, it only access PHY register,
* that is data sheet register 4.10 Autonegotiation Advertisement Register.
*******************************************************************************/

tpm_error_code_t tpm_phy_set_port_flow_control_support
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    bool                state
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_PHY_PAUSE_MODE pause_state, prev_state;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;
    uint32_t         switch_init;
    tpm_gmacs_enum_t gmac_port;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d],state[%d]\n\r",
               __FUNCTION__,owner_id,src_port,state);
    }

    if(state == true)
        pause_state = GT_PHY_PAUSE;
    else
        pause_state = GT_PHY_NO_PAUSE;

    /* check PHY access way */
    retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
    if (retVal != TPM_RC_OK) {
        printk(KERN_ERR
               "Port%d PHY access way check failed\n", src_port);
        return retVal;
    }
    /* PHY accessed directly, call lsp API */
    if (PHY_SMI_MASTER_CPU == phy_access_way)
    {
        if (tpm_db_switch_init_get(&switch_init) != TPM_DB_OK) {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_db_switch_init_get Failed\n", __FUNCTION__, __LINE__);
            retVal = ERR_GENERAL;
            return retVal;
        }
        /* Only PHY connected to GMAC, need write GMAC register */
        if (!switch_init) {
            for (gmac_port = TPM_ENUM_GMAC_0; gmac_port < TPM_MAX_NUM_GMACS; gmac_port++) {
                if (phy_direct_addr == mvBoardPhyAddrGet(gmac_port))
                break;
            }
            if (gmac_port == TPM_MAX_NUM_GMACS) {
                printk(KERN_ERR "ERROR: (%s:%d) Can not find gmac port\n", __FUNCTION__, __LINE__);
                retVal = ERR_GENERAL;
                return retVal;
            }

            /* Write Flow Control reg on GMAC port */
            switch(pause_state) {
            case GT_PHY_NO_PAUSE:
                if (mvNetaFlowCtrlSet((int)gmac_port, MV_ETH_FC_DISABLE) != MV_OK) {
                    printk(KERN_ERR "ERROR: (%s:%d) mvNetaFlowCtrlSet Failed\n", __FUNCTION__, __LINE__);
                    retVal = ERR_GENERAL;
                    return retVal;
                }
                break;
            case GT_PHY_PAUSE:
                if (mvNetaFlowCtrlSet((int)gmac_port, MV_ETH_FC_ENABLE) != MV_OK) {
                    printk(KERN_ERR "ERROR: (%s:%d) mvNetaFlowCtrlSet Failed\n", __FUNCTION__, __LINE__);
                    retVal = ERR_GENERAL;
                    return retVal;
                }
                break;
            default:
                printk(KERN_ERR "ERROR: (%s:%d)Invalid flow control mode%d\n", __FUNCTION__, __LINE__, pause_state);
                retVal = ERR_GENERAL;
                return retVal;
            }
        } else {
            /* Flow control info can not be got from linkpartner through QSGMII auto negitiation, so manual needed */
            lPort = tpm_db_eth_port_switch_port_get(src_port);
            if (lPort == TPM_DB_ERR_PORT_NUM)
            {
                printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
                return ERR_SRC_PORT_INVALID;
            }
            /* Set force FC value */
            if ((retVal = mv_switch_set_port_forced_fc_value(lPort, state)) != TPM_RC_OK) {
                printk(KERN_ERR "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
                return retVal;
            }
            /* Set force FC */
            if ((retVal = mv_switch_set_port_forced_flow_control(lPort, state)) != TPM_RC_OK) {
                printk(KERN_ERR "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
                return retVal;
            }
        }
        if (mvEthPhyPauseSet(phy_direct_addr, (uint32_t)pause_state)) {
            printk(KERN_ERR
                   "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
            retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
        }
    }
    else if (PHY_SMI_MASTER_SWT == phy_access_way)
    {
        /* PHY accessed through switch, as original do */
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        if (TPM_RC_OK == (retVal = mv_switch_get_port_pause(lPort, &prev_state)))
        {
            if (prev_state != pause_state)
            {
                retVal = mv_switch_set_port_pause(lPort, pause_state);
                if (retVal != TPM_RC_OK)
                {
                    printk(KERN_ERR
                           "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
                }
            }
        }
    }
    else if (PHY_SGMII == phy_access_way)
    {  /* nothing to do*/ }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_phy_get_port_flow_control_support
* DESCRIPTION:
*       This routine will get the pause bit in Autonegotiation Advertisement
*        Register.
*
* INPUTS:
*       owner_id   - APP owner id  should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS
*       state      -  false:port pause is off.
*                      true:port pause is on.
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
* COMMENTS:
* data sheet register 4.10 Autonegotiation Advertisement Register
*******************************************************************************/

tpm_error_code_t tpm_phy_get_port_flow_control_support
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    bool               *state
)
{
    tpm_error_code_t  retVal = TPM_RC_OK;
    int32_t           lPort  = 0;
    GT_PHY_PAUSE_MODE pause_state;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d]\n\r",
               __FUNCTION__,owner_id,src_port);
    }
    /* check PHY access way */
    retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
    if (retVal != TPM_RC_OK) {
        printk(KERN_ERR
               "Port%d PHY access way check failed\n", src_port);
        return retVal;
    }
    /* PHY accessed directly, call lsp API */
    if (PHY_SMI_MASTER_CPU == phy_access_way)
    {
        if (mvEthPhyPauseAdminGet(phy_direct_addr, (uint32_t *)&pause_state)) {
            printk(KERN_ERR
                   "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
            retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
        }
    }
    else if (PHY_SMI_MASTER_SWT == phy_access_way)
    {
        /* PHY accessed through switch, as original do */
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_get_port_pause(lPort, &pause_state);
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        }
    }
    else if (PHY_SGMII == phy_access_way)
    {  /* nothing to do*/ }

    if(pause_state == GT_PHY_NO_PAUSE)
        *state = false;
    else
        *state = true;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s: flow_control[%d]\n\r",__FUNCTION__,*state);
    }

    return retVal;
}

/*******************************************************************************
* tpm_phy_get_port_flow_control_state
*
* DESCRIPTION:
*     This routine will get the current pause state.
*        Register.
*
* INPUTS:
*       owner_id   - APP owner id  should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*
*
* OUTPUTS:
*        state     -
*                   false: MAC Pause not implemented in the link partner or in MyPause
*                   true:  MAC Pause is implemented in the link partner and in MyPause
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case, see tpm_error_code_t.
* COMMENTS:
* This function is different from function tpm_phy_set_port_flow_control_support(),
* it reflects the flow control info on MAC of PHY, so this info can only be got from
* MAC, PHY connected to. If there is a switch, this info can be got from switch port register,
* if there is no switch connected to PHY, this info can be got from the MAC connected to it,
* such as GMAC0.
*******************************************************************************/
tpm_error_code_t tpm_phy_get_port_flow_control_state
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    bool               *state
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL          fc_state;
    uint32_t         switch_init;
    tpm_gmacs_enum_t gmac_port;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;
    uint32_t         fc_mode;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d]\n\r",
               __FUNCTION__,owner_id,src_port);
    }

    /* Check swicth init or not */
    if (tpm_db_switch_init_get(&switch_init) != TPM_DB_OK) {
        printk(KERN_ERR "ERROR: (%s:%d) tpm_db_switch_init_get failed\n", __FUNCTION__, __LINE__);
        return ERR_GENERAL;
    }
    if (switch_init) {
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_get_port_pause_state(lPort, &fc_state);
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        }

        if(fc_state == GT_FALSE)
            *state = false;
        else
            *state = true;
    } else {
        /* check PHY access way */
        retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
        if (retVal != TPM_RC_OK) {
            printk(KERN_ERR
                   "Port%d PHY access way check failed\n", src_port);
            return retVal;
        }
        if(PHY_SMI_MASTER_CPU == phy_access_way)
        {
			for (gmac_port = TPM_ENUM_GMAC_0; gmac_port < TPM_MAX_NUM_GMACS; gmac_port++) {
				if (phy_direct_addr == mvBoardPhyAddrGet(gmac_port))
					break;
			}
			if (gmac_port == TPM_MAX_NUM_GMACS) {
				printk(KERN_ERR "ERROR: (%s:%d) Can not find gmac port\n", __FUNCTION__, __LINE__);
				retVal = ERR_GENERAL;
				return retVal;
			}

			/* Get GMAC Flow Control state */
			if (mvNetaFlowCtrlGet((int)gmac_port, &fc_mode) != MV_OK) {
				printk(KERN_ERR "ERROR: (%s:%d) Can not find gmac port\n", __FUNCTION__, __LINE__);
				retVal = ERR_GENERAL;
				return retVal;
			}

			if (fc_mode == MV_ETH_FC_DISABLE)
				*state = false;
			else
				*state = true;
        }
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s: flow_control_state[%d]\n\r",__FUNCTION__,*state);
    }

    return retVal;
}

/*******************************************************************************
* tpm_phy_set_port_loopback
*
*
* INPUTS:
*       owner_id    - APP owner id  should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*       mode        - Internal or external loopback
*       enable      - If GT_TRUE, enable loopback mode
*                     If GT_FALSE, disable loopback mode
*
* OUTPUTS:
* None.
*
* RETURNS:
*       On success  - TPM_RC_OK.
*       On error different types are returned according to the case , see tpm_error_code_t.
*
* COMMENTS:
* data sheet register 0.14 - Loop_back
*
*******************************************************************************/
tpm_error_code_t tpm_phy_set_port_loopback
(
    uint32_t                owner_id,
    tpm_src_port_type_t     src_port,
    tpm_phy_loopback_mode_t mode,
    bool                    enable
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL          state, link_forced;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;
    uint32_t         switch_init;
    uint32_t         gmac_idx;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d],mode[%d], enable[%d]\n\r",
               __FUNCTION__,owner_id, src_port, mode, (uint32_t)enable);
    }

    if(enable == true)
    {
        state = GT_TRUE;
    }
    else
    {
        state = GT_FALSE;
    }
    /* check PHY access way */
    retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
    if (retVal != TPM_RC_OK) {
        printk(KERN_ERR
               "Port%d PHY access way check failed\n", src_port);
        return retVal;
    }
    /* PHY accessed directly, call lsp API */
    if (PHY_SMI_MASTER_CPU == phy_access_way) {
        if (PHY_INTERNAL_LOOPBACK == (GT_PHY_LOOPBACK_MODE)mode) {
            if (mvEthPhyLoopbackSet(phy_direct_addr, state)) {
                printk(KERN_ERR
                       "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
                retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
            }
            /* Force switch port link up if QSGMII mode with switch */
            retVal = tpm_db_switch_init_get(&switch_init);
            if (retVal) {
                printk(KERN_ERR "ERROR: (%s:%d)switch init get failed\n", __FUNCTION__, __LINE__);
                return retVal;
            }
            if (switch_init) {
                lPort = tpm_db_eth_port_switch_port_get(src_port);
                if (lPort == TPM_DB_ERR_PORT_NUM)
                {
                    printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
                    return ERR_SRC_PORT_INVALID;
                }
                retVal = mv_switch_get_port_forced_link(lPort, &link_forced);
                if (retVal != TPM_RC_OK)
                {
                    printk(KERN_ERR
                           "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
                    return retVal;
                }
                if (((state == GT_TRUE) && (link_forced == GT_FALSE)) ||
                    ((state == GT_FALSE)&& (link_forced == GT_TRUE)))
                {
                    retVal = mv_switch_set_port_forced_link(lPort, state);
                    if (retVal != TPM_RC_OK)
                    {
                        printk(KERN_ERR
                               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
                        return retVal;
                    }
                    retVal = mv_switch_set_port_link_value(lPort, state);
                    if (retVal != TPM_RC_OK)
                    {
                        printk(KERN_ERR
                               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
                        return retVal;
                    }
                }
            } else {
                /* get GMAC with Phyaddr */
                for (gmac_idx = TPM_ENUM_GMAC_0; gmac_idx < TPM_MAX_NUM_GMACS; gmac_idx++) {
                    if (phy_direct_addr == mvBoardPhyAddrGet(gmac_idx))
                        break;
                }
                if (gmac_idx == TPM_MAX_NUM_GMACS) {
                    printk(KERN_ERR
                           "%s:%d: GMAC get with phyaddr invalid\r\n", __FUNCTION__,__LINE__);
                    retVal = ERR_GENERAL;
                    return retVal;
                }
                /* Force gmac link or disable it */
                if (mvNetaForceLinkModeSet(gmac_idx, state, 0)) {
                    printk(KERN_ERR
                           "%s:%d: GMAC force link set failed\r\n", __FUNCTION__,__LINE__);
                    retVal = ERR_GENERAL;
                    return retVal;
                }
            }
        } else if (PHY_EXTERNAL_LOOPBACK == (GT_PHY_LOOPBACK_MODE)mode) {
            if (mvEthPhyLineLoopbackSet(phy_direct_addr, state)) {
                printk(KERN_ERR
                       "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
                retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
            }
        } else {
            printk(KERN_INFO
                   "==ENTER==%s: mode[%d] invalid\n\r", __FUNCTION__,mode);
            retVal = ERR_GENERAL;
        }
    }
    else if (PHY_SMI_MASTER_SWT == phy_access_way)
    {
        /* PHY accessed through switch, as original do */
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        switch((GT_PHY_LOOPBACK_MODE)mode)
        {
            case PHY_INTERNAL_LOOPBACK:
                if (lPort == INT_GE_PHY_SWITCH_PORT)
                {
                    retVal = mv_switch_get_port_forced_link(lPort, &link_forced);
                    if (retVal != TPM_RC_OK)
                    {
                        printk(KERN_ERR
                               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
                        break;
                    }
                    if (((state == GT_TRUE) && (link_forced == GT_FALSE)) ||
                        ((state == GT_FALSE)&& (link_forced == GT_TRUE)))
                    {
                        retVal = mv_switch_set_port_forced_link(lPort, state);
                        if (retVal != TPM_RC_OK)
                        {
                            printk(KERN_ERR
                                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
                            break;
                        }
                        retVal = mv_switch_set_port_link_value(lPort, state);
                        if (retVal != TPM_RC_OK)
                        {
                            printk(KERN_ERR
                                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
                            break;
                        }
                    }
                }
                retVal = mv_switch_set_port_loopback(lPort, state);
                break;
            case PHY_EXTERNAL_LOOPBACK:
                retVal = mv_switch_set_port_line_loopback(lPort, state);
                break;
            default:
                break;

        }
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        }
    }
    else if (PHY_SGMII == phy_access_way)
    {  /* nothing to do*/ }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_phy_get_port_loopback
*
* INPUTS:
*       owner_id    - APP owner id  should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*       mode        - Internal or external loopback
*
* OUTPUTS:
*       enable      - If GT_TRUE,  loopback mode is enabled
*                     If GT_FALSE,  loopback mode is disabled
*
* RETURNS:
*       On success  - TPM_RC_OK.
*       On error different types are returned according to the case , see tpm_error_code_t.
*
* COMMENTS:
* data sheet register 0.14 - Loop_back
*
*******************************************************************************/
tpm_error_code_t tpm_phy_get_port_loopback
(
    uint32_t                owner_id,
    tpm_src_port_type_t     src_port,
    tpm_phy_loopback_mode_t mode,
    bool                   *enable
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL          state;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d], mode[%d]\n\r",
               __FUNCTION__,owner_id,src_port, mode);
    }
    /* check PHY access way */
    retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
    if (retVal != TPM_RC_OK) {
        printk(KERN_ERR
               "Port%d PHY access way check failed\n", src_port);
        return retVal;
    }
    /* PHY accessed directly, call lsp API */
    if (PHY_SMI_MASTER_CPU == phy_access_way) {
        if (PHY_INTERNAL_LOOPBACK == (GT_PHY_LOOPBACK_MODE)mode) {
            if (mvEthPhyLoopbackGet(phy_direct_addr, (int *)&state)) {
                printk(KERN_ERR
                       "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
                retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
            }
        } else if (PHY_EXTERNAL_LOOPBACK == (GT_PHY_LOOPBACK_MODE)mode) {
            if (mvEthPhyLineLoopbackGet(phy_direct_addr, (int *)&state)) {
                printk(KERN_ERR
                       "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
                retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
            }
        } else {
            printk(KERN_INFO
                   "==ENTER==%s: mode[%d] invalid\n\r", __FUNCTION__,mode);
            retVal = ERR_GENERAL;
        }
    }
    else if (PHY_SMI_MASTER_SWT == phy_access_way)
    {
        /* PHY accessed through switch, as original do */
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        switch((GT_PHY_LOOPBACK_MODE)mode)
        {
            case PHY_INTERNAL_LOOPBACK:
                retVal = mv_switch_get_port_loopback(lPort, &state);
                break;
            case PHY_EXTERNAL_LOOPBACK:
                retVal = mv_switch_get_port_line_loopback(lPort, &state);
                break;
            default:
                break;
        }
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        }
    }
    else if (PHY_SGMII == phy_access_way)
    {  /* nothing to do*/ }

    if(state == GT_TRUE)
    {
        *enable = true;
    }
    else
    {
        *enable = false;
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s: enable[%d]\n\r",__FUNCTION__,(uint32_t)*enable);
    }

    return retVal;
}

/*******************************************************************************
* tpm_phy_set_port_duplex_mode
*
* DESCRIPTION:
*        Sets duplex mode for a specific logical port. This function will keep
*        the speed and loopback mode to the previous value, but disable others,
*        such as Autonegotiation.
*
* INPUTS:
*       owner_id    - APP owner id  should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*       enable      - Enable/Disable dulpex mode
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       On success  - TPM_RC_OK.
*       On error different types are returned according to the case , see tpm_error_code_t.
*
* COMMENTS:
*         data sheet register 0.8 - Duplex Mode
*
*******************************************************************************/
tpm_error_code_t tpm_phy_set_port_duplex_mode
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    bool                enable
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL          state;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d],enable[%d]\n\r",
               __FUNCTION__,owner_id,src_port,(uint32_t)enable);
    }

    if(enable == true)
    {
        state = GT_TRUE;
    }
    else
    {
        state = GT_FALSE;
    }

    /* check PHY access way */
    retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
    if (retVal != TPM_RC_OK) {
        printk(KERN_ERR
               "Port%d PHY access way check failed\n", src_port);
        return retVal;
    }
    /* PHY accessed directly, call lsp API */
    if (PHY_SMI_MASTER_CPU == phy_access_way) {
        if (mvEthPhyDuplexModeSet(phy_direct_addr, state)) {
            printk(KERN_ERR
                   "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
            retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
        } else {
            /* Update GMAC speed, duplex and flow control */
            retVal = tpm_sw_set_gmac_speed_duplex_fc(owner_id, src_port, false);
            if (retVal != TPM_RC_OK) {
                printk(KERN_ERR "Failed to update GMAC config with port%d\n", src_port);
                return retVal;
            }
        }
    }
    else if (PHY_SMI_MASTER_SWT == phy_access_way)
    {
        /* PHY accessed through switch, as original do */
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_set_port_duplex_mode(lPort, state);
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        }
    }
    else if (PHY_SGMII == phy_access_way)
    {  /* nothing to do*/ }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_phy_get_port_duplex_mode
*
* DESCRIPTION:
*         Gets duplex mode for a specific logical port.
*
* INPUTS:
*       owner_id    - APP owner id  should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.

*
* OUTPUTS:
*       enable      - Enable/Disable dulpex mode
*
* RETURNS:
*       On success  - TPM_RC_OK.
*       On error different types are returned according to the case , see tpm_error_code_t.
*
* COMMENTS:
*         This function will get the duplex configuration info just from PHY's control register
* that is data sheet register 0.8 - Duplex Mode. It is a "get_xxx_admin" function.
*
*******************************************************************************/
tpm_error_code_t tpm_phy_get_port_duplex_mode
(
    uint32_t             owner_id,
    tpm_src_port_type_t  src_port,
    bool                *enable
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_BOOL          state;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d]\n\r",
               __FUNCTION__,owner_id,src_port);
    }

    /* check PHY access way */
    retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
    if (retVal != TPM_RC_OK) {
        printk(KERN_ERR
               "Port%d PHY access way check failed\n", src_port);
        return retVal;
    }
    /* PHY accessed directly, call lsp API */
    if (PHY_SMI_MASTER_CPU == phy_access_way) {
        if (mvEthPhyDuplexModeAdminGet(phy_direct_addr, (int *)&state)) {
            printk(KERN_ERR
                   "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
            retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
        }
    }
    else if (PHY_SMI_MASTER_SWT == phy_access_way)
    {
        /* PHY accessed through switch, as original do */
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_get_port_duplex_mode(lPort, &state);
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        }

        if (trace_sw_dbg_flag)
        {
            printk(KERN_INFO
                    "==EXIT== %s:enable[%d]\n\r",__FUNCTION__,  *enable);
        }
    }
    else if (PHY_SGMII == phy_access_way)
    {  /* nothing to do*/ }

    if(state == GT_TRUE)
    {
        *enable = true;
    }
    else
    {
        *enable = false;
    }

    return retVal;
}

/*******************************************************************************
* tpm_phy_set_port_speed
*
* DESCRIPTION:
*       This routine will disable auto-negotiation and set the PHY port speed .
*
* INPUTS:
*       owner_id    - APP owner id  should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*       speed       -    PHY_SPEED_10_MBPS   - 10Mbps
*                        PHY_SPEED_100_MBPS  - 100Mbps
*                        PHY_SPEED_1000_MBPS - 1000Mbps.
*
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

tpm_error_code_t tpm_phy_set_port_speed
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    tpm_phy_speed_t     speed
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d],speed[%d]\n\r",
               __FUNCTION__,owner_id,src_port, (uint32_t)speed);
    }

    /* check PHY access way */
    retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
    if (retVal != TPM_RC_OK) {
        printk(KERN_ERR
               "Port%d PHY access way check failed\n", src_port);
        return retVal;
    }
    /* PHY accessed directly, call lsp API */
    if (PHY_SMI_MASTER_CPU == phy_access_way) {
        if (mvEthPhySpeedSet(phy_direct_addr, speed)) {
            printk(KERN_ERR
                   "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
            retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
        } else {
            /* Update GMAC speed, duplex and flow control */
            retVal = tpm_sw_set_gmac_speed_duplex_fc(owner_id, src_port, false);
            if (retVal != TPM_RC_OK) {
                printk(KERN_ERR "Failed to update GMAC config with port%d\n", src_port);
                return retVal;
            }
        }
    }
    else if (PHY_SMI_MASTER_SWT == phy_access_way)
    {
        /* PHY accessed through switch, as original do */
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_set_port_speed(lPort, (GT_PHY_SPEED)speed);
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        }
    }
    else if (PHY_SGMII == phy_access_way)
    {  /* nothing to do*/ }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_phy_get_port_speed
*
* DESCRIPTION:
*       This routine will get current PHY port speed .
*
* INPUTS:
*       owner_id    - APP owner id  should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.

*
*
* OUTPUTS:
*        speed      -    PHY_SPEED_10_MBPS   -10Mbps
*                        PHY_SPEED_100_MBPS  -100Mbps
*                        PHY_SPEED_1000_MBPS -1000Mbps.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case - see tpm_error_code_t.
* COMMENTS:
* This function get the PHY config information from PHY controlregister, it is a
* "get_xxx_admin" function, it reflect the configuration to PHY and it only access PHY register.
*
*******************************************************************************/
tpm_error_code_t tpm_phy_get_port_speed
(
    uint32_t            owner_id,
    tpm_src_port_type_t src_port,
    tpm_phy_speed_t    *speed
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_PHY_SPEED     lSpeed;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d]\n\r",
               __FUNCTION__,owner_id,src_port);
    }

    /* check PHY access way */
    retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
    if (retVal != TPM_RC_OK) {
        printk(KERN_ERR
               "Port%d PHY access way check failed\n", src_port);
        return retVal;
    }
    /* PHY accessed directly, call lsp API */
    if (PHY_SMI_MASTER_CPU == phy_access_way) {
        if (mvEthPhySpeedAdminGet(phy_direct_addr, speed)) {
            printk(KERN_ERR
                   "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
            retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
        }
    }
    else if (PHY_SMI_MASTER_SWT == phy_access_way)
    {
        /* PHY accessed through switch, as original do */
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_get_port_speed(lPort, &lSpeed);
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        }

        *speed = (tpm_phy_speed_t)lSpeed;
    }
    else if (PHY_SGMII == phy_access_way)
    {  /* nothing to do*/ }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s: speed[%d]\n\r",__FUNCTION__,(uint32_t)*speed);
    }

    return retVal;
}

/*------------------------------------------------------------------------------
                            Debug functions
------------------------------------------------------------------------------*/


/*******************************************************************************
* tpm_sw_prv_clear_port_counters
*
* DESCRIPTION:
*       This function gets all counters of the given port
*
* INPUTS:
*       None.
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
tpm_error_code_t tpm_sw_prv_clear_port_counters
(
    void
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    tpm_gmacs_enum_t gmac_i;
    tpm_db_gmac_func_t gfunc;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s:\n\r",__FUNCTION__);
    }

    if (tpm_sw_init_check()) {
        for (gmac_i = TPM_ENUM_GMAC_0; gmac_i < TPM_MAX_NUM_GMACS; gmac_i++) {
            tpm_db_gmac_func_get(gmac_i, &gfunc);
            if (gfunc == TPM_GMAC_FUNC_LAN_UNI ||
                gfunc == TPM_GMAC_FUNC_US_MAC_LEARN_DS_LAN_UNI) {
                MV_NETA_PORT_CTRL *pPortCtrl = mvNetaPortHndlGet(gmac_i);
                mvNetaMibCountersClear(gmac_i, pPortCtrl->txpNum);
            }
        }
    } else
        retVal = mv_switch_clear_port_counters();
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

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
    uint32_t owner_id,
    uint8_t  regAddr,
    uint16_t data
)
{
    tpm_error_code_t retVal;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d] regAddr[0x%x] data[0x%x]\n\r",
               __FUNCTION__,owner_id, regAddr, data);
    }

    retVal = mv_switch_set_global_reg (regAddr, data);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:n\r",__FUNCTION__);
    }

    return retVal;
}

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
     uint32_t owner_id,
     uint8_t  regAddr,
    uint16_t *data
)
{
    tpm_error_code_t retVal;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d] regAddr[0x%x]\n\r",
               __FUNCTION__,owner_id, regAddr);
    }

    retVal = mv_switch_get_global_reg (regAddr, data);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:,data[0x%x]\n\r",__FUNCTION__,*data);
    }

    return retVal;
}

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
     uint32_t owner_id,
     uint8_t  lport,
     uint8_t  regAddr,
     uint16_t data
)
{
    tpm_error_code_t retVal;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d] lport[%d] regAddr[0x%x] data[0x%x]\n\r",
                __FUNCTION__,lport, owner_id, regAddr, data);
    }

    retVal = mv_switch_set_port_reg (lport, regAddr, data);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

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
     uint32_t  owner_id,
     uint8_t   lport,
     uint8_t   regAddr,
     uint16_t *data
)
{
    tpm_error_code_t retVal;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d] lport[%d] regAddr[0x%x]\n\r",
                __FUNCTION__,owner_id,lport, regAddr);
    }

    retVal = mv_switch_get_port_reg (lport, regAddr, data);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:,data[0x%x]\n\r",__FUNCTION__,*data);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_print_port_counters
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
*        Clear on read.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_print_port_counters
(
     uint32_t owner_id,
     uint8_t  lport
)
{
    tpm_error_code_t retVal;
    tpm_gmacs_enum_t gmac_i;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d] lport[%d]\n\r",
                __FUNCTION__,owner_id,lport);
    }

    if (tpm_sw_init_check()) {
        if (tpm_src_port_mac_map(lport, &gmac_i)) {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) map to mac failed\n", __FUNCTION__, __LINE__, lport);
            return ERR_SRC_PORT_INVALID;
        }
        retVal = tpm_sw_pm_print_from_gmac(gmac_i);
    } else
        retVal = mv_switch_print_port_counters(lport);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_INFO
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        return retVal;
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

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
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    tpm_gmacs_enum_t gmac_i;
    MV_NETA_PORT_CTRL *pPortCtrl;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s:\n\r",__FUNCTION__);
    }

    if (tpm_sw_init_check()) {
        if (tpm_src_port_mac_map(src_port, &gmac_i)) {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) map to mac failed\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }
        pPortCtrl = mvNetaPortHndlGet(gmac_i);
        mvNetaMibCountersClear(gmac_i, pPortCtrl->txpNum);
    } else {

        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_clean_port_counters(lPort);
    }
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_pm_from_gmac
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
tpm_error_code_t tpm_sw_pm_from_gmac(int port,
				     tpm_swport_pm_1_t   *tpm_swport_pm_1,
				     tpm_swport_pm_3_all_t *tpm_swport_pm_3)
{
    uint32_t         regVaLo, regValHi = 0;
    int32_t          mib = 0;

    /* Check Input */
    if (tpm_swport_pm_1 == NULL || tpm_swport_pm_3 == NULL) {
        printk(KERN_ERR "ERROR: (%s:%d) Invalid Input\n", __FUNCTION__, __LINE__);
        return ERR_GENERAL;
    }

    /* Check GMAC Port */
    if (mvNetaTxpCheck(port, mib)) {
        printk(KERN_ERR "ERROR: (%s:%d) mvNetaTxpCheck failed\n", __FUNCTION__, __LINE__);
        return ERR_GENERAL;
    }
    if (!mvNetaPortHndlGet(port)) {
        printk(KERN_ERR "ERROR: (%s:%d) mvNetaPortHndlGet failed\n", __FUNCTION__, __LINE__);
        return ERR_GENERAL;
    }

    /* Initialize tpm_swport_pm_3 as Zero */
    memset(tpm_swport_pm_1, 0, sizeof(tpm_swport_pm_1_t));
    memset(tpm_swport_pm_3, 0, sizeof(tpm_swport_pm_3_all_t));

    /* Read counter */
    tpm_swport_pm_3->dropEvents = MV_REG_READ(ETH_RX_DISCARD_PKTS_CNTR_REG(port));

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_GOOD_OCTETS_RECEIVED_LOW, &regValHi);
    tpm_swport_pm_3->InGoodOctetsLo = regVaLo;
    tpm_swport_pm_3->InGoodOctetsHi = regValHi;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_BAD_OCTETS_RECEIVED, &regValHi);
    tpm_swport_pm_3->InBadOctets = regVaLo;

    tpm_swport_pm_3->Deferred = MV_REG_READ(ETH_RX_OVERRUN_PKTS_CNTR_REG(port));
    tpm_swport_pm_1->deferredTransmissionCounter = tpm_swport_pm_3->Deferred;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_BROADCAST_FRAMES_RECEIVED, &regValHi);
    tpm_swport_pm_3->InBroadcasts = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_MULTICAST_FRAMES_RECEIVED, &regValHi);
    tpm_swport_pm_3->InMulticasts = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_FRAMES_64_OCTETS, &regValHi);
    tpm_swport_pm_3->Octets64 = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_FRAMES_65_TO_127_OCTETS, &regValHi);
    tpm_swport_pm_3->Octets127 = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_FRAMES_128_TO_255_OCTETS, &regValHi);
    tpm_swport_pm_3->Octets255 = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_FRAMES_256_TO_511_OCTETS, &regValHi);
    tpm_swport_pm_3->Octets511 = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_FRAMES_512_TO_1023_OCTETS, &regValHi);
    tpm_swport_pm_3->Octets1023 = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_FRAMES_1024_TO_MAX_OCTETS, &regValHi);
    tpm_swport_pm_3->OctetsMax = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_GOOD_OCTETS_SENT_LOW, &regValHi);
    tpm_swport_pm_3->OutOctetsLo = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_GOOD_OCTETS_SENT_HIGH, &regValHi);
    tpm_swport_pm_3->OutOctetsHi = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_EXCESSIVE_COLLISION, &regValHi);
    tpm_swport_pm_1->excessiveCollisionCounter = regVaLo;
    tpm_swport_pm_3->Excessive = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_MULTICAST_FRAMES_SENT, &regValHi);
    tpm_swport_pm_3->OutMulticasts = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_BROADCAST_FRAMES_SENT, &regValHi);
    tpm_swport_pm_3->OutBroadcasts = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_FC_SENT, &regValHi);
    tpm_swport_pm_3->OutPause = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_GOOD_FC_RECEIVED, &regValHi);
    tpm_swport_pm_3->InPause = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_UNDERSIZE_RECEIVED, &regValHi);
    tpm_swport_pm_3->Undersize = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_FRAGMENTS_RECEIVED, &regValHi);
    tpm_swport_pm_3->Fragments = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_OVERSIZE_RECEIVED, &regValHi);
    tpm_swport_pm_1->frameTooLongs = regVaLo;
    tpm_swport_pm_3->Oversize = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_JABBER_RECEIVED, &regValHi);
    tpm_swport_pm_3->Jabber = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_MAC_RECEIVE_ERROR, &regValHi);
    tpm_swport_pm_3->InMACRcvErr = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_BAD_CRC_EVENT, &regValHi);
    tpm_swport_pm_3->InFCSErr = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_COLLISION, &regValHi);
    tpm_swport_pm_3->Collisions = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_LATE_COLLISION, &regValHi);
    tpm_swport_pm_3->Late = regVaLo;

    regVaLo = mvNetaMibCounterRead(port, mib, ETH_MIB_INTERNAL_MAC_TRANSMIT_ERR, &regValHi);
    tpm_swport_pm_1->internalMacTransmitErrorCounter = regVaLo;

    return TPM_RC_OK;
}

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
tpm_error_code_t tpm_sw_pm_print_from_gmac(int port)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    tpm_swport_pm_3_all_t tpm_swport_pm_3;
    tpm_swport_pm_1_t     tpm_swport_pm_1;
    MV_NETA_PORT_CTRL *pPortCtrl = mvNetaPortHndlGet(port);

    retVal = tpm_sw_pm_from_gmac(port, &tpm_swport_pm_1, &tpm_swport_pm_3);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        return retVal;
    }
    printk("=======Counters from port[%d]=======\n",port);
    printk("InGoodOctetsLo  %08d    ", tpm_swport_pm_3.InGoodOctetsLo);
    printk("InGoodOctetsHi  %08d   \n", tpm_swport_pm_3.InGoodOctetsHi);
    printk("InBadOctets     %08d    ", tpm_swport_pm_3.InBadOctets);
    printk("OutFCSErr       %08d   \n", tpm_swport_pm_3.OutFCSErr);
    printk("InUnicasts      %08d    ", tpm_swport_pm_3.InUnicasts);
    printk("Deferred        %08d   \n", tpm_swport_pm_3.Deferred);
    printk("InBroadcasts    %08d    ", tpm_swport_pm_3.InBroadcasts);
    printk("InMulticasts    %08d   \n", tpm_swport_pm_3.InMulticasts);
    printk("64Octets        %08d    ", tpm_swport_pm_3.Octets64);
    printk("127Octets       %08d   \n", tpm_swport_pm_3.Octets127);
    printk("255Octets       %08d    ", tpm_swport_pm_3.Octets255);
    printk("511Octets       %08d   \n", tpm_swport_pm_3.Octets511);
    printk("1023Octets      %08d    ", tpm_swport_pm_3.Octets1023);
    printk("MaxOctets       %08d   \n", tpm_swport_pm_3.OctetsMax);
    printk("OutOctetsLo     %08d    ", tpm_swport_pm_3.OutOctetsLo);
    printk("OutOctetsHi     %08d   \n", tpm_swport_pm_3.OutOctetsHi);
    printk("OutUnicasts     %08d    ", tpm_swport_pm_3.OutUnicasts);
    printk("Excessive       %08d   \n", tpm_swport_pm_3.Excessive);
    printk("OutMulticasts   %08d    ", tpm_swport_pm_3.OutMulticasts);
    printk("OutBroadcasts   %08d   \n", tpm_swport_pm_3.OutBroadcasts);
    printk("Single          %08d    ", tpm_swport_pm_3.Single);
    printk("OutPause        %08d   \n", tpm_swport_pm_3.OutPause);
    printk("InPause         %08d    ", tpm_swport_pm_3.InPause);
    printk("Multiple        %08d   \n", tpm_swport_pm_3.Multiple);
    printk("Undersize       %08d    ", tpm_swport_pm_3.Undersize);
    printk("Fragments       %08d   \n", tpm_swport_pm_3.Fragments);
    printk("Oversize        %08d    ", tpm_swport_pm_3.Oversize);
    printk("Jabber          %08d   \n", tpm_swport_pm_3.Jabber);
    printk("InMACRcvErr     %08d    ", tpm_swport_pm_3.InMACRcvErr);
    printk("InFCSErr        %08d   \n", tpm_swport_pm_3.InFCSErr);
    printk("Collisions      %08d    ", tpm_swport_pm_3.Collisions);
    printk("Late            %08d   \n", tpm_swport_pm_3.Late);

    /* Flush all counters */
    mvNetaMibCountersClear(port, pPortCtrl->txpNum);

    return retVal;
}

/*******************************************************************************
* tpm_sw_pm_1_read
*
*
* INPUTS:
*       wner_id         - APP owner id  should be used for all API calls.
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
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_STATS_COUNTER_SET3 statsCounterSet;
    uint32_t         switch_init;
    tpm_gmacs_enum_t gmac_port;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;
    tpm_swport_pm_3_all_t tpm_swport_pm_3;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d]\n\r",
               __FUNCTION__,owner_id, src_port);
    }

    /* Check swicth init or not */
    if (tpm_db_switch_init_get(&switch_init) != TPM_DB_OK) {
        printk(KERN_ERR "ERROR: (%s:%d) tpm_db_switch_init_get failed\n", __FUNCTION__, __LINE__);
        return ERR_GENERAL;
    }

    if (switch_init) {
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_get_port_counters(lPort, &statsCounterSet);
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        }

        tpm_swport_pm_1->fcsErrors                       = statsCounterSet.InFCSErr;
        tpm_swport_pm_1->excessiveCollisionCounter       = statsCounterSet.Excessive;
        tpm_swport_pm_1->lateCollisionCounter            = statsCounterSet.Late;
        tpm_swport_pm_1->frameTooLongs                   = statsCounterSet.Oversize;
        tpm_swport_pm_1->bufferOverflowsOnReceive        = 0;
        tpm_swport_pm_1->bufferOverflowsOnTransmit       = 0;
        tpm_swport_pm_1->singleCollisionFrameCounter     = statsCounterSet.Single;
        tpm_swport_pm_1->multipleCollisionsFrameCounter  = statsCounterSet.Multiple;
        tpm_swport_pm_1->sqeCounter                      = 0;
        tpm_swport_pm_1->deferredTransmissionCounter     = statsCounterSet.Deferred;
        tpm_swport_pm_1->internalMacTransmitErrorCounter = 0;
        tpm_swport_pm_1->carrierSenseErrorCounter        = 0;
        tpm_swport_pm_1->alignmentErrorCounter           = 0;
        tpm_swport_pm_1->internalMacReceiveErrorCounter  = statsCounterSet.InMACRcvErr;
    } else {
        retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
        if (retVal != TPM_RC_OK) {
            printk(KERN_ERR
                   "Port%d PHY access way check failed\n", src_port);
            return retVal;
        }
        if (PHY_SMI_MASTER_CPU == phy_access_way)/* check PHY access way */
        {
			for (gmac_port = TPM_ENUM_GMAC_0; gmac_port < TPM_MAX_NUM_GMACS; gmac_port++) {
				if (phy_direct_addr == mvBoardPhyAddrGet(gmac_port))
					break;
			}
			if (gmac_port == TPM_MAX_NUM_GMACS) {
				printk(KERN_ERR "ERROR: (%s:%d) Can not find gmac port\n", __FUNCTION__, __LINE__);
				return ERR_GENERAL;
			}
        }
        else if (PHY_SGMII == phy_access_way)
        {
        	gmac_port = 0;
        }

        /* Read counter from GMAC */
        retVal = tpm_sw_pm_from_gmac((int)gmac_port, tpm_swport_pm_1, &tpm_swport_pm_3);
        if (retVal != TPM_RC_OK) {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_sw_pm_3_from_gmac failed\n", __FUNCTION__, __LINE__);
            return ERR_GENERAL;
        }

        g_mc_tpm_swport_pm_1.fcsErrors                       += tpm_swport_pm_1->fcsErrors;
        g_mc_tpm_swport_pm_1.excessiveCollisionCounter       += tpm_swport_pm_1->excessiveCollisionCounter;
        g_mc_tpm_swport_pm_1.lateCollisionCounter            += tpm_swport_pm_1->lateCollisionCounter;
        g_mc_tpm_swport_pm_1.frameTooLongs                   += tpm_swport_pm_1->frameTooLongs;
        g_mc_tpm_swport_pm_1.bufferOverflowsOnReceive        += tpm_swport_pm_1->bufferOverflowsOnReceive;
        g_mc_tpm_swport_pm_1.bufferOverflowsOnTransmit       += tpm_swport_pm_1->bufferOverflowsOnTransmit;
        g_mc_tpm_swport_pm_1.singleCollisionFrameCounter     += tpm_swport_pm_1->singleCollisionFrameCounter;
        g_mc_tpm_swport_pm_1.multipleCollisionsFrameCounter  += tpm_swport_pm_1->multipleCollisionsFrameCounter;
        g_mc_tpm_swport_pm_1.sqeCounter                      += tpm_swport_pm_1->sqeCounter;
        g_mc_tpm_swport_pm_1.deferredTransmissionCounter     += tpm_swport_pm_1->deferredTransmissionCounter;
        g_mc_tpm_swport_pm_1.internalMacTransmitErrorCounter += tpm_swport_pm_1->internalMacTransmitErrorCounter;
        g_mc_tpm_swport_pm_1.carrierSenseErrorCounter        += tpm_swport_pm_1->carrierSenseErrorCounter;
        g_mc_tpm_swport_pm_1.alignmentErrorCounter           += tpm_swport_pm_1->alignmentErrorCounter;
        g_mc_tpm_swport_pm_1.internalMacReceiveErrorCounter  += tpm_swport_pm_1->internalMacReceiveErrorCounter;

	g_mc_tpm_swport_pm_3.dropEvents		+= tpm_swport_pm_3.dropEvents;
        g_mc_tpm_swport_pm_3.InGoodOctetsLo	+= tpm_swport_pm_3.InGoodOctetsLo;
        g_mc_tpm_swport_pm_3.InGoodOctetsHi	+= tpm_swport_pm_3.InGoodOctetsHi;
        g_mc_tpm_swport_pm_3.InBadOctets	+= tpm_swport_pm_3.InBadOctets;
        g_mc_tpm_swport_pm_3.OutFCSErr		+= tpm_swport_pm_3.OutFCSErr;
        g_mc_tpm_swport_pm_3.InUnicasts		+= tpm_swport_pm_3.InUnicasts;
        g_mc_tpm_swport_pm_3.Deferred		+= tpm_swport_pm_3.Deferred;
        g_mc_tpm_swport_pm_3.InBroadcasts	+= tpm_swport_pm_3.InBroadcasts;
        g_mc_tpm_swport_pm_3.InMulticasts	+= tpm_swport_pm_3.InMulticasts;
        g_mc_tpm_swport_pm_3.Octets64		+= tpm_swport_pm_3.Octets64;
        g_mc_tpm_swport_pm_3.Octets127		+= tpm_swport_pm_3.Octets127;
        g_mc_tpm_swport_pm_3.Octets255		+= tpm_swport_pm_3.Octets255;
        g_mc_tpm_swport_pm_3.Octets511		+= tpm_swport_pm_3.Octets511;
        g_mc_tpm_swport_pm_3.Octets1023		+= tpm_swport_pm_3.Octets1023;
        g_mc_tpm_swport_pm_3.OctetsMax		+= tpm_swport_pm_3.OctetsMax;
        g_mc_tpm_swport_pm_3.OutOctetsLo	+= tpm_swport_pm_3.OutOctetsLo;
        g_mc_tpm_swport_pm_3.OutOctetsHi	+= tpm_swport_pm_3.OutOctetsHi;
        g_mc_tpm_swport_pm_3.OutUnicasts	+= tpm_swport_pm_3.OutUnicasts;
        g_mc_tpm_swport_pm_3.Excessive		+= tpm_swport_pm_3.Excessive;
        g_mc_tpm_swport_pm_3.OutMulticasts	+= tpm_swport_pm_3.OutMulticasts;
        g_mc_tpm_swport_pm_3.OutBroadcasts	+= tpm_swport_pm_3.OutBroadcasts;
        g_mc_tpm_swport_pm_3.Single		+= tpm_swport_pm_3.Single;
        g_mc_tpm_swport_pm_3.OutPause		+= tpm_swport_pm_3.OutPause;
        g_mc_tpm_swport_pm_3.InPause		+= tpm_swport_pm_3.InPause;
        g_mc_tpm_swport_pm_3.Multiple		+= tpm_swport_pm_3.Multiple;
        g_mc_tpm_swport_pm_3.Undersize		+= tpm_swport_pm_3.Undersize;
        g_mc_tpm_swport_pm_3.Fragments		+= tpm_swport_pm_3.Fragments;
        g_mc_tpm_swport_pm_3.Oversize		+= tpm_swport_pm_3.Oversize;
        g_mc_tpm_swport_pm_3.Jabber		+= tpm_swport_pm_3.Jabber;
        g_mc_tpm_swport_pm_3.InMACRcvErr	+= tpm_swport_pm_3.InMACRcvErr;
        g_mc_tpm_swport_pm_3.InFCSErr		+= tpm_swport_pm_3.InFCSErr;
        g_mc_tpm_swport_pm_3.Collisions		+= tpm_swport_pm_3.Collisions;
        g_mc_tpm_swport_pm_3.Late		+= tpm_swport_pm_3.Late;

	memcpy(tpm_swport_pm_1, &g_mc_tpm_swport_pm_1, sizeof(g_mc_tpm_swport_pm_1));
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_pm_3_read
*
*
* INPUTS:
*       owner_id        - APP owner id  should be used for all API calls.
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
    tpm_swport_pm_3_all_t *tpm_swport_pm_3
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    GT_STATS_COUNTER_SET3 statsCounterSet;
    GT_PORT_STAT2         ctr;
    uint32_t         switch_init;
    tpm_gmacs_enum_t gmac_port;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;
    tpm_swport_pm_1_t   tpm_swport_pm_1;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d]\n\r",
               __FUNCTION__,owner_id, src_port);
    }

    /* Initialize statsCounterSet and ctr as zero */
    memset(&statsCounterSet, 0, sizeof(GT_STATS_COUNTER_SET3));
    memset(&ctr, 0, sizeof(GT_PORT_STAT2));

    /* Check swicth init or not */
    if (tpm_db_switch_init_get(&switch_init) != TPM_DB_OK) {
        printk(KERN_ERR "ERROR: (%s:%d) tpm_db_switch_init_get failed\n", __FUNCTION__, __LINE__);
        return ERR_GENERAL;
    }

    if (switch_init) {
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_get_port_counters(lPort, &statsCounterSet);
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function mv_switch_get_port_counters failed\r\n", __FUNCTION__,__LINE__);
        }

        retVal = mv_switch_get_port_drop_counters(lPort, &ctr);
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function mv_switch_get_port_drop_counters failed\r\n", __FUNCTION__,__LINE__);
        }

        tpm_swport_pm_3->dropEvents	  = (ctr.inDiscardHi<<16) + ctr.inDiscardLo;
        tpm_swport_pm_3->InGoodOctetsLo   = statsCounterSet.InGoodOctetsLo;
        tpm_swport_pm_3->InGoodOctetsHi   = statsCounterSet.InGoodOctetsHi;
        tpm_swport_pm_3->InBadOctets	  = statsCounterSet.InBadOctets;
        tpm_swport_pm_3->OutFCSErr	  = statsCounterSet.OutFCSErr;
        tpm_swport_pm_3->InUnicasts	  = statsCounterSet.InUnicasts;
        tpm_swport_pm_3->Deferred	  = statsCounterSet.Deferred;
        tpm_swport_pm_3->InBroadcasts	  = statsCounterSet.InBroadcasts;
        tpm_swport_pm_3->InMulticasts	  = statsCounterSet.InMulticasts;
        tpm_swport_pm_3->Octets64	  = statsCounterSet.Octets64;
        tpm_swport_pm_3->Octets127	  = statsCounterSet.Octets127;
        tpm_swport_pm_3->Octets255	  = statsCounterSet.Octets255;
        tpm_swport_pm_3->Octets511	  = statsCounterSet.Octets511;
        tpm_swport_pm_3->Octets1023	  = statsCounterSet.Octets1023;
        tpm_swport_pm_3->OctetsMax	  = statsCounterSet.OctetsMax;
        tpm_swport_pm_3->OutOctetsLo	  = statsCounterSet.OutOctetsLo;
        tpm_swport_pm_3->OutOctetsHi	  = statsCounterSet.OutOctetsHi;
        tpm_swport_pm_3->OutUnicasts	  = statsCounterSet.OutUnicasts;
        tpm_swport_pm_3->Excessive	  = statsCounterSet.Excessive;
        tpm_swport_pm_3->OutMulticasts	  = statsCounterSet.OutMulticasts;
        tpm_swport_pm_3->OutBroadcasts	  = statsCounterSet.OutBroadcasts;
        tpm_swport_pm_3->Single 	  = statsCounterSet.Single;
        tpm_swport_pm_3->OutPause	  = statsCounterSet.OutPause;
        tpm_swport_pm_3->InPause	  = statsCounterSet.InPause;
        tpm_swport_pm_3->Multiple	  = statsCounterSet.Multiple;
        tpm_swport_pm_3->Undersize	  = statsCounterSet.Undersize;
        tpm_swport_pm_3->Fragments	  = statsCounterSet.Fragments;
        tpm_swport_pm_3->Oversize	  = statsCounterSet.Oversize;
        tpm_swport_pm_3->Jabber 	  = statsCounterSet.Jabber;
        tpm_swport_pm_3->InMACRcvErr	  = statsCounterSet.InMACRcvErr;
        tpm_swport_pm_3->InFCSErr	  = statsCounterSet.InFCSErr;
        tpm_swport_pm_3->Collisions	  = statsCounterSet.Collisions;
        tpm_swport_pm_3->Late		  = statsCounterSet.Late;
    } else {
        /* check PHY access way */
        retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
        if (retVal != TPM_RC_OK) {
            printk(KERN_ERR
                   "Port%d PHY access way check failed\n", src_port);
            return retVal;
        }
        if (PHY_SMI_MASTER_CPU == phy_access_way)/* check PHY access way */
        {
			for (gmac_port = TPM_ENUM_GMAC_0; gmac_port < TPM_MAX_NUM_GMACS; gmac_port++) {
				if (phy_direct_addr == mvBoardPhyAddrGet(gmac_port))
					break;
			}
			if (gmac_port == TPM_MAX_NUM_GMACS) {
				printk(KERN_ERR "ERROR: (%s:%d) Can not find gmac port\n", __FUNCTION__, __LINE__);
				return ERR_GENERAL;
			}
        }
        else if (PHY_SGMII == phy_access_way)
        {
        	gmac_port = 0;
        }

        /* Read counter from GMAC */
        retVal = tpm_sw_pm_from_gmac((int)gmac_port, &tpm_swport_pm_1, tpm_swport_pm_3);
        if (retVal != TPM_RC_OK) {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_sw_pm_3_from_gmac failed\n", __FUNCTION__, __LINE__);
            return ERR_GENERAL;
        }

	g_mc_tpm_swport_pm_1.fcsErrors                       += tpm_swport_pm_1.fcsErrors;
        g_mc_tpm_swport_pm_1.excessiveCollisionCounter       += tpm_swport_pm_1.excessiveCollisionCounter;
        g_mc_tpm_swport_pm_1.lateCollisionCounter            += tpm_swport_pm_1.lateCollisionCounter;
        g_mc_tpm_swport_pm_1.frameTooLongs                   += tpm_swport_pm_1.frameTooLongs;
        g_mc_tpm_swport_pm_1.bufferOverflowsOnReceive        += tpm_swport_pm_1.bufferOverflowsOnReceive;
        g_mc_tpm_swport_pm_1.bufferOverflowsOnTransmit       += tpm_swport_pm_1.bufferOverflowsOnTransmit;
        g_mc_tpm_swport_pm_1.singleCollisionFrameCounter     += tpm_swport_pm_1.singleCollisionFrameCounter;
        g_mc_tpm_swport_pm_1.multipleCollisionsFrameCounter  += tpm_swport_pm_1.multipleCollisionsFrameCounter;
        g_mc_tpm_swport_pm_1.sqeCounter                      += tpm_swport_pm_1.sqeCounter;
        g_mc_tpm_swport_pm_1.deferredTransmissionCounter     += tpm_swport_pm_1.deferredTransmissionCounter;
        g_mc_tpm_swport_pm_1.internalMacTransmitErrorCounter += tpm_swport_pm_1.internalMacTransmitErrorCounter;
        g_mc_tpm_swport_pm_1.carrierSenseErrorCounter        += tpm_swport_pm_1.carrierSenseErrorCounter;
        g_mc_tpm_swport_pm_1.alignmentErrorCounter           += tpm_swport_pm_1.alignmentErrorCounter;
        g_mc_tpm_swport_pm_1.internalMacReceiveErrorCounter  += tpm_swport_pm_1.internalMacReceiveErrorCounter;

	g_mc_tpm_swport_pm_3.dropEvents		+= tpm_swport_pm_3->dropEvents;
        g_mc_tpm_swport_pm_3.InGoodOctetsLo	+= tpm_swport_pm_3->InGoodOctetsLo;
        g_mc_tpm_swport_pm_3.InGoodOctetsHi	+= tpm_swport_pm_3->InGoodOctetsHi;
        g_mc_tpm_swport_pm_3.InBadOctets	+= tpm_swport_pm_3->InBadOctets;
        g_mc_tpm_swport_pm_3.OutFCSErr		+= tpm_swport_pm_3->OutFCSErr;
        g_mc_tpm_swport_pm_3.InUnicasts		+= tpm_swport_pm_3->InUnicasts;
        g_mc_tpm_swport_pm_3.Deferred		+= tpm_swport_pm_3->Deferred;
        g_mc_tpm_swport_pm_3.InBroadcasts	+= tpm_swport_pm_3->InBroadcasts;
        g_mc_tpm_swport_pm_3.InMulticasts	+= tpm_swport_pm_3->InMulticasts;
        g_mc_tpm_swport_pm_3.Octets64		+= tpm_swport_pm_3->Octets64;
        g_mc_tpm_swport_pm_3.Octets127		+= tpm_swport_pm_3->Octets127;
        g_mc_tpm_swport_pm_3.Octets255		+= tpm_swport_pm_3->Octets255;
        g_mc_tpm_swport_pm_3.Octets511		+= tpm_swport_pm_3->Octets511;
        g_mc_tpm_swport_pm_3.Octets1023		+= tpm_swport_pm_3->Octets1023;
        g_mc_tpm_swport_pm_3.OctetsMax		+= tpm_swport_pm_3->OctetsMax;
        g_mc_tpm_swport_pm_3.OutOctetsLo	+= tpm_swport_pm_3->OutOctetsLo;
        g_mc_tpm_swport_pm_3.OutOctetsHi	+= tpm_swport_pm_3->OutOctetsHi;
        g_mc_tpm_swport_pm_3.OutUnicasts	+= tpm_swport_pm_3->OutUnicasts;
        g_mc_tpm_swport_pm_3.Excessive		+= tpm_swport_pm_3->Excessive;
        g_mc_tpm_swport_pm_3.OutMulticasts	+= tpm_swport_pm_3->OutMulticasts;
        g_mc_tpm_swport_pm_3.OutBroadcasts	+= tpm_swport_pm_3->OutBroadcasts;
        g_mc_tpm_swport_pm_3.Single		+= tpm_swport_pm_3->Single;
        g_mc_tpm_swport_pm_3.OutPause		+= tpm_swport_pm_3->OutPause;
        g_mc_tpm_swport_pm_3.InPause		+= tpm_swport_pm_3->InPause;
        g_mc_tpm_swport_pm_3.Multiple		+= tpm_swport_pm_3->Multiple;
        g_mc_tpm_swport_pm_3.Undersize		+= tpm_swport_pm_3->Undersize;
        g_mc_tpm_swport_pm_3.Fragments		+= tpm_swport_pm_3->Fragments;
        g_mc_tpm_swport_pm_3.Oversize		+= tpm_swport_pm_3->Oversize;
        g_mc_tpm_swport_pm_3.Jabber		+= tpm_swport_pm_3->Jabber;
        g_mc_tpm_swport_pm_3.InMACRcvErr	+= tpm_swport_pm_3->InMACRcvErr;
        g_mc_tpm_swport_pm_3.InFCSErr		+= tpm_swport_pm_3->InFCSErr;
        g_mc_tpm_swport_pm_3.Collisions		+= tpm_swport_pm_3->Collisions;
        g_mc_tpm_swport_pm_3.Late		+= tpm_swport_pm_3->Late;

	memcpy(tpm_swport_pm_3, &g_mc_tpm_swport_pm_3, sizeof(g_mc_tpm_swport_pm_3));
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

/*******************************************************************************
* tpm_sw_clear_port_counter
*
* DESCRIPTION:
*       The API clear port pm counter.
*
* INPUTS:
*       owner_id    - APP owner id should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       none.
*
* RETURNS:
*       On success - TPM_RC_OK.
*       On error different types are returned according to the case  see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/

tpm_error_code_t tpm_sw_clear_port_counter
(
     uint32_t            owner_id,
     tpm_src_port_type_t src_port
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d]\n\r",
               __FUNCTION__,owner_id, src_port);
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_clean_port_counters(lPort);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}
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
     uint32_t owner_id,
     uint16_t db_num
)
{
    tpm_error_code_t retVal = TPM_RC_OK;

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==ENTER==%s: owner_id[%d] db_num[%d]\n\r",
                __FUNCTION__,owner_id,db_num);
    }

    if (tpm_sw_init_check()) {
        /* Check GMAC1 lpk status, if no lpk, no way to add static MAC */
#ifdef CONFIG_MV_MAC_LEARN
        if (tpm_db_gmac1_lpbk_en_get())
            mac_learn_db_valid_print();
        else {
            printk(KERN_ERR "ERROR: (%s:%d) MAC learn not supported\n", __FUNCTION__, __LINE__);
            return ERR_GENERAL;
        }
#endif
    } else {
        retVal =  mv_switch_print_fdb(db_num);
    }
    if ((retVal != TPM_RC_OK) && (retVal != GT_NO_SUCH))
    {
        printk(KERN_INFO
               "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}
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
tpm_error_code_t tpm_sw_flush_vtu(uint32_t owner_id)
{
    tpm_error_code_t retVal;

    SWITCH_INIT_CHECK();

    retVal = mv_switch_flush_vtu();
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
              "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    return retVal;
}
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
tpm_error_code_t tpm_sw_flush_atu(uint32_t owner_id, tpm_flush_atu_type_t flush_type, uint16_t db_num)
{
    tpm_error_code_t retVal;
    GT_FLUSH_CMD flush_cmd;

    SWITCH_INIT_CHECK();

    if (TPM_FLUSH_ATU_ALL == flush_type)
        flush_cmd = GT_FLUSH_ALL;
    else
        flush_cmd = GT_FLUSH_ALL_UNLOCKED;

    if (0 != db_num)
    {
        printk(KERN_ERR
              "%s:%d: function failed, db_num should be 0\r\n", __FUNCTION__,__LINE__);
    }

    retVal = mv_switch_flush_atu(flush_cmd, db_num);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
              "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    return retVal;
}

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
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t          lPort  = 0;
    tpm_phy_ctrl_t   phy_access_way;
    uint32_t         phy_direct_addr;
    GT_BOOL          state;

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d],speed[%d]\n\r",
               __FUNCTION__,owner_id,src_port, (uint32_t)speed);
    }

    if(enable == true)
    {
        state = GT_TRUE;
    }
    else
    {
        state = GT_FALSE;
    }

    /* check PHY access way */
    retVal = tpm_phy_access_check(src_port, &phy_access_way, &phy_direct_addr);
    if (retVal != TPM_RC_OK) {
        printk(KERN_ERR
               "Port%d PHY access way check failed\n", src_port);
        return retVal;
    }
    /* PHY accessed directly, call lsp API */
    if (PHY_SMI_MASTER_CPU == phy_access_way) {
        if (mvEthPhySpeedDuplexModeSet(phy_direct_addr, speed, state)) {
            printk(KERN_ERR
                   "ERROR: (%s:%d) PHY LSP API call failed on port(%d)\n", __FUNCTION__, __LINE__, src_port);
            retVal = ERR_PHY_SRC_PORT_CONN_INVALID;
        } else {
            /* Update GMAC speed, duplex and flow control */
            retVal = tpm_sw_set_gmac_speed_duplex_fc(owner_id, src_port, false);
            if (retVal != TPM_RC_OK) {
                printk(KERN_ERR "Failed to update GMAC config with port%d\n", src_port);
                return retVal;
            }
        }
    }
    else if (PHY_SGMII == phy_access_way)
    {
        /* PHY accessed through switch, as original do */
        lPort = tpm_db_eth_port_switch_port_get(src_port);
        if (lPort == TPM_DB_ERR_PORT_NUM)
        {
            printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
            return ERR_SRC_PORT_INVALID;
        }

        retVal = mv_switch_set_port_speed_duplex_mode(lPort, (GT_PHY_SPEED)speed, state);
        if (retVal != TPM_RC_OK)
        {
            printk(KERN_ERR
                   "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
        }
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

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
)
{
    tpm_error_code_t retVal = TPM_RC_OK;
    int32_t lPort = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],src_port[%d],vid[%d],egress mode[%d]\n",
               __FUNCTION__,owner_id,src_port,vid,eMode);
    }

    if (vid >= TPM_MAX_VID)
    {
        printk(KERN_INFO
           "%s:%d:==ERROR== invalid VID[%d]\r\n", __FUNCTION__,__LINE__,vid);
        return ERR_SW_VID_INVALID;
    }

    lPort = tpm_db_eth_port_switch_port_get(src_port);
    if (lPort == TPM_DB_ERR_PORT_NUM)
    {
        printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __FUNCTION__, __LINE__, src_port);
        return ERR_SRC_PORT_INVALID;
    }

    retVal = mv_switch_port_add_vid_set_egrs_mode(lPort, vid, TPM_GMAC0_AMBER_PORT_NUM, eMode);
    if (retVal != TPM_RC_OK)
    {
        printk(KERN_ERR
           "%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
    }

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
            "==EXIT== %s:\n\r",__FUNCTION__);
    }

    return retVal;
}

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
)
{
    int32_t int_ret;
    uint32_t switch_init;

    int_ret = tpm_db_switch_init_get(&switch_init);
    if (int_ret)
        printk(KERN_INFO "tpm_db_switch_init_get failed\n");
    if (!switch_init) {
        printk(KERN_INFO "switch is not allowed to init\n");
        return;
    }

    if (trace_sw_dbg_flag)
    {
       printk(KERN_INFO
               "==ENTER==%s:\n\r",__FUNCTION__);
    }

    mv_switch_drv_init();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
                "==EXIT== %s\n\r",__FUNCTION__);
    }
}

/*******************************************************************************
* tpm_sw_set_static_multicast_mac
*
* DESCRIPTION:
*       This function creates or destory a static MAC entry in the MAC address table for taget
*       UNI port bitmap in the integrated switch
*
* INPUTS:
*       owner_id    	- APP owner id, should be used for all API calls.
*       tpm_trg_port    - target port bm in UNI port index, TPM_TRG_UNI_0 | TPM_TRG_UNI_1 | ...
*       static_mac  	- 6 byte network order MAC source address.
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
tpm_error_code_t tpm_sw_set_static_multicast_mac
(
    uint32_t            owner_id,
    tpm_trg_port_type_t tpm_trg_port,
    uint8_t             static_mac[6]
)
{
    int32_t ret_code;
    uint32_t trg_port_bm = 0;

    SWITCH_INIT_CHECK();

    if (trace_sw_dbg_flag)
    {
        printk(KERN_INFO
               "==ENTER==%s: owner_id[%d],tpm_trg_port[0x%x],static_mac [0x%02x%02x%02x%02x%02x%02x]\n",
               __FUNCTION__,owner_id,tpm_trg_port,static_mac[0],static_mac[1], static_mac[2],
               static_mac[3],static_mac[4], static_mac[5]);
    }

	if((static_mac[0] != 0x01) && (static_mac[0] != 0x33 || static_mac[1] != 0x33))
    {
    	printk(KERN_ERR
           "%s:%d: function failed, 0x%x:%x:%x:%x:%x:%x is not multicast address \r\n", 
           __FUNCTION__,__LINE__, static_mac[0], static_mac[1], static_mac[2], static_mac[3], 
           static_mac[4], static_mac[5]);
    	IF_ERROR(ERR_SW_MAC_INVALID);
    }
    
    trg_port_bm = tpm_db_trg_port_switch_port_get(tpm_trg_port);

    if (tpm_trg_port == 0) {
        ret_code = tpm_sw_del_static_mac(TPM_MOD_OWNER_TPM, static_mac);
        IF_ERROR(ret_code);
    } else {
        ret_code = tpm_sw_set_static_mac_w_ports_mask(TPM_MOD_OWNER_TPM, trg_port_bm, static_mac);
        IF_ERROR(ret_code);
    }

    return(TPM_RC_OK);
}


