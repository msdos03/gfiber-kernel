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
#include "tpm_types.h"
#include "tpm_internal_types.h"
#include "tpm_xml_params.h"
#include "ezxml.h"

char *g_pstr_xml_cfg_file = US_XML_CFG_FILE;
static ezxml_t g_p_xmlHead = NULL;

/*******************************************************************************
*                          Internal functions
*******************************************************************************/
static ezxml_t get_xml_head_ptr(char *cfg_file)
{
	if (g_p_xmlHead == NULL) {
		g_p_xmlHead = ezxml_parse_file(cfg_file);

		if (g_p_xmlHead == NULL)
			printk(KERN_ERR "Failed to open XML configuration file - %s\n", cfg_file);
	}

	return g_p_xmlHead;
}

void free_xml_head_ptr(void)
{
	if (g_p_xmlHead != NULL) {
		ezxml_free(g_p_xmlHead);
		g_p_xmlHead = NULL;
	}
}

static uint32_t get_dig_number(const char *arg)
{
	uint32_t val = 0;

	if ((arg[1] == 'x') || (arg[1] == 'X'))
		sscanf(&arg[2], "%x", &val);
	else
		val = simple_strtoul(arg, NULL, 10);

	return val;
}

static int get_int_param(ezxml_t xml, char *name, uint32_t *num)
{
	ezxml_t xmlElement;

	xmlElement = ezxml_child(xml, name);
	if (xmlElement == NULL)
		return US_RC_NOT_FOUND;
	else if (xmlElement->txt == NULL) {
		printk(KERN_ERR "NULL value for %s in XML config. file\n", name);
		return US_RC_NOT_FOUND;
	}

	*num = get_dig_number(xmlElement->txt);
	return US_RC_OK;
}

static int get_char_param(ezxml_t xml, char *name, uint8_t *str, uint8_t len)
{
	ezxml_t xmlElement;

	xmlElement = ezxml_child(xml, name);
	if (xmlElement == NULL) {
		printk(KERN_ERR "Failed to find %s in XML config. file\n", name);
		return US_RC_NOT_FOUND;
	} else if (xmlElement->txt == NULL) {
		printk(KERN_ERR "NULL value for %s in XML config. file\n", name);
		return US_RC_NOT_FOUND;
	}

	memcpy(str, xmlElement->txt, len);
	str[len] = 0;
	return US_RC_OK;
}

static int get_debug_port_attrs(ezxml_t xml, uint32_t *num, uint32_t *valid)
{
	const char *attr_str;
	int rc = US_RC_FAIL;

	attr_str = ezxml_attr(xml, US_XML_ID_ATTR);
	if (attr_str != NULL) {
		*num = get_dig_number(attr_str);
		attr_str = ezxml_attr(xml, US_XML_VALID_ATTR);
		if (attr_str != NULL) {
			*valid = get_dig_number(attr_str);
			rc = US_RC_OK;
		} else
			printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_VALID_ATTR);
	} else
		printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_ID_ATTR);

	return rc;
}

static int get_rx_queue_attrs(ezxml_t xml, uint32_t gmac, tpm_init_gmac_rx_t *gmac_rx, int max_rx_queues_num)
{
	const char *attr_str;
	int que_id;

	attr_str = ezxml_attr(xml, US_XML_ID_ATTR);
	if (attr_str == NULL) {
		printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_ID_ATTR);
		return (US_RC_FAIL);
	}
	que_id = get_dig_number(attr_str);

	if (que_id >= max_rx_queues_num) {
		printk(KERN_ERR "%s: Invalid ID number %s\n", __func__, attr_str);
		return (US_RC_FAIL);
	}

	attr_str = ezxml_attr(xml, US_XML_SIZE_ATTR);
	if (attr_str == NULL) {
		printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_SIZE_ATTR);
		return (US_RC_FAIL);
	}

	gmac_rx[gmac].rx_queue[que_id].queue_size = get_dig_number(attr_str);
	gmac_rx[gmac].valid = 1;
	gmac_rx[gmac].rx_queue[que_id].valid = 1;

	return US_RC_OK;
}

static int get_tx_queue_attrs(ezxml_t xml, uint32_t port, tpm_init_gmac_tx_t *gmac_tx, int max_tx_queues_num)
{
	const char *attr_str;
	int que_id;
	int rc = US_RC_FAIL;

	attr_str = ezxml_attr(xml, US_XML_ID_ATTR);
	if (attr_str != NULL) {
		que_id = get_dig_number(attr_str);

		if (que_id < max_tx_queues_num) {
			attr_str = ezxml_attr(xml, US_XML_QUEUE_SHED_ATTR);
			if (attr_str != NULL) {
				gmac_tx[port].tx_queue[que_id].sched_method = get_dig_number(attr_str);
				attr_str = ezxml_attr(xml, US_XML_QUEUE_OWNER_ATTR);
				if (attr_str != NULL) {
					gmac_tx[port].tx_queue[que_id].queue_owner = get_dig_number(attr_str);
					attr_str = ezxml_attr(xml, US_XML_QUEUE_OW_Q_NUM_ATTR);
					if (attr_str != NULL) {
						gmac_tx[port].tx_queue[que_id].owner_queue_num =
							get_dig_number(attr_str);
						attr_str = ezxml_attr(xml, US_XML_SIZE_ATTR);
						if (attr_str != NULL) {
							gmac_tx[port].tx_queue[que_id].queue_size =
								get_dig_number(attr_str);
							attr_str = ezxml_attr(xml, US_XML_QUEUE_WEIGHT_ATTR);
							if (attr_str != NULL) {
								gmac_tx[port].tx_queue[que_id].queue_weight =
									get_dig_number(attr_str);
								gmac_tx[port].valid = 1;
								gmac_tx[port].tx_queue[que_id].valid = 1;
								rc = US_RC_OK;
							} else
								printk(KERN_ERR "%s: Failed to get %s\n", __func__,
									US_XML_QUEUE_WEIGHT_ATTR);
						} else
							printk(KERN_ERR "%s: Failed to get %s\n", __func__,
								US_XML_SIZE_ATTR);
					} else
						printk(KERN_ERR "%s: Failed to get %s\n", __func__,
							US_XML_QUEUE_OW_Q_NUM_ATTR);
				} else
					printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_QUEUE_OWNER_ATTR);
			} else
				printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_QUEUE_SHED_ATTR);
		} else
			printk(KERN_ERR "%s: Invalid ID number %s\n", __func__, attr_str);
	} else
		printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_ID_ATTR);

	return rc;
}

static int get_pnc_range_attrs(ezxml_t xml, tpm_init_pnc_range_t *pnc_range, int max_pnc_ranges)
{
	const char *attr_str;
	int range_num;
	int rc = US_RC_FAIL;

	attr_str = ezxml_attr(xml, US_XML_NUM_ATTR);
	if (attr_str != NULL) {
		range_num = get_dig_number(attr_str);
		if (range_num < max_pnc_ranges) {
			pnc_range[range_num].range_num = range_num;
			attr_str = ezxml_attr(xml, US_XML_TYPE_ATTR);
			if (attr_str != NULL) {
				pnc_range[range_num].range_type = get_dig_number(attr_str);
				attr_str = ezxml_attr(xml, US_XML_SIZE_ATTR);
				if (attr_str != NULL) {
					pnc_range[range_num].range_size = get_dig_number(attr_str);
					attr_str = ezxml_attr(xml, US_XML_TPM_CNTR_GRP_ATTR);
					if (attr_str != NULL) {
						pnc_range[range_num].cntr_grp = get_dig_number(attr_str);
						attr_str = ezxml_attr(xml, US_XML_TPM_LU_MASK_ATTR);
						if (attr_str != NULL) {
							pnc_range[range_num].lu_mask = get_dig_number(attr_str);
							attr_str = ezxml_attr(xml, US_XML_TPM_MIN_RES_LVL_ATTR);
							if (attr_str != NULL) {
								pnc_range[range_num].min_reset_level =
									get_dig_number(attr_str);
								pnc_range[range_num].valid = 1;
								rc = US_RC_OK;
							} else
								printk(KERN_ERR "%s: Failed to get %s\n", __func__,
									US_XML_TPM_MIN_RES_LVL_ATTR);
						} else
							printk(KERN_ERR "%s: Failed to get %s\n", __func__,
								US_XML_TPM_LU_MASK_ATTR);
					} else
						printk(KERN_ERR "%s: Failed to get %s\n", __func__,
							US_XML_TPM_CNTR_GRP_ATTR);
				} else
					printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_SIZE_ATTR);
			} else
				printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_TYPE_ATTR);
		} else
			printk(KERN_ERR "%s: Invalid Range number %s\n", __func__, attr_str);
	} else
		printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_NUM_ATTR);

	return rc;
}

static int get_mod_chain_attrs(ezxml_t xml, tpm_init_chain_data_t *chain_data)
{
	const char *attr_str;
	int rc = US_RC_FAIL;

	attr_str = ezxml_attr(xml, US_XML_TPM_CHAIN_TYPE_ATTR);
	if (attr_str != NULL) {
		chain_data->type = get_dig_number(attr_str);
		attr_str = ezxml_attr(xml, US_XML_TPM_CHAIN_NUM_ATTR);
		if (attr_str != NULL) {
			chain_data->num = get_dig_number(attr_str);
			return US_RC_OK;
		} else
			printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_TPM_CHAIN_NUM_ATTR);
	} else
		printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_TPM_CHAIN_TYPE_ATTR);

	return rc;
}

static int get_vlan_ety_attrs(ezxml_t xml, uint32_t *etypes, int max_etypes)
{
	const char *attr_str;
	int reg_id;
	int rc = US_RC_FAIL;

	attr_str = ezxml_attr(xml, US_XML_ID_ATTR);
	if (attr_str != NULL) {
		reg_id = get_dig_number(attr_str);
		if (reg_id < max_etypes) {
			attr_str = ezxml_attr(xml, US_XML_TYPE_ATTR);
			if (attr_str != NULL) {
				etypes[reg_id] = get_dig_number(attr_str);
				rc = US_RC_OK;
			} else
				printk(KERN_ERR  "%s: Failed to get %s\n", __func__, US_XML_TYPE_ATTR);
		} else
			printk(KERN_ERR "%s: Invalid ID number %s\n", __func__, attr_str);
	} else
		printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_ID_ATTR);

	return rc;
}

static int get_filter_tpid_attrs(ezxml_t xml, tpm_init_tpid_comb_t *tpid)
{
	const char *attr_str;
	const char *v1_tpid = NULL;
	const char *v2_tpid = NULL;
	int rc = US_RC_FAIL;

	attr_str = ezxml_attr(xml, US_XML_TYPE_ATTR);
	if (attr_str != NULL) {
		v1_tpid = attr_str;
		v2_tpid = strstr(attr_str, ",");

		tpid->v1_tpid = get_dig_number(v1_tpid);
		if (v2_tpid != NULL) {
			v2_tpid++;
			tpid->v2_tpid = get_dig_number(v2_tpid);
		} else
			tpid->v2_tpid = MV_TPM_UN_INITIALIZED_INIT_PARAM;
		rc = US_RC_OK;
	} else
		printk("%s: Failed to get %s\n", __func__, US_XML_TYPE_ATTR);

	return rc;
}

/*******************************************************************************
* get_pon_type_param()
*
* DESCRIPTION:      Get WAN type from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  pon_type - EPON, GPON, ETH or P2P
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_pon_type_param(tpm_init_pon_type_t *pon_type)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc;
	uint32_t local_pon_type;

	if (pon_type == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*pon_type = TPM_NONE;

	xmlHead = ezxml_parse_file(US_XML_CFG_FILE_GLOBAL);
	if (xmlHead == NULL) {
		printk(KERN_ERR "Failed to open XML configuration file - %s\n", US_XML_CFG_FILE_GLOBAL);
		return US_RC_FAIL;
	}

 	xmlElement = ezxml_child(xmlHead, US_XML_WAN_E);
 	if (xmlElement == NULL) {
 		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_WAN_E, US_XML_CFG_FILE_GLOBAL);
 		rc = US_RC_NOT_FOUND;
 	} else
 		rc = get_int_param(xmlElement, US_XML_PON_TYPE_E, &local_pon_type);

	if (rc == US_RC_OK) {
		*pon_type = (tpm_init_pon_type_t) local_pon_type;

#ifdef US_DEBUG_PRINT
	{
       	char * pon_str[] = {"EPON", "GPON", "P2P", "NONE"};
		printk("pon_type = %s \n", pon_str[*pon_type]);
	}
#endif
	}

	ezxml_free(xmlHead);
	return rc;
}

/*******************************************************************************
* get_omci_etype_param()
*
* DESCRIPTION:      Get OMCI ETY from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  ety - Ethernet type
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_omci_etype_param(uint32_t *ety)
{
	int rc;
	ezxml_t xmlHead;
	ezxml_t xmlElement;

	if (ety == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*ety = 0;

	xmlHead = ezxml_parse_file(US_XML_CFG_FILE_OMCI);
	if (xmlHead == NULL) {
		printk(KERN_ERR "Failed to open XML configuration file - %s\n", US_XML_CFG_FILE_OMCI);
		return US_RC_FAIL;
	}

	xmlElement = ezxml_child(xmlHead, US_XML_OMCI_E);
	if (xmlElement == NULL) {
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_OMCI_E, US_XML_CFG_FILE_OMCI);
		rc = US_RC_NOT_FOUND;
	} else
		rc = get_int_param(xmlElement, US_XML_OMCI_ETY_E, ety);

	if (rc == US_RC_OK) {
#ifdef US_DEBUG_PRINT
		printk("OMCI ETY = 0x%4.4X\n", *ety);
#endif
	}

	ezxml_free(xmlHead);
	return rc;
}

/*******************************************************************************
* get_debug_port_params()
*
* DESCRIPTION:      Get debug port from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  valid - whether the debug port is valid
*           num   - debug port number
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_debug_port_params(uint32_t *valid, uint32_t *num)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if ((valid == NULL) || (num == NULL)) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*valid = 0;
	*num = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_PORT_INIT_E, 0, US_XML_DEBUG_PORT_E, -1);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_DEBUG_PORT_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		get_debug_port_attrs(xmlElement, num, valid);

#ifdef US_DEBUG_PRINT
		printk("DEBUG port = %d valid = %d\n", *num, *valid);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_fc_def_params
*
* DESCRIPTION:      Get FC default parameters from XML configuration file
*
* INPUTS:
*
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_fc_def_params(tpm_init_fc_params_t *port_fc_conf)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;
	uint32_t u32_value;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_FC_PARAMS_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_FC_PARAMS_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc |= get_int_param(xmlElement, US_XML_ENABLED_E, &u32_value);
		port_fc_conf->enabled = (uint8_t)u32_value;
		rc |= get_int_param(xmlElement, US_XML_PORT_E, &u32_value);
		port_fc_conf->port = (uint8_t)u32_value;
       	rc |= get_int_param(xmlElement, US_XML_TX_PORT_E, &u32_value);
		port_fc_conf->tx_port = (uint8_t)u32_value;
		rc |= get_int_param(xmlElement, US_XML_TX_Q_E, &u32_value);
		port_fc_conf->tx_queue = (uint8_t)u32_value;

		rc |= get_int_param(xmlElement, US_XML_Q_SAMPLE_FREQ_E, &port_fc_conf->queue_sample_freq);
		rc |= get_int_param(xmlElement, US_XML_THRESH_HIGH_E,	&port_fc_conf->thresh_high);
		rc |= get_int_param(xmlElement, US_XML_THRESH_LOW_E, 	&port_fc_conf->thresh_low);
		rc |= get_int_param(xmlElement, US_XML_TGT_PORT_E, 	&port_fc_conf->tgt_port);

#ifdef US_DEBUG_PRINT
		printk("SW port FC %s\n", (port_fc_conf->enabled) ? "Enabled" : "Disabled");
		if (port_fc_conf->enabled) {
       		printk(" port %d\n", port_fc_conf->port);
			printk(" tgt_port %d\n", port_fc_conf->tgt_port);
			printk(" tx_port %d\n", port_fc_conf->tx_port);
			printk(" tx_queue %d\n", port_fc_conf->tx_queue);
			printk(" queue_sample_freq %d\n", port_fc_conf->queue_sample_freq);
			printk(" thresh_high %d\n", port_fc_conf->thresh_high);
			printk(" thresh_low %d\n", port_fc_conf->thresh_low);
		}
#endif
	}

	return rc;
}

/*******************************************************************************
* get_igmp_snoop_params_enable()
*
* DESCRIPTION:      Get IGMP snooping parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  enable - whether the IGMP snooping is enabled
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_igmp_snoop_params_enable(uint32_t *enable)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (enable == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*enable = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_IGMP_SNOOP_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_IGMP_SNOOP_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_ENABLED_E, enable);

#ifdef US_DEBUG_PRINT
		printk("IGMP snooping: enable %d \n", *enable);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_igmp_snoop_params_cpu_queue()
*
* DESCRIPTION:      Get IGMP snooping parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  enable - whether the IGMP snooping is enabled
*           que    - queue number
*           all    - whether all packets should be verified
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_igmp_snoop_params_cpu_queue(uint32_t *que)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (que == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*que = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_IGMP_SNOOP_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_IGMP_SNOOP_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_IGMP_CPU_Q_E, que);

#ifdef US_DEBUG_PRINT
		printk("IGMP snooping:  que %d\n", *que);
#endif
	}

 /*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_igmp_snoop_params_port_frwd_mode()
*
* DESCRIPTION:      Get IGMP snooping parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  enable - whether the IGMP snooping is enabled
*           que    - queue number
*           all    - whether all packets should be verified
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_igmp_snoop_params_port_frwd_mode(uint32_t *mode, tpm_src_port_type_t port)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;
	char *xml_param_str;

	if (mode == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	switch (port) {
	case TPM_SRC_PORT_WAN:
		xml_param_str = US_XML_IGMP_PKT_FRWD_MODE_WAN;
		break;
	case TPM_SRC_PORT_UNI_0:
		xml_param_str = US_XML_IGMP_PKT_FRWD_MODE_UNI_0;
		break;
	case TPM_SRC_PORT_UNI_1:
		xml_param_str = US_XML_IGMP_PKT_FRWD_MODE_UNI_1;
		break;
	case TPM_SRC_PORT_UNI_2:
		xml_param_str = US_XML_IGMP_PKT_FRWD_MODE_UNI_2;
		break;
	case TPM_SRC_PORT_UNI_3:
		xml_param_str = US_XML_IGMP_PKT_FRWD_MODE_UNI_3;
		break;
	case TPM_SRC_PORT_UNI_4:
		xml_param_str = US_XML_IGMP_PKT_FRWD_MODE_UNI_4;
		break;
	case TPM_SRC_PORT_UNI_5:
		xml_param_str = US_XML_IGMP_PKT_FRWD_MODE_UNI_5;
		break;
	case TPM_SRC_PORT_UNI_6:
		xml_param_str = US_XML_IGMP_PKT_FRWD_MODE_UNI_6;
		break;
	case TPM_SRC_PORT_UNI_7:
		xml_param_str = US_XML_IGMP_PKT_FRWD_MODE_UNI_7;
		break;
	case TPM_SRC_PORT_UNI_VIRT:
		xml_param_str = US_XML_IGMP_PKT_FRWD_MODE_UNI_VIRT;
		break;
	case TPM_SRC_PORT_UNI_ANY:
		return US_RC_NOT_FOUND;

	default:
#ifdef US_DEBUG_PRINT
			printk(KERN_ERR "illegal input SRC port: %d\n", port);
#endif
			return US_RC_FAIL;
	}

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_IGMP_SNOOP_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_IGMP_SNOOP_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {

		rc = get_int_param(xmlElement, xml_param_str, mode);

#ifdef US_DEBUG_PRINT
		printk("igmp_mode port(%d), mode(%d) \n", port, *mode);
#endif
	}

 /*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_gmac_mh_en_params()
*
* DESCRIPTION:      Get GMAC Marvell Header enable parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  gmac0_mh_en  - GMAC0 Marvell Header Enable configuration
*           gmac1_mh_en  - GMAC0 Marvell Header Enable configuration
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_gmac_mh_en_params(uint32_t *gmac0_mh_en, uint32_t *gmac1_mh_en)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if ((gmac0_mh_en == NULL) || (gmac1_mh_en == NULL)) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*gmac0_mh_en = 0;
	*gmac1_mh_en = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_PORT_INIT_E, 0, US_XML_GMAC_CONFIG_E, 0, US_XML_GMAC_MH_ENA_E, -1);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_GMAC_MH_ENA_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_GMAC_0_MH_ENA_E, gmac0_mh_en);
		rc |= get_int_param(xmlElement, US_XML_GMAC_1_MH_ENA_E, gmac1_mh_en);
#ifdef US_DEBUG_PRINT
		printk("======================================\n");
		printk("        GMAC Marvell Header Enable    \n");
		printk("======================================\n");
		printk("gmac0_mh_en %d \n", *gmac0_mh_en);
		printk("gmac1_mh_en %d \n", *gmac1_mh_en);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_gmac_conn_params()
*
* DESCRIPTION:      Get GMAC connection parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  num_tcont_llid  - TCONT/LLID number
*           gmac0_con       - GMAC0 connection type
*           gmac1_con       - GMAC1 connection type
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_gmac_conn_params(uint32_t *num_tcont_llid)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (num_tcont_llid == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*num_tcont_llid = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_PORT_INIT_E, 0, US_XML_GMAC_CONFIG_E, 0, US_XML_GMAC_CONN_E, -1);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_GMAC_CONN_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_TCONT_LLID_E, num_tcont_llid);
#ifdef US_DEBUG_PRINT
		printk("======================================\n");
		printk("        GMAC connection               \n");
		printk("======================================\n");
		printk("num_tcont_llid %d \n", *num_tcont_llid);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_gmac_pool_bufs_params()
*
* DESCRIPTION:      Get GMAC Buffer Pool parameters from XML configuration file
*
* INPUTS:   max_gmacs_num    - MAX GMACs in the system
*
* OUTPUTS:  gmac_bufs       - pointer to the gmac_buffer pool data
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_gmac_pool_bufs_params(tpm_init_gmac_bufs_t *gmac_bufs, int max_gmacs_num)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	ezxml_t xmlGmac;
	const char *xmlStr;
	int rc = US_RC_OK;
	uint32_t gmac;
	uint32_t large_pkt_pool_bufs;
	uint32_t small_pkt_pool_bufs;

	if (gmac_bufs == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	if (max_gmacs_num == 0) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: Invalid max_gmacs_num \n", __func__);
#endif
		return US_RC_FAIL;
	}

	memset(gmac_bufs, 0, sizeof(tpm_init_gmac_bufs_t) * max_gmacs_num);

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_PORT_INIT_E, 0, US_XML_GMAC_CONFIG_E, 0, US_XML_GMAC_BM_BUF_E, -1);
	if (xmlElement == NULL)
		rc = US_RC_FAIL;
	else {
		xmlGmac = ezxml_child(xmlElement, US_XML_GMAC_E);
		if (xmlGmac) {
			for (; xmlGmac; xmlGmac = xmlGmac->next) {
				xmlStr = ezxml_attr(xmlGmac, US_XML_ID_ATTR);
				if (xmlStr == NULL) {
					printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_ID_ATTR);
					rc = US_RC_FAIL;
					break;
				}
				gmac = get_dig_number(xmlStr);
				if (gmac > (max_gmacs_num - 1)) {
					printk(KERN_ERR "%s: Invalid GMAC id number - %s\n", __func__, xmlStr);
					rc = US_RC_FAIL;
					break;
				}

				xmlStr = ezxml_attr(xmlGmac, US_XML_LARGE_POOL_BUF_ATTR);
				if (xmlStr == NULL) {
					printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_LARGE_POOL_BUF_ATTR);
					rc = US_RC_FAIL;
					break;
				}
				large_pkt_pool_bufs = get_dig_number(xmlStr);

				xmlStr = ezxml_attr(xmlGmac, US_XML_SHORT_POOL_BUF_ATTR);
				if (xmlStr == NULL) {
					printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_SHORT_POOL_BUF_ATTR);
					rc = US_RC_FAIL;
					break;
				}
				small_pkt_pool_bufs = get_dig_number(xmlStr);

				gmac_bufs[gmac].valid = 1;
				gmac_bufs[gmac].large_pkt_buffers = large_pkt_pool_bufs;
				gmac_bufs[gmac].small_pkt_buffers = small_pkt_pool_bufs;
			}
		}
#ifdef US_DEBUG_PRINT
		printk("======================================\n");
		printk("               GMAC BUFS              \n");
		printk("======================================\n");
		for (gmac = 0; gmac < max_gmacs_num; gmac++) {
			if (gmac_bufs[gmac].valid) {
				printk("gmac %d  large_bufs %d  small_bufs %d\n",
				       gmac, gmac_bufs[gmac].large_pkt_buffers, gmac_bufs[gmac].small_pkt_buffers);
			}
		}
		printk("======================================\n");
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}


/*******************************************************************************
* get_eth_cmplx_profile_params()
*
* DESCRIPTION:      Get GMAC connection parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  num_tcont_llid  - TCONT/LLID number
*           gmac0_con       - GMAC0 connection type
*           gmac1_con       - GMAC1 connection type
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_eth_cmplx_profile_params(tpm_eth_complex_profile_t *eth_cmplx_profile)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (eth_cmplx_profile == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*eth_cmplx_profile = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_ETH_CMPLX_CONFIG_E, -1);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_ETH_CMPLX_CONFIG_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_ETH_PROFILE_E, eth_cmplx_profile);
#ifdef US_DEBUG_PRINT
		printk("======================================\n");
		printk("            ETH COMPLEX               \n");
		printk("======================================\n");
		printk("eth_cmplx_profile %d \n", *eth_cmplx_profile);
#endif
	}

	return rc;
}

/*******************************************************************************
* get_gmac_port_conf_params()
*
* DESCRIPTION:      Get GMAC Buffer Pool parameters from XML configuration file
*
* INPUTS:   max_gmacs_num    - MAX GMACs in the system
*
* OUTPUTS:  gmac_bufs       - pointer to the gmac_buffer pool data
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_gmac_port_conf_params(tpm_init_gmac_conn_conf_t *gmac_port_conf, int max_gmac_port_num)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	ezxml_t xmlPort;
	const char *xmlStr;
	int rc = US_RC_OK;
	uint32_t port_src;
	uint32_t port_nr;

	if ((gmac_port_conf == NULL) || (max_gmac_port_num == 0)) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: gmac_port_conf=%p max_gmac_port_num=%d\n",
		       __func__, gmac_port_conf, max_gmac_port_num);
#endif
		return US_RC_FAIL;
	}

	memset(gmac_port_conf, 0, sizeof(tpm_init_gmac_conn_conf_t) * max_gmac_port_num);

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);
	if (xmlHead == NULL)
		return US_RC_FAIL;


	xmlElement = ezxml_get(xmlHead, US_XML_ETH_CMPLX_CONFIG_E, 0, US_XML_GMAC_PORTS_E, -1);
	if (xmlElement == NULL)
		return US_RC_NOT_FOUND;

	xmlPort = ezxml_child(xmlElement, US_XML_PORT_E);
	if (xmlPort == NULL)
		return US_RC_FAIL;

	for (; xmlPort; xmlPort = xmlPort->next) {
		/* mandatory field */
		xmlStr = ezxml_attr(xmlPort, US_XML_ID_ATTR);
		if (xmlStr == NULL) {
			printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_ID_ATTR);
			rc = US_RC_FAIL;
			break;
		}
		port_nr = get_dig_number(xmlStr);
		if (port_nr > (max_gmac_port_num - 1)) {
			printk(KERN_ERR "%s: Invalid PORT id number - %s\n", __func__, xmlStr);
			rc = US_RC_FAIL;
			break;
		}

		/* optional field */
		port_src = TPM_SRC_PORT_ILLEGAL;
		xmlStr = ezxml_attr(xmlPort, US_XML_ETH_PORT_SRC_ATTR);
		if (xmlStr != NULL)
			port_src = get_dig_number(xmlStr);

		gmac_port_conf[port_nr].valid = 1;
		gmac_port_conf[port_nr].port_src = port_src;
	}

#ifdef US_DEBUG_PRINT
	 printk("======================================\n");
	 printk("              GMAC PORTS              \n");
	 printk("======================================\n");
	 for (port_nr = 0; port_nr < max_gmac_port_num; port_nr++) {
	 	if (gmac_port_conf[port_nr].valid) {
	 		printk("port %d  port_src %d\n",
	 		       port_nr, gmac_port_conf[port_nr].port_src);
	 	}
	 }
	 printk("======================================\n");
#endif

	return rc;
}


/*******************************************************************************
* get_eth_port_conf_params()
*
* DESCRIPTION:      Get GMAC Buffer Pool parameters from XML configuration file
*
* INPUTS:   max_gmacs_num    - MAX GMACs in the system
*
* OUTPUTS:  gmac_bufs       - pointer to the gmac_buffer pool data
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_eth_port_conf_params(tpm_init_eth_port_conf_t *eth_port_bufs, int max_eth_port_num)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	ezxml_t xmlPort;
	const char *xmlStr;
	int rc = US_RC_OK;
	uint32_t switch_port;
	uint32_t port_src;
	uint32_t port_nr;

	if ((eth_port_bufs == NULL) || (max_eth_port_num == 0)) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: eth_port_bufs=%p max_eth_port_num=%d\n", __func__, eth_port_bufs, max_eth_port_num);
#endif
		return US_RC_FAIL;
	}

	memset(eth_port_bufs, 0, sizeof(tpm_init_eth_port_conf_t) * max_eth_port_num);

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);
	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_ETH_CMPLX_CONFIG_E, 0, US_XML_ETH_SW_PORT_ATTR, -1);
	if (xmlElement == NULL)
		return US_RC_NOT_FOUND;

	xmlPort = ezxml_child(xmlElement, US_XML_PORT_E);
	if (xmlPort == NULL)
		return US_RC_FAIL;

	for (port_nr = 0; xmlPort && (port_nr < max_eth_port_num); xmlPort = xmlPort->next, port_nr++) {
		/* mandatory field */
		xmlStr = ezxml_attr(xmlPort, US_XML_ID_ATTR);
		if (xmlStr == NULL) {
			printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_ID_ATTR);
			rc = US_RC_FAIL;
			break;
		}
		switch_port = get_dig_number(xmlStr);
		if (switch_port > 6) {
			printk(KERN_ERR "%s: Invalid PORT id number - %s\n", __func__, xmlStr);
			rc = US_RC_FAIL;
			break;
		}
		
		/* mandatory field */
		xmlStr = ezxml_attr(xmlPort, US_XML_ETH_PORT_SRC_ATTR);
		if (xmlStr == NULL) {
			printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_ETH_PORT_SRC_ATTR);
			rc = US_RC_FAIL;
			break;
		}
		port_src = get_dig_number(xmlStr);

		eth_port_bufs[port_nr].valid = 1;
		eth_port_bufs[port_nr].switch_port = switch_port;
		eth_port_bufs[port_nr].port_src = port_src;
       }
#ifdef US_DEBUG_PRINT
	printk("======================================\n");
	printk("               ETH PORTS              \n");
	printk("======================================\n");
	for (port_nr = 0; port_nr < max_eth_port_num; port_nr++) {
		if (eth_port_bufs[port_nr].valid) {
			printk("%d)  src_port %d  switch_port %d\n",
			       port_nr,
			       eth_port_bufs[port_nr].port_src, 
			       eth_port_bufs[port_nr].switch_port);
		}
	}
	printk("======================================\n");
#endif

	return rc;
}

/*******************************************************************************
* get_backup_wan_params()
*
* DESCRIPTION:      Get GMAC connection parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  num_tcont_llid  - TCONT/LLID number
*           gmac0_con       - GMAC0 connection type
*           gmac1_con       - GMAC1 connection type
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_backup_wan_params(uint32_t *backup_wan)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (backup_wan == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*backup_wan = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_ETH_CMPLX_CONFIG_E, -1);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_ETH_CMPLX_CONFIG_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_BACKUP_WAN_E, backup_wan);
#ifdef US_DEBUG_PRINT
		printk("======================================\n");
		printk("            ETH COMPLEX               \n");
		printk("======================================\n");
		printk("backup_wan %d \n", *backup_wan);
#endif
	}

	return rc;
}

/*******************************************************************************
* get_gmac_rxq_params()
*
* DESCRIPTION:
*
* INPUTS:   max_gmacs_num    - MAX GMACs in the system
*           max_rx_queues_num - MAX possible rx_queues per GMAC
*
* OUTPUTS:  gmac_rx             - pointer to the gmac rx_queues data
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_gmac_rxq_params(tpm_init_gmac_rx_t *gmac_rx, int max_gmacs_num, int max_rx_queues_num)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	const char *xmlStr;
	ezxml_t xmlGmac, xmlQue;
	int rc = US_RC_OK;
	uint32_t gmac;
	uint32_t stop_parsing = 0;

	if (gmac_rx == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	if (max_gmacs_num == 0) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: Invalid max_gmacs_num \n", __func__);
#endif
		return US_RC_FAIL;
	}
	if (max_rx_queues_num == 0) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: Invalid max_tx_queues_num \n", __func__);
#endif
		return US_RC_FAIL;
	}

	memset(gmac_rx, 0, sizeof(tpm_init_gmac_rx_t) * max_gmacs_num);

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_PORT_INIT_E, 0, US_XML_GMAC_CONFIG_E, 0, US_XML_GMAC_RXQUEUES_E, -1);
	if (xmlElement == NULL)
		rc = US_RC_FAIL;
	else {
		xmlGmac = ezxml_child(xmlElement, US_XML_GMAC_E);
		if (xmlGmac) {
			for (; (xmlGmac && !stop_parsing); xmlGmac = xmlGmac->next) {
				xmlStr = ezxml_attr(xmlGmac, US_XML_ID_ATTR);
				if (xmlStr == NULL) {
					printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_ID_ATTR);
					rc = US_RC_FAIL;
					break;
				}
				gmac = get_dig_number(xmlStr);
				if (gmac > (max_gmacs_num - 1)) {
					printk(KERN_ERR "%s: Invalid GMAC id number - %s\n", __func__, xmlStr);
					rc = US_RC_FAIL;
					break;
				}

				/* Get GMAC rx queue sizes */
				for (xmlQue = ezxml_child(xmlGmac, US_XML_QUEUE_E); xmlQue; xmlQue = xmlQue->next) {
					rc = get_rx_queue_attrs(xmlQue, gmac, gmac_rx, max_rx_queues_num);

					if (rc != US_RC_OK) {
						stop_parsing = 1;
						break;
					}
				}

			}
		}
#ifdef US_DEBUG_PRINT
		{

			uint32_t rxq;
			printk("======================================\n");
			printk("               GMAC RXQs              \n");
			printk("======================================\n");
			for (gmac = 0; gmac < max_gmacs_num; gmac++) {
				if (gmac_rx[gmac].valid) {
					for (rxq = 0; rxq < max_rx_queues_num; rxq++) {
						if (gmac_rx[gmac].rx_queue[rxq].valid) {
							printk("gmac %d rxq %d size %d\n",
							       gmac, rxq, gmac_rx[gmac].rx_queue[rxq].queue_size);
						}
					}
				}
			}
			printk("======================================\n");
		}
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_gmac_tx_params()
*
* DESCRIPTION:      Get GMAC TX parameters from XML configuration file
*
* INPUTS:   max_tx_ports_num    - MAX TX ports in the system
*           max_tx_queues_num   - MAX TX queues per port
*
* OUTPUTS:  gmac_tx             - pointer to the TX port data
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_gmac_tx_params(tpm_init_gmac_tx_t *gmac_tx, int max_tx_ports_num, int max_tx_queues_num)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	ezxml_t xmlGmac;
	ezxml_t xmlQueMap;
	ezxml_t xmlQue;
	int rc = US_RC_OK;
	uint32_t port;
	const char *xmlStr;
	uint8_t stop_parsing = 0;

	if (gmac_tx == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	if ((max_tx_ports_num == 0) || (max_tx_queues_num == 0)) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: Invalid max_int_ports_num \n", __func__);
#endif
		return US_RC_FAIL;
	}

	memset(gmac_tx, 0, sizeof(tpm_init_gmac_tx_t) * max_tx_ports_num);

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_PORT_INIT_E, 0, US_XML_TX_MOD_PARAMS_E, -1);
	if (xmlElement == NULL)
		rc = US_RC_FAIL;
	else {
		for (xmlGmac = ezxml_child(xmlElement, US_XML_TX_MOD_E);
		     xmlGmac && !stop_parsing; xmlGmac = xmlGmac->next) {
			xmlStr = ezxml_attr(xmlGmac, US_XML_ID_ATTR);
			if (xmlStr == NULL) {
				printk(KERN_ERR "%s: Failed to get %s\n", __func__, US_XML_ID_ATTR);
				rc = US_RC_FAIL;
				break;
			}

			port = get_dig_number(xmlStr);
			if (port > max_tx_ports_num) {
				printk(KERN_ERR "%s: Invalid GMAC id number - %s\n", __func__, xmlStr);
				rc = US_RC_FAIL;
				break;
			}

			xmlQueMap = ezxml_child(xmlGmac, US_XML_QUEUE_MAP_E);
			for (xmlQue = ezxml_child(xmlQueMap, US_XML_QUEUE_E); xmlQue; xmlQue = xmlQue->next) {
				rc = get_tx_queue_attrs(xmlQue, port, gmac_tx, max_tx_queues_num);

				if (rc != US_RC_OK) {
					stop_parsing = 1;
					break;
				}
			}
		}

#ifdef US_DEBUG_PRINT
		{
			int que;

			printk("======================================\n");
			printk("               GMAC TX                \n");
			printk("======================================\n");
			for (port = 0; port < max_tx_ports_num; port++) {
				if (gmac_tx[port].valid) {
					printk("TX port %d  \n", port);
					printk("------------\n");

					for (que = 0; que < max_tx_queues_num; que++) {
						if (gmac_tx[port].tx_queue[que].valid) {
							printk(KERN_INFO
								"TX queue %d: valid %d owner %d owner num %d "
								"size %d weight %d sched %d\n",
								que, gmac_tx[port].tx_queue[que].valid,
								gmac_tx[port].tx_queue[que].queue_owner,
								gmac_tx[port].tx_queue[que].owner_queue_num,
								gmac_tx[port].tx_queue[que].queue_size,
								gmac_tx[port].tx_queue[que].queue_weight,
								gmac_tx[port].tx_queue[que].sched_method);
						}
					}
					/*} */
				}
				printk("======================================\n");
			}
		}
#endif
	}
	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_pnc_range_params()
*
* DESCRIPTION:      Get PnC ranges parameters from XML configuration file
*
* INPUTS:   max_pnc_ranges_num  - MAX PnC ranges in the system
*
* OUTPUTS:  pnc_range           - pointer to the internal PnC range table
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_pnc_range_params(tpm_init_pnc_range_t *pnc_range, int max_pnc_ranges_num)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	ezxml_t xmlRange;
	int rc = US_RC_OK;

	if (pnc_range == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	if (max_pnc_ranges_num == 0) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: Invalid max_pnc_ranges_num \n", __func__);
#endif
		return US_RC_FAIL;
	}

	memset(pnc_range, 0, sizeof(tpm_init_pnc_range_t) * max_pnc_ranges_num);

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_TPM_E, 0, US_XML_TPM_PNC_E, 0, US_XML_TPM_PNC_RANGE_PARAMS_E, -1);
	if (xmlElement == NULL)
		rc = US_RC_FAIL;
	else {
		for (xmlRange = ezxml_child(xmlElement, US_XML_TPM_PNC_RANGE_E); xmlRange; xmlRange = xmlRange->next) {
			rc = get_pnc_range_attrs(xmlRange, pnc_range, max_pnc_ranges_num);

			if (rc != US_RC_OK)
				break;
		}

#ifdef US_DEBUG_PRINT
		{
			int range;

			printk("======================================\n");
			printk("              PnC range               \n");
			printk("======================================\n");
			for (range = 0; range < max_pnc_ranges_num; range++) {
				if (pnc_range[range].valid)
					printk("%d: num %d type %d size %d cntr_grp %d "
						"lu_mask %d min_res_level %d\n",
						range,
						pnc_range[range].range_num,
						pnc_range[range].range_type,
						pnc_range[range].range_size,
						pnc_range[range].cntr_grp,
						pnc_range[range].lu_mask, pnc_range[range].min_reset_level);
			}
			printk("======================================\n");
		}
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_ds_mh_config_params()
*
* DESCRIPTION:      Get downstream MH parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  ds_mh_set - from PnC RI or Rx ctrl reg
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_ds_mh_config_params(tpm_init_mh_src_t *dsMhConf)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;
	uint32_t val;

	if (dsMhConf == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*dsMhConf = TPM_MH_SRC_RX_CTRL;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_TPM_E);
	if (xmlElement == NULL) {
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_TPM_E, g_pstr_xml_cfg_file);
		rc = US_RC_NOT_FOUND;
	} else
		rc = get_int_param(xmlElement, US_XML_DS_MH_SET_E, &val);

	if (rc == US_RC_OK) {
		*dsMhConf = (tpm_init_mh_src_t) val;

#ifdef US_DEBUG_PRINT
		printk("dsMhConf = %d\n", *dsMhConf);
#endif
	}
	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_validation_enabled_config_params()
*
* DESCRIPTION:      Get validation enable parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  validation_enabled - enable or not TPM validation
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_validation_enabled_config_params(tpm_init_tpm_validation_t *validEn)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc;
	uint32_t val;

	if (validEn == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*validEn = TPM_VALID_DISABLED;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_TPM_E);
	if (xmlElement == NULL) {
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_TPM_E, g_pstr_xml_cfg_file);
		rc = US_RC_NOT_FOUND;
	} else
		rc = get_int_param(xmlElement, US_XML_VALID_EN_E, &val);

	if (rc == US_RC_OK) {
		*validEn = (tpm_init_tpm_validation_t) val;

#ifdef US_DEBUG_PRINT
		printk("validEn = %d\n", *validEn);
#endif
	}
	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_epon_connect_tpm_params()
*
* DESCRIPTION:      Get EPON connect TPM APIS parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  connected - disconnect or connected.
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_epon_connect_tpm_params(PON_connect_type_t *connected)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc;
	uint32_t val;

	if (connected == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*connected = ONU_PON_CONNECT;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_EPON_E);
	if (xmlElement == NULL) {
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_EPON_E, g_pstr_xml_cfg_file);
		rc = US_RC_NOT_FOUND;
	} else
		rc = get_int_param(xmlElement, US_XML_EPON_CONNECT_E, &val);

	if (rc == US_RC_OK) {
		*connected = (PON_connect_type_t) val;

#ifdef US_DEBUG_PRINT
		printk("connected = %d\n", *connected);
#endif
	}
	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_epon_oam_rx_q_params()
*
* DESCRIPTION:      Get EPON CPU OAM RX Q parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  OAM CPU RX Queue value
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_epon_oam_rx_q_params(uint32_t *oam_cpu_rx_queue)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc;
	uint32_t val;

	if (oam_cpu_rx_queue == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	/* default queue is 7 - highest queue */
	*oam_cpu_rx_queue = 7;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_EPON_E);
	if (xmlElement == NULL) {
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_EPON_E, g_pstr_xml_cfg_file);
		rc = US_RC_NOT_FOUND;
	} else
		rc = get_int_param(xmlElement, US_XML_EPON_OAM_RXQ_E, &val);

	if (rc == US_RC_OK) {
		*oam_cpu_rx_queue = val;

#ifdef US_DEBUG_PRINT
		printk("oam_cpu_rx_queue = %d\n", *oam_cpu_rx_queue);
#endif
	}
	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_cfg_pnc_parse_param()
*
* DESCRIPTION:      Get mode of how to configure the PNC PARSE in LSP
*
* INPUTS:
*
* OUTPUTS:  cfgPncParse - enabled/disabled
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_cfg_pnc_parse_param(tpm_init_cfg_pnc_parse_t *cfgPncParse)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc;
	uint32_t val;

	if (cfgPncParse == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*cfgPncParse = TPM_NONE;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_TPM_E);
	if (xmlElement == NULL)
		rc = US_RC_NOT_FOUND;
	else
		rc = get_int_param(xmlElement, US_XML_CFG_PNC_PARSER, &val);

	if (rc == US_RC_OK) {
		*cfgPncParse = (tpm_init_cfg_pnc_parse_t) val;

#ifdef US_DEBUG_PRINT
		printk("cfgPncParse = %d\n", *cfgPncParse);
#endif
	}
	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_cpu_loopback_param()
*
* DESCRIPTION:      Get CPU egress loopback state
*
* INPUTS:
*
* OUTPUTS:  cfgCpuLoopback - enabled/disabled
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_cpu_loopback_param(tpm_init_cpu_loopback_t *cpuLoopback)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc;
	uint32_t val;

	if (cpuLoopback == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*cpuLoopback = TPM_NONE;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_TPM_E);
	if (xmlElement == NULL)
		rc = US_RC_NOT_FOUND;
	else
		rc = get_int_param(xmlElement, US_XML_CFG_CPU_LOOPBACK, &val);

	if (rc == US_RC_OK) {
		*cpuLoopback = (tpm_init_cpu_loopback_t) val;

#ifdef US_DEBUG_PRINT
		printk("cfgCpuLoopback = %d\n", *cpuLoopback);
#endif
	}
	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_trace_debug_info_param()
*
* DESCRIPTION:      Get mode of how to configure the PNC PARSE in LSP
*
* INPUTS:
*
* OUTPUTS:  cfgPncParse - enabled/disabled
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_trace_debug_info_param(uint32_t *trace_debug_info)
{

	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc;
	uint32_t val;

	if (trace_debug_info == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*trace_debug_info = 0x00000000;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_TPM_E);
	if (xmlElement == NULL)
		rc = US_RC_NOT_FOUND;
	else
		rc = get_int_param(xmlElement, US_XML_TRACE_DEBUG_info, &val);

	if (rc == US_RC_OK) {
		*trace_debug_info = val;

#ifdef US_DEBUG_PRINT
		printk("trace_debug_info = 0x%x\n", *trace_debug_info);
#endif
	}
	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_vitual_uni_enable_params()
*
* DESCRIPTION:      Get WiFi via GMAC1 feature parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  enable - whether the WiFi via GMAC1 is enabled
*           port   - port number for WiFi identification
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_vitual_uni_enable_params(uint32_t *enable)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;
	uint32_t val1;

	if (enable == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif

		printk(KERN_ERR "get_wifi_vitual_uni_enable_params => NULL pointer\n");
		return US_RC_FAIL;
	}

	*enable = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_TPM_E);
	if (xmlElement == NULL)
		rc = US_RC_NOT_FOUND;
	else {
		rc = get_int_param(xmlElement, US_XML_WIFI_ENABLED, &val1);
		/* add port parameter to the XML - in the future - meantime default port: UNI_4 = 5 */
		/*        rc |= get_int_param (xmlElement, US_XML_WIFI_PORT, &val2);*/
	}

	if (rc == US_RC_OK) {
		*enable = val1;

#ifdef US_DEBUG_PRINT
		printk("wifi_vir_uni_enable => en= %d\n", *enable);
#endif
	}
	/*ezxml_free(xmlHead); */
	return rc;

}

#if 0
/*******************************************************************************
* get_default_vlan_tpid_params()
*
* DESCRIPTION:      Get double tag support parameter from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  enable - whether the WiFi via GMAC1 is enabled
*           port   - port number for WiFi identification
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_default_vlan_tpid_params(uint16_t *v1_tpid, uint16_t *v2_tpid)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;
	uint32_t val1, val2;

	if ((v1_tpid == NULL) || (v2_tpid == NULL)) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif

		return US_RC_FAIL;
	}

	*v1_tpid = *v2_tpid = 0x8100;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_TPM_E);
	if (xmlElement == NULL)
		rc = US_RC_NOT_FOUND;
	else {
		rc |= get_int_param(xmlElement, US_XML_DEF_V1_TPID, &val1);
		rc |= get_int_param(xmlElement, US_XML_DEF_V2_TPID, &val2);
	}

	if (rc == US_RC_OK) {
		*v1_tpid = val1;
		*v2_tpid = val2;

#ifdef US_DEBUG_PRINT
		printk(KERN_INFO"get_double_tag_support_params => v1_tpid= 0x%x  v2_tpid= 0x%x\n", *v1_tpid, *v2_tpid);
#endif
	}
	/*ezxml_free(xmlHead); */
	return rc;

}
#endif

/*******************************************************************************
* get_default_vlan_tpid_params()
*
* DESCRIPTION:      Get double tag support parameter from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  enable - whether the WiFi via GMAC1 is enabled
*           port   - port number for WiFi identification
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_default_vlan_tpid_params(uint32_t *opt_num, tpm_init_tpid_comb_t *opt)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	ezxml_t xmlTpid;
	int optNum;
	int rc;

	if ((opt_num == NULL) || (opt == NULL)) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	optNum = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);
	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_TPM_E, 0, US_XML_TPM_VLAN_FILTER_TPID_E, -1);
	if (xmlElement == NULL) {
		printk("Failed to find %s in XML config. file %s\n", US_XML_TPM_VLAN_FILTER_TPID_E, g_pstr_xml_cfg_file);
		rc = US_RC_NOT_FOUND;
	} else {
		for (xmlTpid = ezxml_child(xmlElement, US_XML_TPM_FILTER_TPID_E); \
			xmlTpid && (optNum < TPM_MAX_TPID_COMB_NUM); \
			xmlTpid = xmlTpid->next, optNum++) {
			rc = get_filter_tpid_attrs(xmlTpid, &(opt[optNum]));
			if (rc != US_RC_OK)
			break;
		}
		*opt_num = optNum;
	}

	return rc;
}

/*******************************************************************************
* get_pon_def_params()
*
* DESCRIPTION: Get PON default parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  param[PON_XML_DEF_PAR_SERIAL_NUM]		- serial number
*           param[PON_XML_DEF_PAR_PASSWORD]			- password
*           param[PON_XML_DEF_PAR_DIS_SERIAL_NUM]	- whether SN should be disabled
*           param[PON_XML_DEF_PARAM_CLEAR_GEM]		- whether gem ports should be removed on reset
*           param[PON_XML_DEF_PARAM_CLEAR_TCONT]	- whether tconts should be removed on reset
*   		param[PON_XML_DEF_PARAM_SERIAL_NUM_SRC]	- whether serial number should be taken from xml
*   													or should the digit part be taken from MAC address
*   		param[PON_XML_DEF_PARAM_XVR_POLARITY]	- tranceiver polarity
*   		param[PON_XML_DEF_PARAM_DG_POLARITY]	- dying gasp polarity
*   		param[PON_XML_DEF_PARAM_RESTORE_GEM]	- whether restore or not GEM ports after returning
*   													from state-7 (EMG STOP)
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_pon_def_params (void  **param)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc;
	int	paramIdx;
	unsigned char   *sn = (unsigned char *)param[PON_XML_DEF_PAR_SERIAL_NUM];
	unsigned char   *pswd = (unsigned char *)param[PON_XML_DEF_PAR_PASSWORD];

	if (param == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: parameters list is a NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	for (paramIdx = PON_XML_DEF_PAR_SERIAL_NUM; paramIdx < PON_XML_DEF_PARAM_MAX; paramIdx++) {
		if (param[paramIdx] == NULL) {
#ifdef US_DEBUG_PRINT
			printk(KERN_ERR "%s: parameter #%d is a NULL pointer\n", __func__, paramIdx);
#endif
			return US_RC_FAIL;
		}
	}

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_PON_E);
	if (xmlElement == NULL) {
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_PON_E, g_pstr_xml_cfg_file);
		rc = US_RC_NOT_FOUND;

	} else {

		sn[0] = '\0';
		rc = get_char_param(xmlElement, US_XML_PON_SN_E, sn, US_XML_PON_SN_LEN);
		pswd[0] = '\0';
		rc |= get_char_param(xmlElement, US_XML_PON_PASSWD_E, pswd, US_XML_PON_PASSWD_LEN);
		*(uint32_t *)param[PON_XML_DEF_PAR_DIS_SERIAL_NUM] = 0;
		rc |= get_int_param(xmlElement, US_XML_PON_DIS_SN_E, (uint32_t *)param[PON_XML_DEF_PAR_DIS_SERIAL_NUM]);
		*(uint32_t *)param[PON_XML_DEF_PARAM_CLEAR_GEM] = 0;
		rc |= get_int_param (xmlElement, US_XML_PON_GEM_RST_E, (uint32_t *)param[PON_XML_DEF_PARAM_CLEAR_GEM]);
		*(uint32_t *)param[PON_XML_DEF_PARAM_CLEAR_TCONT] = 0;
		rc |= get_int_param(xmlElement, US_XML_PON_TCONT_RST_E, (uint32_t *)param[PON_XML_DEF_PARAM_CLEAR_TCONT]);
		*(uint32_t *)param[PON_XML_DEF_PARAM_SERIAL_NUM_SRC] = 0;
		rc |= get_int_param(xmlElement, US_XML_PON_SN_SRC_E, (uint32_t *)param[PON_XML_DEF_PARAM_SERIAL_NUM_SRC]);
		*(uint32_t *)param[PON_XML_DEF_PARAM_XVR_POLARITY] = 0;
		rc |= get_int_param(xmlElement, US_XML_PON_XVR_POL_E, (uint32_t *)param[PON_XML_DEF_PARAM_XVR_POLARITY]);
		*(uint32_t *)param[PON_XML_DEF_PARAM_DG_POLARITY] = 1;
		rc |= get_int_param(xmlElement, US_XML_PON_DG_POL_E, (uint32_t *)param[PON_XML_DEF_PARAM_DG_POLARITY]);
		*(uint32_t *)param[PON_XML_DEF_PARAM_RESTORE_GEM] = 1;
		rc |= get_int_param(xmlElement, US_XML_PON_GEM_RESTOR_E, (uint32_t *)param[PON_XML_DEF_PARAM_RESTORE_GEM]);

#ifdef US_DEBUG_PRINT
		printk(KERN_DEBUG "PON default params: SN %s pswd %s dis %d\n", sn, pswd,
			   *(uint32_t *)param[PON_XML_DEF_PAR_DIS_SERIAL_NUM]);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_epon_def_params()
*
* DESCRIPTION: Get EPON default parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  dba_mode      - DBA mode is either 0 for SW or 1 for HW
*           xvr_pol       - XVR polarity (0 - high, 1 - low)
*           dg_pol        - Dying Gasp polarity (1 - high, 0 - low)
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_epon_def_params(unsigned long *xvr_pol, unsigned long *dg_pol)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_EPON_E);
	if (xmlElement == NULL) {
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_EPON_E, g_pstr_xml_cfg_file);
		rc = US_RC_NOT_FOUND;
	} else {
		*xvr_pol = 0;
		rc = get_int_param(xmlElement, US_XML_EPON_XVR_POL_E, (uint32_t *) xvr_pol);
		*dg_pol = 1;
		rc |= get_int_param(xmlElement, US_XML_EPON_DG_POL_E, (uint32_t *) dg_pol);
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_pon_dg_polarity()
*
* DESCRIPTION: Get PON Dying Gasp polarity value from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  dg_pol - Daing Gasp polarity is either 0 for high or 1 for low
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_pon_dg_polarity(unsigned long *dg_pol)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_PON_E);
	if (xmlElement == NULL) {
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_PON_E, g_pstr_xml_cfg_file);
		rc = US_RC_NOT_FOUND;
	} else {
		*dg_pol = 1;
		rc = get_int_param(xmlElement, US_XML_PON_DG_POL_E, (uint32_t *) dg_pol);
		if (rc == US_RC_NOT_FOUND) {
			printk(KERN_ERR "Failed to find %s tag in XML config. file \n", US_XML_PON_DG_POL_E);
			*dg_pol = 1;
			rc = US_RC_OK;
		}
	}

	/*ezxml_free(xmlHead); */
	return rc;

}

/*******************************************************************************
* get_vlan_etypes_params()
*
* DESCRIPTION:      Get VLAN ETY parameters from XML configuration file
*
* INPUTS:   vlan_ety_reg_num    - Number of VLAN ETY registers in the system
*
* OUTPUTS:  mod_tpid            - pointer to the VLANs ETY table
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_vlan_mod_vlan_tpid_params(uint32_t *mod_vlan_tpid, int mod_vlan_tpid_reg_num)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	ezxml_t xmlEty;
	int rc = US_RC_OK, i;

	if (mod_vlan_tpid == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	if (mod_vlan_tpid_reg_num == 0) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: Invalid vlan_ety_reg_num \n", __func__);
#endif
		return US_RC_FAIL;
	}

	for (i = 0; i < mod_vlan_tpid_reg_num; i++)
		mod_vlan_tpid[i] = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_TPM_E, 0, US_XML_TPM_MOD_VLAN_TPID_E, -1);
	if (xmlElement == NULL)
		rc = US_RC_NOT_FOUND;
	else {
		for (xmlEty = ezxml_child(xmlElement, US_XML_TPM_MOD_TPID_E); xmlEty; xmlEty = xmlEty->next) {
			rc = get_vlan_ety_attrs(xmlEty, mod_vlan_tpid, mod_vlan_tpid_reg_num);

			if (rc != US_RC_OK)
				break;
		}

#ifdef US_DEBUG_PRINT
		{
			int reg;

			printk("============================\n");
			printk("  Reg#   VLAN etype   \n");
			printk("============================\n");
			for (reg = 0; reg < mod_vlan_tpid_reg_num; reg++)
				printk("   %d        0x%x\n", reg, mod_vlan_tpid[reg]);

			printk("============================\n");
		}
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_modification_params()
*
* DESCRIPTION: Get TPM modification parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  config  - configuration parameters of Modification table
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_modification_params(tpm_init_mod_params_t *config)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (config == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	config->udp_checksum_use_init_bm = 0;
	config->udp_checksum_update = 0;

	xmlElement = ezxml_get(xmlHead, US_XML_TPM_E, 0, US_XML_TPM_MOD_E, -1);
	if (xmlElement == NULL) {
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_TPM_MOD_E, g_pstr_xml_cfg_file);
		rc = US_RC_NOT_FOUND;
	} else {
		rc |= get_int_param(xmlElement, US_XML_TPM_MOD_UDP_CKS_USE_INIT_E, &config->udp_checksum_use_init_bm);
		rc |= get_int_param(xmlElement, US_XML_TPM_MOD_UDP_CKS_CALC_E, &config->udp_checksum_update);

	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_chain_config_params()
*
* DESCRIPTION:      Get num_vlan_tags parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  num_vlan_tags - vlan nmuber
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_chain_config_params(tpm_init_mod_chain_t *mod_chain)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	ezxml_t xmlChain;
	int chainNum;
	int rc;

	if (mod_chain == NULL)
		return US_RC_FAIL;

	memset(mod_chain, 0, sizeof(tpm_init_mod_chain_t));
	chainNum = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_TPM_E, 0, US_XML_TPM_MOD_E, 0, US_XML_TPM_MOD_CHAIN_PARAM_E, -1);
	if (xmlElement == NULL) {
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_TPM_MOD_CHAIN_PARAM_E, g_pstr_xml_cfg_file);
		rc = US_RC_NOT_FOUND;
	} else {
		for (xmlChain = ezxml_child(xmlElement, US_XML_TPM_MOD_CHAIN_E);
		     xmlChain && (chainNum < TPM_MAX_MOD_CHAIN_NUM); xmlChain = xmlChain->next, chainNum++) {
			rc = get_mod_chain_attrs(xmlChain, &(mod_chain->chain_data[chainNum]));

			if (rc != US_RC_OK)
				break;
		}

		mod_chain->chain_num = chainNum;
	}

	return rc;
}

/* jinghua add for MTU setting */
/*******************************************************************************
* get_mtu_enable()
*
* DESCRIPTION:      Get mtu enable parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  enable - mtu enable
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_mtu_enable(tpm_init_mtu_setting_enable_t *enable)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;
	uint32_t val;

	if (enable == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*enable = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_MTU_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_MTU_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_MTU_SETTING_ENABLE_E, &val);
		if (rc == US_RC_OK) {
			*enable = (tpm_init_mtu_setting_enable_t) val;

#ifdef US_DEBUG_PRINT
			printk("US_XML_MTU_SETTING_ENABLE_E: enable %d \n", *enable);
#endif
		}

	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_ipv4_mtu_us()
*
* DESCRIPTION:      Get ipv4 mtu parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  mtu - ipv4 mtu
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_ipv4_mtu_us(uint32_t *mtu)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (mtu == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*mtu = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_MTU_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_MTU_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_IPV4_MTU_US_E, mtu);

#ifdef US_DEBUG_PRINT
		printk("IPV4_MTU: mtu %d \n", *mtu);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_ipv4_pppoe_mtu_us()
*
* DESCRIPTION:      Get ipv4 pppoe mtu parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  mtu - ipv4 pppoe mtu
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_ipv4_pppoe_mtu_us(uint32_t *mtu)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (mtu == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*mtu = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_MTU_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_MTU_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_IPV4_PPPOE_MTU_US_E, mtu);

#ifdef US_DEBUG_PRINT
		printk("IPV4_PPPOE_MTU: mtu %d \n", *mtu);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_ipv6_mtu_us()
*
* DESCRIPTION:      Get ipv6 mtu parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  mtu - ipv6 mtu
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_ipv6_mtu_us(uint32_t *mtu)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (mtu == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*mtu = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_MTU_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_MTU_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_IPV6_MTU_US_E, mtu);

#ifdef US_DEBUG_PRINT
		printk("IPV6_MTU: mtu %d \n", *mtu);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_ipv6_pppoe_mtu_us()
*
* DESCRIPTION:      Get ipv6 pppoe mtu parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  mtu - ipv6 pppoe mtu
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_ipv6_pppoe_mtu_us(uint32_t *mtu)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (mtu == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*mtu = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_MTU_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_MTU_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_IPV6_PPPOE_MTU_US_E, mtu);

#ifdef US_DEBUG_PRINT
		printk("IPV6_PPPOE_MTU: mtu %d \n", *mtu);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_ipv4_mtu_ds()
*
* DESCRIPTION:      Get ipv4 mtu parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  mtu - ipv4 mtu
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_ipv4_mtu_ds(uint32_t *mtu)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (mtu == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*mtu = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_MTU_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_MTU_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_IPV4_MTU_DS_E, mtu);

#ifdef US_DEBUG_PRINT
		printk("IPV4_MTU: mtu %d \n", *mtu);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_ipv6_mtu_ds()
*
* DESCRIPTION:      Get ipv6 mtu parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  mtu - ipv6 mtu
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_ipv6_mtu_ds(uint32_t *mtu)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (mtu == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*mtu = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_MTU_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_MTU_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_IPV6_MTU_DS_E, mtu);

#ifdef US_DEBUG_PRINT
		printk("IPV6_MTU: mtu %d \n", *mtu);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_pppoe_add_enable()
*
* DESCRIPTION:      Get get_pppoe_add_enable parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  enable - pppoe_enable
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_pppoe_add_enable(tpm_init_pppoe_add_enable_t *enable)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;
	uint32_t val;

	if (enable == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*enable = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_TRAFFIC_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_TRAFFIC_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_PPPOE_ENABLE_E, &val);
		if (rc == US_RC_OK) {
			*enable = (tpm_init_pppoe_add_enable_t) val;

#ifdef US_DEBUG_PRINT
			printk("PPPOE_ENABLE: enable %d \n", *enable);
#endif
		}
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_num_vlan_tags()
*
* DESCRIPTION:      Get num_vlan_tags parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  num_vlan_tags - vlan nmuber
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_num_vlan_tags(uint32_t *num_vlan_tags)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (num_vlan_tags == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*num_vlan_tags = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_TRAFFIC_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_TRAFFIC_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_NUM_VLAN_TAGS_E, num_vlan_tags);

#ifdef US_DEBUG_PRINT
		printk("num_vlan_tags %d \n", *num_vlan_tags);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

int get_cpu_rx_queue(uint32_t *cpu_rx_queue)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (cpu_rx_queue == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*cpu_rx_queue = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_TRAFFIC_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_TRAFFIC_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_CPU_RX_QUEUE_E, cpu_rx_queue);

#ifdef US_DEBUG_PRINT
		printk("cpu_rx_queue %d \n", *cpu_rx_queue);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

int get_ttl_illegal_action(uint32_t *ttl_illegal_action)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (ttl_illegal_action == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*ttl_illegal_action = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_TRAFFIC_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_TRAFFIC_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_TTL_ZERO_ACTION_E, ttl_illegal_action);

#ifdef US_DEBUG_PRINT
		printk("ttl_illegal_action %d \n", *ttl_illegal_action);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

int get_tcp_flag_check(uint32_t *tcp_flag_check)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (tcp_flag_check == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*tcp_flag_check = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_TRAFFIC_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_TRAFFIC_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_TCP_FLAG_CHECK_E, tcp_flag_check);

#ifdef US_DEBUG_PRINT
		printk("tcp_flag_check %d \n", *tcp_flag_check);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

int get_mc_filter_mode(tpm_mc_filter_mode_t *mc_filter_mode)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (mc_filter_mode == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*mc_filter_mode = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_MC_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_MC_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_MC_FILTER_MODE_E, mc_filter_mode);

#ifdef US_DEBUG_PRINT
		printk("mc_filter_mode %d \n", *mc_filter_mode);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

int get_mc_per_uni_vlan_xlat(uint32_t *mc_per_uni_vlan_xlat)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (mc_per_uni_vlan_xlat == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*mc_per_uni_vlan_xlat = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_MC_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_MC_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_MC_PER_UNI_VLAN_XLAT_E, mc_per_uni_vlan_xlat);

#ifdef US_DEBUG_PRINT
		printk("mc_per_uni_vlan_xlat %d \n", *mc_per_uni_vlan_xlat);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

int get_mc_pppoe_enable(uint32_t *mc_pppoe_enable)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (mc_pppoe_enable == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*mc_pppoe_enable = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_MC_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_MC_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_MC_PPPOE_ENABLE_E, mc_pppoe_enable);

#ifdef US_DEBUG_PRINT
		printk("mc_pppoe_enable %d \n", *mc_pppoe_enable);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

int get_mc_queue(uint32_t *hwf_queue, uint32_t *cpu_queue)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (hwf_queue == NULL || cpu_queue == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*hwf_queue = 0;
	*cpu_queue = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_MC_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_MC_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_MC_HWF_Q_E, hwf_queue);

#ifdef US_DEBUG_PRINT
		printk("mc_hwf_queue %d \n", *hwf_queue);
#endif

		if (*hwf_queue >= 8) {
			printk(KERN_ERR "%s: Invalid mc_hwf_queue %d\n", __func__, *hwf_queue);
			rc = US_RC_FAIL;
		}

		rc = get_int_param(xmlElement, US_XML_MC_CPU_Q_E, cpu_queue);

#ifdef US_DEBUG_PRINT
		printk("mc_cpu_queue %d \n", *cpu_queue);
#endif

		if (*cpu_queue >= 8) {
			printk(KERN_ERR "%s: Invalid mc_cpu_queue %d\n", __func__, *cpu_queue);
			rc = US_RC_FAIL;
		}
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

int get_catch_all_pkt_action(uint32_t *catch_all_pkt_action)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (catch_all_pkt_action == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*catch_all_pkt_action = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_TPM_E, 0, US_XML_TPM_PNC_E, -1);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_TPM_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_CATCH_ALL_PKT_ACTION_E, catch_all_pkt_action);

#ifdef US_DEBUG_PRINT
		printk("catch_all_pkt_action %d \n", *catch_all_pkt_action);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

int get_ety_dsa_enable(uint32_t *ety_dsa_enable)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (ety_dsa_enable == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*ety_dsa_enable = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_TRAFFIC_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_TRAFFIC_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_ETY_DSA_ENABLE_E, ety_dsa_enable);

#ifdef US_DEBUG_PRINT
		printk("ety_dsa_enable %d \n", *ety_dsa_enable);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

int get_ipv6_5t_enable(uint32_t *ipv6_5t_enable)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (ipv6_5t_enable == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*ipv6_5t_enable = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_TPM_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_TPM_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_IPV6_5T_ENA_E, ipv6_5t_enable);

#ifdef US_DEBUG_PRINT
		printk("ipv6_5t_enable %d \n", *ipv6_5t_enable);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

int get_ipv4_mc_support(uint32_t *ipv4_mc_support)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (ipv4_mc_support == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*ipv4_mc_support = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_MC_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_MC_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_IPV4_MC_SUPPORT_E, ipv4_mc_support);

#ifdef US_DEBUG_PRINT
		printk("ipv4_mc_support %d \n", *ipv4_mc_support);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

int get_ipv6_mc_support(uint32_t *ipv6_mc_support)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (ipv6_mc_support == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*ipv6_mc_support = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_MC_SETTING_E);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_MC_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_IPV6_MC_SUPPORT_E, ipv6_mc_support);

#ifdef US_DEBUG_PRINT
		printk("ipv6_mc_support %d \n", *ipv6_mc_support);
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_split_mod_enable()
*
* DESCRIPTION:      Get split mod enable parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  enable - split enable
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_split_mod_enable(uint32_t *split_mod_enable)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;
	uint32_t val;

	if (split_mod_enable == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*split_mod_enable = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_TPM_E, 0, US_XML_TPM_MOD_E, 0, US_XML_SPLIT_MOD_SETTING_E, -1);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_SPLIT_MOD_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_SPLIT_MOD_ENABLE_E, &val);
		if (rc == US_RC_OK) {
			*split_mod_enable = (tpm_init_split_mod_enable_t) val;

#ifdef US_DEBUG_PRINT
			printk("split_mod_enable %d \n", *split_mod_enable);
#endif
		}
	}
	/*ezxml_free(xmlHead); */
	return rc;
}

int get_split_mod_mode(tpm_init_split_mod_mode_t *split_mod_mode)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;
	uint32_t val;

	if (split_mod_mode == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*split_mod_mode = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_TPM_E, -1);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_TPM_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_SPLIT_MOD_MODE_E, &val);
		if (rc == US_RC_OK) {
			*split_mod_mode = (tpm_init_split_mod_mode_t)val;

#ifdef US_DEBUG_PRINT
			printk("split_mod_mode %d \n", *split_mod_mode);
#endif
		}
	}
	/*ezxml_free(xmlHead); */
	return rc;
}

static int get_pbit_param(ezxml_t xml, char *name, uint8_t *pbit, uint32_t *pbit_num)
{
	ezxml_t xmlElement;
	char tmp[30];
	uint8_t len,i,j,k;

	memset(tmp, '\0', 30);

	xmlElement = ezxml_child(xml, name);
	if (xmlElement == NULL)
		return US_RC_NOT_FOUND;
	else if (xmlElement->txt == NULL) {
		printk(KERN_ERR "NULL value for %s in XML config. file\n", name);
		return US_RC_NOT_FOUND;
	}
	len = strlen(xmlElement->txt);
	for (i = 0, j = 0, k = 0; i < len;) {
		while((xmlElement->txt[i] != ',') && (i < len)) {
			if (xmlElement->txt[i] != ' ') {
				tmp[k] = xmlElement->txt[i];
				k++;
			}
			i++;
		}
		pbit[j] = get_dig_number(tmp);
		memset(tmp, '\0', 30);
		j++;
		i++;
		k = 0;
	}
	/*get valid pbit num*/
	*pbit_num = j;

	return US_RC_OK;
}

/*******************************************************************************
* get_split_mot_p_bits()
*
* DESCRIPTION:      Get split mod p_bits supported parameters from XML configuration file
*
* INPUTS:
*
* OUTPUTS:  p_bits[] - p_bit supported
*	    p_bit_num - number of valid p_bit
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_split_mot_p_bits(uint8_t *p_bits, uint32_t *p_bit_num)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;
	uint8_t i = 0;

	if (p_bits == NULL || p_bit_num == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	memset(p_bits,0xFF,8);

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_TPM_E, 0, US_XML_TPM_MOD_E, 0, US_XML_SPLIT_MOD_SETTING_E, -1);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_SPLIT_MOD_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_pbit_param(xmlElement, US_XML_SPLIT_MOD_P_BITS_E, &p_bits[0], p_bit_num);

#ifdef US_DEBUG_PRINT
		printk("split_mod_p_bit: ");
		for (i = 0; i < 8; i++) {
			if (p_bits[i] != 0xFF)
				printk("%d ", p_bits[i]);
		}
		printk("\n");
#endif
	}

	/*ezxml_free(xmlHead); */
	return rc;
}

/*******************************************************************************
* get_split_mod_vlan_num()
*
* DESCRIPTION:      Get vlan number that needs split pbit and vid modification
*
* INPUTS:
*
* OUTPUTS:  split_vlan_num - vlan nmuber
*
* RETURNS:  US_RC_OK, US_RC_FAIL or US_RC_NOT_FOUND
*
*******************************************************************************/
int get_split_mod_vlan_num(uint32_t *split_vlan_num)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;

	if (split_vlan_num == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*split_vlan_num = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_TPM_E, 0, US_XML_TPM_MOD_E, 0, US_XML_SPLIT_MOD_SETTING_E, -1);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_SPLIT_MOD_SETTING_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_SPLIT_MOD_VLAN_NUM_E, split_vlan_num);

#ifdef US_DEBUG_PRINT
		printk("split_vlan_num %d \n", *split_vlan_num);
#endif
	}
	/*ezxml_free(xmlHead); */
	return rc;
}

int get_ctc_cm_enable(uint32_t *ctc_cm_enable)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;
	uint32_t val;

	if (ctc_cm_enable == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*ctc_cm_enable = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_TPM_E, -1);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_TPM_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_CTC_CNM_ENA_E, &val);
		if (rc == US_RC_OK) {
			*ctc_cm_enable = (tpm_init_ctc_cm_enable_t)val;

#ifdef US_DEBUG_PRINT
			printk("ctc_cnm_enable %d \n", *ctc_cm_enable);
#endif
		}
	}
	/*ezxml_free(xmlHead); */
	return rc;
}
int get_ctc_cm_ipv6_parse_window(uint32_t *ctc_cm_ipv6_parse_window)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;
	uint32_t val;

	if (ctc_cm_ipv6_parse_window == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*ctc_cm_ipv6_parse_window = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_TPM_E, -1);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_TPM_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_CTC_CNM_IPv6_PARSE_WIN_E, &val);
		if (rc == US_RC_OK) {
			*ctc_cm_ipv6_parse_window = val;

#ifdef US_DEBUG_PRINT
			printk("ipv6_parse_window_t %d \n", *ctc_cm_ipv6_parse_window);
#endif
		}
	}
	/*ezxml_free(xmlHead); */
	return rc;
}

int get_pnc_mac_learn_enable(uint32_t *pnc_mac_learn_enable)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;
	uint32_t val;

	if (pnc_mac_learn_enable == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	*pnc_mac_learn_enable = 0;

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_get(xmlHead, US_XML_TPM_E, -1);
	if (xmlElement == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n",
			US_XML_TPM_E, g_pstr_xml_cfg_file);
#endif
		rc = US_RC_NOT_FOUND;
	} else {
		rc = get_int_param(xmlElement, US_XML_PNC_MAC_LEARN_ENA_E, &val);
		if (rc == US_RC_OK) {
			*pnc_mac_learn_enable = (tpm_init_pnc_mac_learn_enable_t)val;

#ifdef US_DEBUG_PRINT
			printk("pnc_mac_learn_enable %d \n", *pnc_mac_learn_enable);
#endif
		}
	}
	/*ezxml_free(xmlHead); */
	return rc;
}
int get_switch_init_params(uint32_t *switch_init)
{
	ezxml_t xmlHead;
	ezxml_t xmlElement;
	int rc = US_RC_OK;
	uint32_t val;

	if (switch_init == NULL) {
#ifdef US_DEBUG_PRINT
		printk(KERN_ERR "%s: NULL pointer\n", __func__);
#endif
		return US_RC_FAIL;
	}

	xmlHead = get_xml_head_ptr(g_pstr_xml_cfg_file);

	if (xmlHead == NULL)
		return US_RC_FAIL;

	xmlElement = ezxml_child(xmlHead, US_XML_TPM_E);
	if (xmlElement == NULL) {
		printk(KERN_ERR "Failed to find %s in XML config. file %s\n", US_XML_TPM_E, g_pstr_xml_cfg_file);
		rc = US_RC_NOT_FOUND;
	} else
		rc = get_int_param(xmlElement, US_XML_SWITCH_INIT_E, &val);

	if (rc == US_RC_OK) {
		*switch_init = val;

#ifdef US_DEBUG_PRINT
		printk("switch_init = %d\n", *switch_init);
#endif
	}
	/*ezxml_free(xmlHead); */
	return rc;
}


