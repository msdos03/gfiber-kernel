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
* tpm_api.c
*
* DESCRIPTION:
*               Traffic Processor Manager = TPM
*
* DEPENDENCIES:
*               None
*
* CREATED BY:   OctaviaP
*
* DATE CREATED:
*
* FILE REVISION NUMBER:
*               Revision: 1.1.1.1
*
*
*******************************************************************************/
#include <linux/version.h>
#include "tpm_common.h"
#include "tpm_header.h"

#define RET_BUSY_ERROR(ret_code, busy_ret_code)\
		if (ret_code != TPM_OK)\
			return(ret_code);\
		else\
			return (busy_ret_code);

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/********************************** Packet Processor APIs *********************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/********************************** Administrative APIs ***********************/
/******************************************************************************/

/*******************************************************************************
* tpm_create_ownerid()
*
* DESCRIPTION:      Creates an ownerId per an application group.
*
* INPUTS:
*
* OUTPUTS:
* owner_id           - ID of an application group.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* This function is used for error prevention and not as a security mechanism.
*
*******************************************************************************/
tpm_error_code_t tpm_create_ownerid(uint32_t *owner_id)
{
	printk(KERN_WARNING "\n  <<tpm_create_ownerid>> NOT implemented. \n");
	return TPM_RC_OK;
}
EXPORT_SYMBOL(tpm_create_ownerid);

/*******************************************************************************
* tpm_request_api_ownership()
*
* DESCRIPTION:      Establishes an ownership between owner_id and a group of APIs.
*
* INPUTS:
* owner_id           - ID of an application which requests ownership on a group of APIs.
* api_type           - the API group whom ownership is requested.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns API_OWNERSHIP_SUCCESS. On error, see tpm_api_ownership_error_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_api_ownership_error_t tpm_request_api_ownership(uint32_t owner_id,
						    tpm_api_type_t api_type)
{
	printk(KERN_WARNING "\n  <<tpm_request_api_ownership>> NOT implemented. \n");
	return API_OWNERSHIP_SUCCESS;
}
EXPORT_SYMBOL(tpm_request_api_ownership);

/*******************************************************************************
* tpm_mib_reset()
*
* DESCRIPTION:      Performs MIB reset
*
* INPUTS:
* owner_id           - ID of an application which requests ownership on a group of APIs.
* reset_level        - The reset level determines the which elements will not be reset in the API call
*                      ( by comparing to the element's reset level)
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns API_OWNERSHIP_SUCCESS. On error, see tpm_api_ownership_error_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_mib_reset(uint32_t owner_id,
			       tpm_reset_level_enum_t reset_level)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_mib_reset(owner_id, reset_level);
	return (ret_code);
}
EXPORT_SYMBOL(tpm_mib_reset);

/*******************************************************************************
* tpm_erase_section()
*
* DESCRIPTION:      Erases a section per an application group.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* api_type          - the API group area to be deleted.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_erase_section(uint32_t owner_id,
				   tpm_api_type_t api_type)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_erase_section(owner_id, api_type);
	return ret_code;
}
EXPORT_SYMBOL(tpm_erase_section);

/*******************************************************************************
* tpm_get_section_free_size()
*
* DESCRIPTION:      Returns the free size of an application group.
*
* INPUTS:
* api_type         - the API group for retrieving the section size.
*
* OUTPUTS:
* cur_size          - number of free entries per API group.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_get_section_free_size(tpm_api_type_t	api_type,
					   int32_t	       *cur_size)
{
	uint32_t api_rng_size, num_valid_entries, last_valid, tbl_start;
	tpm_pnc_ranges_t prim_pnc_range;
	tpm_api_sections_t api_section = TPM_INVALID_SECTION;
	int32_t int_ret_code;

	if (!cur_size)
		return ERR_NULL_POINTER;

	if (ILLEGAL_API(api_type)) {
		printk(KERN_ERR "Error, API Type does not exist (%d))\n", api_type);
		return ERR_API_TYPE_INVALID;
	}

	int_ret_code = tpm_db_api_section_get_from_api_type(api_type, &api_section);
	if (int_ret_code != TPM_OK) {
		printk(KERN_ERR "Error, API Type does not exist (%d))\n", api_type);
		return (int_ret_code);
	}

	int_ret_code = tpm_db_api_section_get(api_section, &api_rng_size, &num_valid_entries,
					      &prim_pnc_range, &last_valid, &tbl_start);
	if (int_ret_code != TPM_OK) {
		printk(KERN_ERR "Error, API Section does not exist\n");
		return (int_ret_code);
	}

	*cur_size = api_rng_size - num_valid_entries;
	return TPM_RC_OK;
}
EXPORT_SYMBOL(tpm_get_section_free_size);

/******************************************************************************/
/********************************** Data Forwarding APIs **********************/
/******************************************************************************/

/*******************************************************************************
* tpm_add_l2_rule()
*
* DESCRIPTION:      Creates a new primary L2 processing ACL.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter has a double function:
*                       . The source port is part of the rules parsing key.
*                       . The source port determines if the acl entry being
*                         created is for the upstream acl or the downstream acl.
*                      In case of an upstream entry, the parameter determines if the packet
*                      arrives from a specific LAN port or ANY LAN port.
* rule_num           - Entry index to be added in the current ACL
* parse_rule_bm      - Bitmap containing the significant flags for parsing fields of the packet.
*                      possible values for L2 API:
*                        TPM_L2_PARSE_MAC_DA|TPM_L2_PARSE_MAC_SA|TPM_L2_PARSE_ONE_VLAN_TAG
*                        |TPM_L2_PARSE_TWO_VLAN_TAG|TPM_L2_PARSE_ETYPE|TPM_L2_PARSE_PPPOE_SES
*                        |TPM_L2_PARSE_PPP_PROT|TPM_L2_PARSE_GEMPORT)
* parse_flags_bm     - Bitmap containing the significant flags result of the primary ACL filtering.
*                      possible values for L2 API:
*                        TPM_PARSE_FLAG_TAG1_TRUE|TPM_PARSE_FLAG_TAG1_FLASE|
*                        TPM_PARSE_FLAG_TAG2_TRUE|TPM_PARSE_FLAG_TAG2_FALSE
* l2_key             - Information to create a parsing key for the rule.
*                      Some pointers may be NULL depending on the parse_rule_bm.
* pkt_frw            - Information for packet forwarding decision.
* pkt_mod            - Packet modification information.
* pkt_mod_bm         - Bitmap containind the set of fields in the packet to be changed.
* rule_action        - Action associated to the rule = drop/set target/set packet modification/to CPU
*                      possible "next_phase" for L2 API   -> STAGE_L3_TYPE , STAGE_DONE
*
* OUTPUTS:
*  rule_idx         - Unique rule identification number which is used when deleting the rule.
*                     (this is not the rule_num)
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_add_l2_rule(uint32_t owner_id,
				 tpm_src_port_type_t src_port,
				 uint32_t rule_num,
				 uint32_t *rule_idx,
				 tpm_parse_fields_t parse_rule_bm,
				 tpm_parse_flags_t parse_flags_bm,
				 tpm_l2_acl_key_t *l2_key,
				 tpm_pkt_frwd_t *pkt_frwd,
				 tpm_pkt_mod_t *pkt_mod,
				 tpm_pkt_mod_bm_t pkt_mod_bm,
				 tpm_rule_action_t *rule_action)
{
	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_section Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_L2_PRIM, rule_num);
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_proc_add_l2_prim_acl_rule(owner_id, src_port, rule_num, rule_idx, parse_rule_bm, parse_flags_bm,
						 l2_key, pkt_frwd, pkt_mod, pkt_mod_bm, rule_action);
	if (ret_code != TPM_OK)
	{
		/* to remove MC_AI_BITS if add l2 acl rule failed */
		tpm_proc_mc_vid_remove_ai_bits(rule_action, src_port, l2_key, ret_code);
	}
	busy_ret_code = tpm_proc_api_busy_done(TPM_API_L2_PRIM, rule_num);
	RET_BUSY_ERROR(ret_code, busy_ret_code);
}
EXPORT_SYMBOL(tpm_add_l2_rule);

/*******************************************************************************
* tpm_add_l3_type_rule()
*
* DESCRIPTION:      Creates a new L3 type (ether type of pppoe proto) processing ACL.
*                   It is used for operations that are not possible to be performed
*                   in a single ACL or to ease the primary ACL processing
*                   (as a helper of the primary L2 ACL).
*                   The L3 type ACL is optional.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter has a double function:
*                       . The source port is part of the rules parsing key.
*                       . The source port determines if the acl entry being
*                         created is for the upstream acl or the downstream acl.
*                      In case of an upstream entry, the parameter determines if the packet
*                      arrives from a specific LAN port or ANY LAN port.
* rule_num           - Entry index to be added in the current ACL
* parse_rule_bm      - Bitmap containing the significant flags for parsing fields of the packet.
*                      possible values for L3 API:
*                        TPM_L2_PARSE_ETYPE|TPM_L2_PARSE_PPPOE_SES|TPM_L2_PARSE_PPP_PROT
* parse_flags_bm     - Bitmap containing the significant flags result of the primary ACL filtering.
*                      possible values for L3 API:
*                        TPM_PARSE_FLAG_TAG1_TRUE|TPM_PARSE_FLAG_TAG1_FLASE|
*                        TPM_PARSE_FLAG_TAG2_TRUE|TPM_PARSE_FLAG_TAG2_FALSE|
*                        TPM_PARSE_FLAG_MTM_TRUE|TPM_PARSE_FLAG_MTM_FALSE|
*                        TPM_PARSE_FLAG_TO_CPU_TRUE|TPM_PARSE_FLAG_TO_CPU_FALSE
* l3_key             - Structure for PPPoE proto or ether type. In order to define a rule for
*                      any ether type, the ether type value should be set to 0xFFFF
* action_drop        - If this stage is dropping the packet.
* pkt_frw            - Information for packet forwarding decision.
* rule_action        - Action associated to the rule = drop/set target/set packet modification/to CPU
*                      possible "next_phase" for L3 API   ->  STAGE_IPv4, STAGE_IPv6_GEN, STAGE_DONE
*
* OUTPUTS:
*  rule_idx         - Unique rule identification number, which is used when deleting the rule.
*                     (this is not the rule_num)
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_add_l3_type_rule(uint32_t owner_id,
				      tpm_src_port_type_t src_port,
				      uint32_t rule_num,
				      uint32_t *rule_idx,
				      tpm_parse_fields_t parse_rule_bm,
				      tpm_parse_flags_t parse_flags_bm,
				      tpm_l3_type_key_t *l3_key,
				      tpm_pkt_frwd_t *pkt_frwd,
				      tpm_rule_action_t *rule_action)
{
	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_type Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_L3_TYPE, rule_num);
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_proc_add_l3_type_acl_rule(owner_id, src_port, rule_num,
						 rule_idx, parse_rule_bm, parse_flags_bm,
						 l3_key, pkt_frwd, rule_action);
	busy_ret_code = tpm_proc_api_busy_done(TPM_API_L3_TYPE, rule_num);
	RET_BUSY_ERROR(ret_code, busy_ret_code);
}
EXPORT_SYMBOL(tpm_add_l3_type_rule);

/*******************************************************************************
* tpm_del_l2_rule()
*
* DESCRIPTION:      Deletes an existing L2 ACL primary rule.
*                   Any of the existing access-list entries may be deleted. The src_port parameter
*                   determines if the rule to be deleted belongs to the upstream access-list
*                   or the downstream access-list. All parameters are compulsory. If the rule number
*                   does not match the internally stored src_port and parsing key
*                   (parse_rule_bm and l2_key), the API will return an error.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter has a double function:
*                       . The source port is part of the rules parsing key.
*                       . The source port determines if the acl entry being
*                         created is for the upstream acl or the downstream acl.
*                      In case of an upstream entry, the parameter determines if the packet
*                      arrives from a specific LAN port or ANY LAN port.
* rule_idx           - Unique rule idenitifcation number specifying the rule to be deleted.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_del_l2_rule(uint32_t owner_id,
				 uint32_t rule_idx)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_del_l2_prim_acl_rule(owner_id, rule_idx, TPM_EXT_CALL);
	return ret_code;
}
EXPORT_SYMBOL(tpm_del_l2_rule);

/*******************************************************************************
* tpm_del_l3_type_rule()
*
* DESCRIPTION:      Deletes an existing ethernet type or PPPoE proto access-list entry .
*                   Any of the existing access-list entries may be deleted. The src_port parameter
*                   determines if the rule to be deleted belongs to the upstream access-list
*                   or the downstream access-list. All parameters are compulsory. If the rule number
*                   does not match the internally stored src_port and parsing key
*                   (parse_rule_bm and l3_key), the API will return an error.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter has a double function:
*                       . The source port is part of the rules parsing key.
*                       . The source port determines if the acl entry being
*                         created is for the upstream acl or the downstream acl.
*                      In case of an upstream entry, the parameter determines if the packet
*                      arrives from a specific LAN port or ANY LAN port.
* rule_idx           - Unique rule idenitifcation number specifying the rule to be deleted.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_del_l3_type_rule(uint32_t owner_id,
				      uint32_t rule_idx)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_del_l3_type_acl_rule(owner_id, rule_idx, TPM_EXT_CALL);
	return ret_code;
}
EXPORT_SYMBOL(tpm_del_l3_type_rule);

/*******************************************************************************
* tpm_add_ipv4_rule()
*
* DESCRIPTION:      Creates a new IPv4 processing ACL.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter has a double function:
*                       . The source port is part of the rules parsing key.
*                       . The source port determines if the acl entry being
*                         created is for the upstream acl or the downstream acl.
*                      In case of an upstream entry, the parameter determines if the packet
*                      arrives from a specific LAN port or ANY LAN port.
* rule_num           - Entry index to be added in the current ACL
* parse_rule_bm      - Bitmap containing the significant flags for parsing fields of the packet.
*                      possible values for IPv4 API:
*                        TPM_IPv4_PARSE_SIP|TPM_IPv4_PARSE_DIP|TPM_IPv4_PARSE_DSCP
*                         |TPM_IPv4_PARSE_PROTO|TPM_PARSE_L4_SRC|TPM_PARSE_L4_DST
* parse_flags_bm     - Bitmap containing the significant flags result of the primary ACL filtering.
*                      possible values for IPv4 API:
*                        TPM_PARSE_FLAG_TAG1_TRUE|TPM_PARSE_FLAG_TAG1_FLASE|
*                        TPM_PARSE_FLAG_MTM_TRUE|TPM_PARSE_FLAG_MTM_FALSE|
*                        TPM_PARSE_FLAG_TO_CPU_TRUE|TPM_PARSE_FLAG_TO_CPU_FALSE|
*                        TPM_PARSE_FLAG_PPPOE_TRUE|TPM_PARSE_FLAG_PPPOE_FALSE
* ipv4_key           - Information to create an IPv4 parsing key for the rule.
*                      Some pointers may be NULL depending on the parse_rule_bm.
* pkt_frwd           - Information for packet forwarding decision.
* pkt_mod            - VLAN packet modification information.
* pkt_mod_bm         - Bitmap containing the significant fields to modify (used for GWY only - in SFU is NULL)
* rule_action        - Action associated to the rule = drop/set target/set packet modification/to CPU
*                      and the next phase.
*                      possible "next_phase" for IPv4 API   ->  STAGE_DONE
*
* OUTPUTS:
*  rule_idx         - Unique rule identification number, which is used when deleting the rule.
*                     (this is not the rule_num)
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_add_ipv4_rule(uint32_t owner_id,
				   tpm_src_port_type_t src_port,
				   uint32_t rule_num,
				   uint32_t *rule_idx,
				   tpm_parse_fields_t parse_rule_bm,
				   tpm_parse_flags_t parse_flags_bm,
				   tpm_ipv4_acl_key_t *ipv4_key,
				   tpm_pkt_frwd_t *pkt_frwd,
				   tpm_pkt_mod_t *pkt_mod,
				   tpm_pkt_mod_bm_t pkt_mod_bm,
				   tpm_rule_action_t *rule_action)
{
	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_type Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_IPV4, rule_num);
	if (ret_code != TPM_OK)
		return(ret_code);


	ret_code = tpm_proc_add_ipv4_acl_rule(owner_id, src_port, rule_num, rule_idx, parse_rule_bm,
					      parse_flags_bm, ipv4_key, pkt_frwd, pkt_mod, pkt_mod_bm, rule_action);

	busy_ret_code = tpm_proc_api_busy_done(TPM_API_IPV4, rule_num);

	RET_BUSY_ERROR(ret_code, busy_ret_code);
}
EXPORT_SYMBOL(tpm_add_ipv4_rule);

/*******************************************************************************
* tpm_del_ipv4_rule()
*
* DESCRIPTION:      Deletes an existing IPv4 ACL rule.
*                   Both rule number and key are compulsory.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter has a double function:
*                       . The source port is part of the rules parsing key.
*                       . The source port determines if the acl entry being
*                         created is for the upstream acl or the downstream acl.
*                      In case of an upstream entry, the parameter determines if the packet
*                      arrives from a specific LAN port or ANY LAN port.
* rule_idx           - Unique rule idenitifcation number specifying the rule to be deleted.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_del_ipv4_rule(uint32_t owner_id,
				   uint32_t rule_idx)
{

	tpm_error_code_t ret_code;

	ret_code = tpm_proc_del_ipv4_acl_rule(owner_id, rule_idx, TPM_EXT_CALL);
	return (ret_code);
}
EXPORT_SYMBOL(tpm_del_ipv4_rule);

/*******************************************************************************
* tpm_add_ipv6_nh_rule()
*
* DESCRIPTION:      Creates a new IPv6 NH processing ACL.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* rule_num           - Entry index to be added in the current ACL
* parse_flags_bm     - Bitmap containing the significant flags result of the primary ACL filtering.
*                      possible values for IPv6 NH API:
*                        TPM_PARSE_FLAG_MTM_TRUE|TPM_PARSE_FLAG_MTM_FALSE|
*                        TPM_PARSE_FLAG_TO_CPU_TRUE|TPM_PARSE_FLAG_TO_CPU_FALSE|
* nh                 - Information to create a NH parsing key for the rule.
* pkt_frwd           - Information for packet forwarding decision.
* rule_action        - Action associated to the rule = drop/set target/set packet modification/to CPU
*                      and the next phase (for GWY only).
*                      possible "next_phase" for IPv6 NH API   ->  STAGE_IPV6_L4,STAGE_DONE
* OUTPUTS:
*  rule_idx         - Unique rule identification number, which is used when deleting the rule.
*                     (this is not the rule_num)
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_add_ipv6_nh_rule(uint32_t owner_id,
				      uint32_t rule_num,
				      uint32_t *rule_idx,
				      tpm_parse_flags_t parse_flags_bm,
				      tpm_nh_iter_t nh_iter,
				      uint32_t nh,
				      tpm_pkt_frwd_t *pkt_frwd,
				      tpm_rule_action_t *rule_action)
{
	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_type Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_IPV6_NH, rule_num);
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_proc_add_ipv6_nh_acl_rule(owner_id, rule_num, rule_idx, parse_flags_bm,
						 nh_iter, nh, pkt_frwd, rule_action);

	busy_ret_code = tpm_proc_api_busy_done(TPM_API_IPV6_NH, rule_num);

	RET_BUSY_ERROR(ret_code, busy_ret_code);
}
EXPORT_SYMBOL(tpm_add_ipv6_nh_rule);

/*******************************************************************************
* tpm_del_ipv6_nh_rule()
*
* DESCRIPTION:      Deletes an existng IPv6 NH processing ACL.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* rule_idx           - Unique rule idenitifcation number specifying the rule to be deleted.
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_del_ipv6_nh_rule(uint32_t owner_id,
				      uint32_t rule_idx)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_del_ipv6_nh_acl_rule(owner_id, rule_idx, TPM_EXT_CALL);
	return ret_code;
}
EXPORT_SYMBOL(tpm_del_ipv6_nh_rule);

/*******************************************************************************
* tpm_add_ipv6_gen_rule()
*
* DESCRIPTION:      Creates a new IPv6 gen processing ACL.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter has a double function:
*                       . The source port is part of the rules parsing key.
*                       . The source port determines if the acl entry being
*                         created is for the upstream acl or the downstream acl.
*                      In case of an upstream entry, the parameter determines if the packet
*                      arrives from a specific LAN port or ANY LAN port.
* rule_num           - Entry index to be added in the current ACL
* parse_rule_bm      - Bitmap containing the significant flags for parsing fields of the packet.
*                      possible values for IPv6 GEN API: 0
* parse_flags_bm     - Bitmap containing the significant flags result of the primary ACL filtering.
*                      possible values for IPv6 GEN API:
*                        TPM_PARSE_FLAG_TAG1_TRUE|TPM_PARSE_FLAG_TAG1_FLASE|
*                        TPM_PARSE_FLAG_MTM_TRUE|TPM_PARSE_FLAG_MTM_FALSE|
*                        TPM_PARSE_FLAG_TO_CPU_TRUE|TPM_PARSE_FLAG_TO_CPU_FALSE|
*                        TPM_PARSE_FLAG_PPPOE_TRUE|TPM_PARSE_FLAG_PPPOE_FALSE
* ipv6_gen_key     - Information to create an IPv6 gen parsing key for the rule.
*                      Some pointers may be NULL depending on the parse_rule_bm.
* pkt_frwd           - Information for packet forwarding decision.
* rule_action        - Action associated to the rule = drop/set target/set packet modification/to CPU
*                      and the next phase (for GWY only).
*                      possible "next_phase" for IPv6 GEN API   ->  STAGE_IPv6_DIP, ,STAGE_DONE
*
* OUTPUTS:
*  rule_idx          - Unique rule identification number, which is used when deleting the rule.
*                      (this is not the rule_num)
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_add_ipv6_gen_rule(uint32_t owner_id,
					 tpm_src_port_type_t src_port,
					 uint32_t rule_num,
					 uint32_t *rule_idx,
					 tpm_parse_fields_t parse_rule_bm,
					 tpm_parse_flags_t parse_flags_bm,
					 tpm_ipv6_gen_acl_key_t *ipv6_gen_key,
					 tpm_pkt_frwd_t *pkt_frwd,
					 tpm_pkt_mod_t *pkt_mod,
					 tpm_pkt_mod_bm_t pkt_mod_bm,
					 tpm_rule_action_t *rule_action)
{
	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_type Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_IPV6_GEN, rule_num);
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_proc_add_ipv6_gen_acl_rule(owner_id, src_port, rule_num, rule_idx, parse_rule_bm,
						    parse_flags_bm, ipv6_gen_key, pkt_frwd, pkt_mod, pkt_mod_bm,
						    rule_action);

	busy_ret_code = tpm_proc_api_busy_done(TPM_API_IPV6_GEN, rule_num);

	RET_BUSY_ERROR(ret_code, busy_ret_code);
}
EXPORT_SYMBOL(tpm_add_ipv6_gen_rule);

/*******************************************************************************
* tpm_del_ipv6_gen_rule()
*
* DESCRIPTION:      Deletes an existng IPv6 gen processing ACL.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter has a double function:
*                       . The source port is part of the rules parsing key.
*                       . The source port determines if the acl entry being
*                         created is for the upstream acl or the downstream acl.
*                      In case of an upstream entry, the parameter determines if the packet
*                      arrives from a specific LAN port or ANY LAN port.
* rule_idx           - Unique rule idenitifcation number specifying the rule to be deleted.
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_del_ipv6_gen_rule(uint32_t owner_id,
					 uint32_t rule_idx)
{
	tpm_error_code_t ret_code;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable;

	tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);

	if (ipv6_5t_enable != TPM_IPV6_5T_DISABLED)
		return ERR_IPV6_API_ILLEGAL_CALL;

	ret_code = tpm_proc_del_ipv6_gen_acl_rule(owner_id, rule_idx, TPM_EXT_CALL);
	return ret_code;
}
EXPORT_SYMBOL(tpm_del_ipv6_gen_rule);

/*******************************************************************************
* tpm_add_ipv6_dip_rule()
*
* DESCRIPTION:      Creates a new IPv6 DIP processing ACL.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter has a double function:
*                       . The source port is part of the rules parsing key.
*                       . The source port determines if the acl entry being
*                         created is for the upstream acl or the downstream acl.
*                      In case of an upstream entry, the parameter determines if the packet
*                      arrives from a specific LAN port or ANY LAN port.
* rule_num           - Entry index to be added in the current ACL
* parse_rule_bm      - Bitmap containing the significant flags for parsing fields of the packet.
*                      possible values for IPv6 DIP API: TPM_IPv6_PARSE_DIP
* parse_flags_bm     - Bitmap containing the significant flags result of the primary ACL filtering.
*                      possible values for IPv6 DIP API:
*                        TPM_PARSE_FLAG_TAG1_TRUE|TPM_PARSE_FLAG_TAG1_FLASE|
*                        TPM_PARSE_FLAG_MTM_TRUE|TPM_PARSE_FLAG_MTM_FALSE|
*                        TPM_PARSE_FLAG_TO_CPU_TRUE|TPM_PARSE_FLAG_TO_CPU_FALSE|
*                        TPM_PARSE_FLAG_PPPOE_TRUE|TPM_PARSE_FLAG_PPPOE_FALSE
* ipv6_dip_key       - Information to create an IPv6 DIP parsing key for the rule.
*                      Some pointers may be NULL depending on the parse_rule_bm.
* pkt_frwd           - Information for packet forwarding decision.
* pkt_mod            - Packet modification information.
* pkt_mod_bm         - Bitmap containing the significant fields to modify (used for GWY only - in SFU is NULL)
* rule_action        - Action associated to the rule = drop/set target/set packet modification/to CPU
*                      and the next phase (for GWY only).
*                      possible "next_phase" for IPv6 GEN API   ->  STAGE_IPv6_NHSTAGE_DONE
*
* OUTPUTS:
*  rule_idx          - Unique rule identification number, which is used when deleting the rule.
*                      (this is not the rule_num)
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_add_ipv6_dip_rule(uint32_t owner_id,
				       tpm_src_port_type_t src_port,
				       uint32_t rule_num,
				       uint32_t *rule_idx,
				       tpm_parse_fields_t parse_rule_bm,
				       tpm_parse_flags_t parse_flags_bm,
				       tpm_ipv6_addr_key_t *ipv6_dip_key,
				       tpm_pkt_frwd_t *pkt_frwd,
				       tpm_pkt_mod_t *pkt_mod,
				       tpm_pkt_mod_bm_t pkt_mod_bm,
				       tpm_rule_action_t *rule_action)
{
	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_type Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_IPV6_DIP, rule_num);
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_proc_add_ipv6_dip_acl_rule(owner_id, src_port, rule_num, rule_idx, parse_rule_bm,
						parse_flags_bm, ipv6_dip_key, pkt_frwd, pkt_mod, pkt_mod_bm,
						rule_action);

	busy_ret_code = tpm_proc_api_busy_done(TPM_API_IPV6_DIP, rule_num);

	RET_BUSY_ERROR(ret_code, busy_ret_code);
}
EXPORT_SYMBOL(tpm_add_ipv6_dip_rule);

/*******************************************************************************
* tpm_del_ipv6_dip_rule()
*
* DESCRIPTION:      Deletes an existng IPv6 DIP processing ACL.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter has a double function:
*                       . The source port is part of the rules parsing key.
*                       . The source port determines if the acl entry being
*                         created is for the upstream acl or the downstream acl.
*                      In case of an upstream entry, the parameter determines if the packet
*                      arrives from a specific LAN port or ANY LAN port.
* rule_idx           - Unique rule idenitifcation number specifying the rule to be deleted.
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_del_ipv6_dip_rule(uint32_t owner_id,
				       uint32_t rule_idx)
{
	tpm_error_code_t ret_code;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable;

	tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);

	if (ipv6_5t_enable != TPM_IPV6_5T_DISABLED)
		return ERR_IPV6_API_ILLEGAL_CALL;

	ret_code = tpm_proc_del_ipv6_dip_acl_rule(owner_id, rule_idx, TPM_EXT_CALL);
	return ret_code;
}
EXPORT_SYMBOL(tpm_del_ipv6_dip_rule);

/*******************************************************************************
* tpm_add_ipv6_l4_ports_rule()
*
* DESCRIPTION:      Creates a new IPv6 L4 processing ACL.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter has a double function:
*                       . The source port is part of the rules parsing key.
*                       . The source port determines if the acl entry being
*                         created is for the upstream acl or the downstream acl.
*                      In case of an upstream entry, the parameter determines if the packet
*                      arrives from a specific LAN port or ANY LAN port.
* rule_num           - Entry index to be added in the current ACL
* parse_rule_bm      - Bitmap containing the significant flags for parsing fields of the packet.
*                      possible values for L4 API: TPM_PARSE_L4_SRC|TPM_PARSE_L4_DST
* parse_flags_bm     - Bitmap containing the significant flags result of the primary ACL filtering.
*                      possible values for L4 API:
*                        TPM_PARSE_FLAG_MTM_TRUE|TPM_PARSE_FLAG_MTM_FALSE|
*                        TPM_PARSE_FLAG_TO_CPU_TRUE|TPM_PARSE_FLAG_TO_CPU_FALSE|
*                        TPM_PARSE_FLAG_L4_TCP|TPM_PARSE_FLAG_L4_UDP
* l4_key             - Information to create an L4 parsing key for the rule.
*                      Some pointers may be NULL depending on the parse_rule_bm.
* pkt_frwd           - Information for packet forwarding decision.
* pkt_mod            - Packet modification information.
* pkt_mod_bm         - Bitmap containing the significant fields to modify (used for GWY only - in SFU is NULL)
* rule_action        - Action associated to the rule = drop/set target/set packet modification/to CPU
*                      and the next phase (for GWY only).
*                      possible "next_phase" for L4 API   ->  STAGE_DONE
*
* OUTPUTS:
*  rule_idx          - Unique rule identification number, which is used when deleting the rule.
*                      (this is not the rule_num)
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_add_ipv6_l4_ports_rule(uint32_t owner_id,
					    tpm_src_port_type_t src_port,
					    uint32_t rule_num,
					    uint32_t *rule_idx,
					    tpm_parse_fields_t parse_rule_bm,
					    tpm_parse_flags_t parse_flags_bm,
					    tpm_l4_ports_key_t *l4_key,
					    tpm_pkt_frwd_t *pkt_frwd,
					    tpm_pkt_mod_t *pkt_mod,
					    tpm_pkt_mod_bm_t pkt_mod_bm,
					    tpm_rule_action_t *rule_action)
{

	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_type Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_IPV6_L4, rule_num);
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_proc_add_ipv6_l4_ports_acl_rule(owner_id, src_port, rule_num, rule_idx, parse_rule_bm,
							parse_flags_bm, l4_key, pkt_frwd, pkt_mod, pkt_mod_bm,
							rule_action);

	busy_ret_code = tpm_proc_api_busy_done(TPM_API_IPV6_L4, rule_num);

	RET_BUSY_ERROR(ret_code, busy_ret_code);
}
EXPORT_SYMBOL(tpm_add_ipv6_l4_ports_rule);

/*******************************************************************************
* tpm_del_ipv6_l4_ports_rule()
*
* DESCRIPTION:      Deletes an existng L4 processing ACL.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter has a double function:
*                       . The source port is part of the rules parsing key.
*                       . The source port determines if the acl entry being
*                         created is for the upstream acl or the downstream acl.
*                      In case of an upstream entry, the parameter determines if the packet
*                      arrives from a specific LAN port or ANY LAN port.
* rule_idx           - Unique rule idenitifcation number specifying the rule to be deleted.
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_del_ipv6_l4_ports_rule(uint32_t owner_id,
					    uint32_t rule_idx)
{
	tpm_error_code_t ret_code;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable;

	tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);

	if (ipv6_5t_enable != TPM_IPV6_5T_DISABLED)
		return ERR_IPV6_API_ILLEGAL_CALL;

	ret_code = tpm_proc_del_ipv6_l4_ports_acl_rule(owner_id, rule_idx, TPM_EXT_CALL);
	return ret_code;
}
EXPORT_SYMBOL(tpm_del_ipv6_l4_ports_rule);


tpm_error_code_t tpm_add_ipv6_gen_5t_rule(uint32_t owner_id,
					  tpm_dir_t src_dir,
					  uint32_t rule_num,
					  uint32_t *rule_idx,
					  tpm_parse_fields_t parse_rule_bm,
					  tpm_parse_flags_t parse_flags_bm,
					  tpm_l4_ports_key_t *l4_key,
					  tpm_ipv6_gen_acl_key_t *ipv6_gen_key,
					  tpm_pkt_frwd_t *pkt_frwd,
					  tpm_pkt_mod_t *pkt_mod,
					  tpm_pkt_mod_bm_t pkt_mod_bm,
					  tpm_rule_action_t *rule_action)
{
	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_type Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_IPV6_GEN, rule_num);
	if (ret_code != TPM_OK)
		return(ret_code);


	ret_code = tpm_proc_add_ipv6_gen_5t_rule(owner_id, src_dir, rule_num, rule_idx, parse_rule_bm, parse_flags_bm,
						l4_key, ipv6_gen_key, pkt_frwd, pkt_mod, pkt_mod_bm, rule_action);

	busy_ret_code = tpm_proc_api_busy_done(TPM_API_IPV6_GEN, rule_num);

	RET_BUSY_ERROR(ret_code, busy_ret_code);
}
EXPORT_SYMBOL(tpm_add_ipv6_gen_5t_rule);

tpm_error_code_t tpm_del_ipv6_gen_5t_rule(uint32_t owner_id, uint32_t rule_idx)
{
	tpm_error_code_t ret_code;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable;

	tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);

	if (ipv6_5t_enable == TPM_IPV6_5T_DISABLED)
		return ERR_IPV6_API_ILLEGAL_CALL;

	ret_code = tpm_proc_del_ipv6_gen_5t_rule(owner_id, rule_idx, TPM_EXT_CALL);
	return ret_code;
}
EXPORT_SYMBOL(tpm_del_ipv6_gen_5t_rule);

tpm_error_code_t tpm_add_ipv6_dip_5t_rule(uint32_t owner_id,
					  tpm_dir_t src_dir,
					  uint32_t rule_num,
					  uint32_t *rule_idx,
					  tpm_parse_fields_t parse_rule_bm,
					  tpm_parse_flags_t parse_flags_bm,
					  tpm_l4_ports_key_t *l4_key,
					  tpm_ipv6_gen_acl_key_t *ipv6_gen_key,
					  tpm_ipv6_addr_key_t *ipv6_dip_key,
					  tpm_pkt_frwd_t *pkt_frwd,
					  tpm_pkt_mod_t *pkt_mod,
					  tpm_pkt_mod_bm_t pkt_mod_bm,
					  tpm_rule_action_t *rule_action)
{
	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_type Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_IPV6_DIP, rule_num);
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_proc_add_ipv6_dip_5t_rule(owner_id, src_dir, rule_num, rule_idx, parse_rule_bm, parse_flags_bm,
										l4_key, ipv6_gen_key, ipv6_dip_key, pkt_frwd, pkt_mod, pkt_mod_bm, rule_action);


	busy_ret_code = tpm_proc_api_busy_done(TPM_API_IPV6_DIP, rule_num);

	RET_BUSY_ERROR(ret_code, busy_ret_code);
}
EXPORT_SYMBOL(tpm_add_ipv6_dip_5t_rule);

tpm_error_code_t tpm_del_ipv6_dip_5t_rule(uint32_t owner_id, uint32_t rule_idx)
{
	tpm_error_code_t ret_code;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable;

	tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);

	if (ipv6_5t_enable == TPM_IPV6_5T_DISABLED)
		return ERR_IPV6_API_ILLEGAL_CALL;

	ret_code = tpm_proc_del_ipv6_dip_5t_rule(owner_id, rule_idx, TPM_EXT_CALL);
	return ret_code;
}
EXPORT_SYMBOL(tpm_del_ipv6_dip_5t_rule);

tpm_error_code_t tpm_add_ipv6_l4_ports_5t_rule(uint32_t owner_id,
					       tpm_dir_t src_dir,
					       uint32_t rule_num,
					       uint32_t *rule_idx,
					       tpm_parse_fields_t parse_rule_bm,
					       tpm_parse_flags_t parse_flags_bm,
					       tpm_l4_ports_key_t *l4_key,
					       tpm_pkt_frwd_t *pkt_frwd,
					       tpm_pkt_mod_t *pkt_mod,
					       tpm_pkt_mod_bm_t pkt_mod_bm,
					       tpm_rule_action_t *rule_action)
{
	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_type Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_IPV6_L4, rule_num);
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_proc_add_ipv6_l4_ports_5t_rule(owner_id, src_dir, rule_num, rule_idx, parse_rule_bm, parse_flags_bm,
						      l4_key, pkt_frwd, pkt_mod, pkt_mod_bm, rule_action);

	busy_ret_code = tpm_proc_api_busy_done(TPM_API_IPV6_L4, rule_num);

	RET_BUSY_ERROR(ret_code, busy_ret_code);
}
EXPORT_SYMBOL(tpm_add_ipv6_l4_ports_5t_rule);

tpm_error_code_t tpm_del_ipv6_l4_ports_5t_rule(uint32_t owner_id, uint32_t rule_idx)
{
	tpm_error_code_t ret_code;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable;

	tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);

	if (ipv6_5t_enable == TPM_IPV6_5T_DISABLED)
		return ERR_IPV6_API_ILLEGAL_CALL;

	ret_code = tpm_proc_del_ipv6_l4_ports_5t_rule(owner_id, rule_idx, TPM_EXT_CALL);
	return ret_code;
}
EXPORT_SYMBOL(tpm_del_ipv6_l4_ports_5t_rule);


/*******************************************************************************
* tpm_set_active_wan()
*
* DESCRIPTION:      Set active WAN port
*
* INPUTS:
* owner_id          - APP owner id  should be used for all API calls.
* active_wan        - active wan, GMAC0, GMAC1, PON
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_set_active_wan(uint32_t owner_id,
				    tpm_gmacs_enum_t active_wan)
{
	tpm_error_code_t tpm_ret;

	tpm_ret = tpm_proc_set_active_wan(active_wan);
	if(TPM_OK != tpm_ret) {
		TPM_OS_ERROR(TPM_DB_MOD, "set active wan has failed with error code (%d)\n", tpm_ret);
		return tpm_ret;
	}

	return (TPM_OK);
}
EXPORT_SYMBOL(tpm_set_active_wan);
/*******************************************************************************
* tpm_hot_swap_profile()
*
* DESCRIPTION:      Swap profile and update all the ACL rules according to
*                   the new profile
*
* INPUTS:
* owner_id          - APP owner id  should be used for all API calls.
* profile_id        - the new profile that system is swapping to
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_hot_swap_profile(uint32_t owner_id,
				    tpm_eth_complex_profile_t profile_id)
{
#ifdef CONFIG_MV_ETH_WAN_SWAP

	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_type Busy */
	ret_code = tpm_proc_check_all_api_busy();
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_proc_hot_swap_profile(owner_id, profile_id);

	busy_ret_code = tpm_proc_all_api_busy_done();

	RET_BUSY_ERROR(ret_code, busy_ret_code);
#else
	TPM_OS_ERROR(TPM_DB_MOD, "hot swap profile feature is not supported on this product!\n");
	return TPM_OK;

#endif /* CONFIG_MV_ETH_WAN_SWAP */

}
EXPORT_SYMBOL(tpm_hot_swap_profile);

/******************************************************************************/
/********************************** MC handling APIs **************************/
/******************************************************************************/

/*******************************************************************************
* tpm_add_ipv4_mc_stream()
*
* DESCRIPTION:      Creates a new IPv4 MC stream.
*                   It is APIs caller responsibility to maintain the correct number of
*                   each stream number.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* stream_num         - MC stream number.
* vid                - VLAN ID (0-4095). If set to 4096 - stream is untagged.
*                      If set to 0xFFFF - do not care.
* ipv4_src_add       - IPv4 source IP address in network order.
* ipv4_src_add       - IPv4 destination IP address in network order.
* ignore_ipv4_src    - when set to 1 - the IP source is not part of the key.
* dest_port_bm       - bitmap which includes all destination UNI ports.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_add_ipv4_mc_stream(uint32_t owner_id,
					uint32_t stream_num,
					tpm_mc_igmp_mode_t igmp_mode,
					uint8_t mc_stream_pppoe,
					uint16_t vid,
					uint8_t ipv4_src_add[4],
					uint8_t ipv4_dst_add[4],
					uint8_t ignore_ipv4_src,
					tpm_trg_port_type_t dest_port_bm)
{
	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_type Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_IPV4_MC, stream_num);
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_proc_add_ipv4_mc_stream(owner_id, stream_num, igmp_mode, mc_stream_pppoe,
						vid, ipv4_src_add, ipv4_dst_add, ignore_ipv4_src,
						TPM_INVALID_QUEUE, dest_port_bm);

	busy_ret_code = tpm_proc_api_busy_done(TPM_API_IPV4_MC, stream_num);

	RET_BUSY_ERROR(ret_code, busy_ret_code);
}
EXPORT_SYMBOL(tpm_add_ipv4_mc_stream);

/*******************************************************************************
* tpm_add_ipv4_mc_stream_set_queue()
*
* DESCRIPTION:      Creates a new IPv4 MC stream with dest Queue.
*                   It is APIs caller responsibility to maintain the correct number of
*                   each stream number.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* stream_num         - MC stream number.
* vid                - VLAN ID (0-4095). If set to 4096 - stream is untagged.
*                      If set to 0xFFFF - do not care.
* ipv4_src_add       - IPv4 source IP address in network order.
* ipv4_src_add       - IPv4 destination IP address in network order.
* ignore_ipv4_src    - when set to 1 - the IP source is not part of the key.
* dest_queue          - destination queue number.
* dest_port_bm       - bitmap which includes all destination UNI ports.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_add_ipv4_mc_stream_set_queue(uint32_t owner_id,
					uint32_t stream_num,
					tpm_mc_igmp_mode_t igmp_mode,
					uint8_t mc_stream_pppoe,
					uint16_t vid,
					uint8_t ipv4_src_add[4],
					uint8_t ipv4_dst_add[4],
					uint8_t ignore_ipv4_src,
					uint16_t dest_queue,
					tpm_trg_port_type_t dest_port_bm)
{
	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_type Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_IPV4_MC, stream_num);
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_proc_add_ipv4_mc_stream(owner_id, stream_num, igmp_mode, mc_stream_pppoe,
						vid, ipv4_src_add, ipv4_dst_add, ignore_ipv4_src,
						dest_queue, dest_port_bm);

	busy_ret_code = tpm_proc_api_busy_done(TPM_API_IPV4_MC, stream_num);

	RET_BUSY_ERROR(ret_code, busy_ret_code);
}
EXPORT_SYMBOL(tpm_add_ipv4_mc_stream_set_queue);

/*******************************************************************************
* tpm_updt_ipv4_mc_stream()
*
* DESCRIPTION:      Updates an existing IPv4 MC stream.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* stream_num         - MC stream number.
* dest_port_bm       - bitmap which includes all destination UNI ports.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_updt_ipv4_mc_stream(uint32_t owner_id,
					 uint32_t stream_num,
					 tpm_trg_port_type_t dest_port_bm)
{
	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_type Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_IPV4_MC, stream_num);
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_proc_updt_ipv4_mc_stream(owner_id, stream_num, dest_port_bm);

	busy_ret_code = tpm_proc_api_busy_done(TPM_API_IPV4_MC, stream_num);

	RET_BUSY_ERROR(ret_code, busy_ret_code);
}
EXPORT_SYMBOL(tpm_updt_ipv4_mc_stream);

/*******************************************************************************
* tpm_del_ipv4_mc_stream()
*
* DESCRIPTION:      Deletes an existing IPv4 MC stream.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* stream_num         - MC stream number.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_del_ipv4_mc_stream(uint32_t owner_id,
					uint32_t stream_num)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_del_ipv4_mc_stream(owner_id, stream_num);
	return ret_code;
}
EXPORT_SYMBOL(tpm_del_ipv4_mc_stream);

/*******************************************************************************
* tpm_add_ipv6_mc_stream()
*
* DESCRIPTION:      Creates a new ipv6 MC stream.
*                   It is APIs caller responsibility to maintain the correct number of
*                   each stream number.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* stream_num         - MC stream number.
* vid                - VLAN ID (0-4095). If set to 4096 - stream is untagged.
*                      If set to 0xFFFF - do not care.
* ipv6_src_add       - ipv6 source IP address in network order.
* ipv6_dst_add       - ipv6 destination IP address in network order.
* ignore_ipv6_src    - when set to 1 - the IP source is not part of the key.
* dest_port_bm       - bitmap which includes all destination UNI ports.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_add_ipv6_mc_stream(uint32_t owner_id,
					uint32_t stream_num,
					tpm_mc_igmp_mode_t igmp_mode,
					uint8_t mc_stream_pppoe,
					uint16_t vid,
					uint8_t ipv6_src_add[16],
					uint8_t ipv6_dst_add[16],
					uint8_t ignore_ipv6_src,
					tpm_trg_port_type_t dest_port_bm)
{
	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_type Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_IPV6_MC, stream_num);
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_proc_add_ipv6_mc_stream(owner_id, stream_num, igmp_mode, mc_stream_pppoe,
						vid, ipv6_src_add, ipv6_dst_add, ignore_ipv6_src,
						TPM_INVALID_QUEUE, dest_port_bm);


	busy_ret_code = tpm_proc_api_busy_done(TPM_API_IPV6_MC, stream_num);

	RET_BUSY_ERROR(ret_code, busy_ret_code);

}
EXPORT_SYMBOL(tpm_add_ipv6_mc_stream);

/*******************************************************************************
* tpm_add_ipv6_mc_stream_set_queue()
*
* DESCRIPTION:      Creates a new ipv6 MC stream with specified destination queue number.
*                   It is APIs caller responsibility to maintain the correct number of
*                   each stream number.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* stream_num         - MC stream number.
* vid                - VLAN ID (0-4095). If set to 4096 - stream is untagged.
*                      If set to 0xFFFF - do not care.
* ipv6_src_add       - ipv6 source IP address in network order.
* ipv6_dst_add       - ipv6 destination IP address in network order.
* ignore_ipv6_src    - when set to 1 - the IP source is not part of the key.
* dest_queue          - destination queue number.
* dest_port_bm       - bitmap which includes all destination UNI ports.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_add_ipv6_mc_stream_set_queue(uint32_t owner_id,
					uint32_t stream_num,
					tpm_mc_igmp_mode_t igmp_mode,
					uint8_t mc_stream_pppoe,
					uint16_t vid,
					uint8_t ipv6_src_add[16],
					uint8_t ipv6_dst_add[16],
					uint8_t ignore_ipv6_src,
					uint16_t dest_queue,
					tpm_trg_port_type_t dest_port_bm)
{
	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_type Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_IPV6_MC, stream_num);
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_proc_add_ipv6_mc_stream(owner_id, stream_num, igmp_mode, mc_stream_pppoe,
						vid, ipv6_src_add, ipv6_dst_add, ignore_ipv6_src,
						dest_queue, dest_port_bm);


	busy_ret_code = tpm_proc_api_busy_done(TPM_API_IPV6_MC, stream_num);

	RET_BUSY_ERROR(ret_code, busy_ret_code);

}
EXPORT_SYMBOL(tpm_add_ipv6_mc_stream_set_queue);

/*******************************************************************************
* tpm_updt_ipv6_mc_stream()
*
* DESCRIPTION:      Updates an existing ipv6 MC stream.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* stream_num         - MC stream number.
* dest_port_bm       - bitmap which includes all destination UNI ports.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_updt_ipv6_mc_stream(uint32_t owner_id,
					 uint32_t stream_num,
					 tpm_trg_port_type_t dest_port_bm)
{	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_type Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_IPV6_MC, stream_num);
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_proc_updt_ipv6_mc_stream(owner_id, stream_num, dest_port_bm);

	busy_ret_code = tpm_proc_api_busy_done(TPM_API_IPV6_MC, stream_num);

	RET_BUSY_ERROR(ret_code, busy_ret_code);
}
EXPORT_SYMBOL(tpm_updt_ipv6_mc_stream);

/*******************************************************************************
* tpm_del_ipv6_mc_stream()
*
* DESCRIPTION:      Deletes an existing ipv6 MC stream.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* stream_num         - MC stream number.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_del_ipv6_mc_stream(uint32_t owner_id,
					uint32_t stream_num)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_del_ipv6_mc_stream(owner_id, stream_num);
	return ret_code;
}
EXPORT_SYMBOL(tpm_del_ipv6_mc_stream);

/******************************************************************************/
/********************************** Management protocol APIs ******************/
/******************************************************************************/

/*******************************************************************************
* tpm_omci_add_channel()
*
* DESCRIPTION:      Establishes a communication channel for the OMCI management protocol.
*                   The API sets the gemportid, the Rx input queue in the CPU, and the
*                   Tx T-CONT and queue parameters, which are configured in the driver.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* gem_port           - for OMCI Rx frames - the gem port wherefrom the OMCI frames are received.
* cpu_rx_queue       - for OMCI Rx frames - the CPU rx queue number.
* tcont_num          - for OMCI Tx frames - the TCONT number where to send the OMCI frames.
* cpu_tx_queue       - for OMCI Tx frames - the CPU tx queue number.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_omci_add_channel(uint32_t owner_id,
				      tpm_gem_port_key_t gem_port,
				      uint32_t cpu_rx_queue,
				      tpm_trg_port_type_t tcont_num,
				      uint32_t cpu_tx_queue)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_omci_add_channel(owner_id, gem_port, cpu_rx_queue, tcont_num, cpu_tx_queue);
	return ret_code;
}
EXPORT_SYMBOL(tpm_omci_add_channel);

/*******************************************************************************
* tpm_omci_del_channel()
*
* DESCRIPTION:      Deletes an existing communication channel for the OMCI management protocol.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_omci_del_channel(uint32_t owner_id)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_omci_del_channel(owner_id);
	return ret_code;
}
EXPORT_SYMBOL(tpm_omci_del_channel);

/*******************************************************************************
* tpm_oam_epon_add_channel()
*
* DESCRIPTION:      Establishes a communication channel for the OAM EPON management protocol.
*                   The API sets the Rx input queue in the CPU, and the
*                   Tx T-CONT and queue parameters, which are configured in the driver.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* cpu_rx_queue       - for OAM (EPON) Rx frames - the CPU rx queue number.
* llid_num           - for OAM (EPON)Tx frames - the LLID number where to send the OMCI frames.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_oam_epon_add_channel(uint32_t owner_id,
					  uint32_t cpu_rx_queue,
					  tpm_trg_port_type_t llid_num)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_oam_epon_add_channel(owner_id, cpu_rx_queue, llid_num);
	return ret_code;
}
EXPORT_SYMBOL(tpm_oam_epon_add_channel);

/*******************************************************************************
* tpm_loop_detect_add_channel()
*
* DESCRIPTION:      Establishes a communication channel for loop detection management protocol.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_loop_detect_add_channel(uint32_t owner_id, tpm_ether_type_key_t ety)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_loop_detect_add_channel(owner_id, ety);
	return ret_code;
}
EXPORT_SYMBOL(tpm_loop_detect_add_channel);

/*******************************************************************************
* tpm_loop_detect_del_channel()
*
* DESCRIPTION:      remove a communication channel for loop detection management protocol.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_loop_detect_del_channel(uint32_t owner_id)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_loop_detect_del_channel(owner_id);
	return ret_code;
}
EXPORT_SYMBOL(tpm_loop_detect_del_channel);

/*******************************************************************************
* tpm_oam_loopback_add_channel()
*
* DESCRIPTION:      Establishes a communication channel for the OAM remote loopback.
*
*
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_oam_loopback_add_channel(uint32_t owner_id)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_oam_loopback_add_channel(owner_id);
	return ret_code;
}
EXPORT_SYMBOL(tpm_oam_loopback_add_channel);

/*******************************************************************************
* tpm_oam_loopback_del_channel()
*
* DESCRIPTION:      Del a communication channel for the OAM remote loopback .
*
*
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_oam_loopback_del_channel(uint32_t owner_id)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_oam_loopback_del_channel(owner_id);
	return ret_code;
}
EXPORT_SYMBOL(tpm_oam_loopback_del_channel);

/*******************************************************************************
* tpm_oam_epon_del_channel()
*
* DESCRIPTION:      Deletes an existing communication channel for the OAM EPON management protocol.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_oam_epon_del_channel(uint32_t owner_id)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_oam_epon_del_channel(owner_id);
	return ret_code;
}
EXPORT_SYMBOL(tpm_oam_epon_del_channel);

/*******************************************************************************
* tpm_get_next_valid_rule()
*
* DESCRIPTION:      General purpose API to retrieve the internal configuration of an existing ACL.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* direction          - direction - UpStream or DownStream - TPM_DOWNSTREAM = 0 and TPM_UPSTREAM = 1.
* current_index      - the entry index in the section (rule_num/stream_num).
*                      In case it is (-1) - the get next function will point to the first ruleof the section.
* rule_type          - type of the rule to retrieve - indicates the api group where to point.
*
* OUTPUTS:
* next_index         - returns the first following index (rule_num/stream_num) after the index in
*                      the current_index parameter. It is invalid, if there is no next_index.
* rule_idx           - Unique rule identification number. Equals to stream_num for functions that take a stream_num.
* tpm_rule           - points to a structure holding the information of a single rule,
*                      of the type specified in the rule_type param.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_get_next_valid_rule(uint32_t owner_id,
					 int32_t current_index,
					 tpm_api_type_t rule_type,
					 int32_t *next_index,
					 uint32_t *rule_idx,
					 tpm_rule_entry_t *tpm_rule)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_get_next_valid_rule(owner_id, current_index, rule_type, next_index, rule_idx, tpm_rule);
	return ret_code;
}
EXPORT_SYMBOL(tpm_get_next_valid_rule);

/******************************************************************************/
/********************************** Configuration retrieval APIs **************/
/******************************************************************************/

/*******************************************************************************
* tpm_omci_get_channel()
*
* DESCRIPTION:      Retrieves the OMCI management protocol channel information.
*
* INPUTS:
*  None.
*
* OUTPUTS:
* is_valid           - indicates that the OMCI channel is valid or not.
* gem_port           - for OMCI Rx frames - the gem port wherefrom the OMCI frames are received.
* cpu_rx_queue       - for OMCI Rx frames - the CPU rx queue number.
* tcont_num          - for OMCI Tx frames - the TCONT number where to send the OMCI frames.
* cpu_tx_queue       - for OMCI Tx frames - the CPU tx queue number.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_omci_get_channel(uint32_t *is_valid,
				      tpm_gem_port_key_t *gem_port,
				      uint32_t *cpu_rx_queue,
				      tpm_trg_port_type_t *tcont_num,
				      uint32_t *cpu_tx_queue)
{
	if (!is_valid || !gem_port || !cpu_rx_queue || !tcont_num || !cpu_tx_queue)
		return ERR_NULL_POINTER;

	tpm_db_omci_channel_get(is_valid, gem_port, cpu_rx_queue, cpu_tx_queue, tcont_num);
	return TPM_RC_OK;
}
EXPORT_SYMBOL(tpm_omci_get_channel);

/*******************************************************************************
* tpm_set_port_igmp_frwd_mode()
*
* DESCRIPTION:      Set the IGMP status of a UNI port or WAN port.
*
* INPUTS:
* src_port          - source port to set
* state             - how to process IGMP packets
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_set_port_igmp_frwd_mode(tpm_src_port_type_t src_port,
					     tpm_igmp_frwd_mode_t mode)
{
	tpm_error_code_t ret_code = TPM_RC_OK;
	tpm_igmp_frwd_mode_t _mode;

	tpm_db_igmp_get_port_frwd_mode(src_port, &_mode);

	if (_mode != mode) {
		if (TPM_SRC_PORT_WAN == src_port)
			ret_code = tpm_proc_add_wan_port_igmp_rule(mode);
		else
			ret_code = tpm_proc_add_uni_port_igmp_rule(src_port, mode);

		tpm_db_igmp_set_port_frwd_mode(src_port, mode);
	}

	return ret_code;
}
EXPORT_SYMBOL(tpm_set_port_igmp_frwd_mode);

/*******************************************************************************
* tpm_get_port_igmp_frwd_mode()
*
* DESCRIPTION:      Get the IGMP status of a UNI port or WAN port.
*
* INPUTS:
* src_port          - source port to get
*
* OUTPUTS:
* state             - how to process IGMP packets
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_get_port_igmp_frwd_mode(tpm_src_port_type_t src_port,
					     tpm_igmp_frwd_mode_t *mode)
{
	if (!mode)
		return ERR_NULL_POINTER;

	tpm_db_igmp_get_port_frwd_mode(src_port, mode);

	return TPM_RC_OK;
}
EXPORT_SYMBOL(tpm_get_port_igmp_frwd_mode);

/*******************************************************************************
* tpm_set_igmp_cpu_rx_queue()
*
* DESCRIPTION:      Set queue number which IGMP packets are forwarded to.
*
* INPUTS:
* queue             - queue number
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_set_igmp_cpu_rx_queue(uint32_t queue)
{
	tpm_error_code_t ret_code = TPM_RC_OK;
	uint32_t _queue;
	tpm_igmp_frwd_mode_t _mode;
	tpm_src_port_type_t src_port;

	tpm_db_igmp_get_cpu_queue(&_queue);

	if (_queue != queue) {
		tpm_db_igmp_set_cpu_queue(queue);

		for (src_port = TPM_SRC_PORT_UNI_0; src_port <= TPM_SRC_PORT_WAN; src_port++) {

			/* virt uni do not need to add PNC entry for IGMP */
			if (  (src_port == TPM_SRC_PORT_UNI_VIRT)
				|| (tpm_db_eth_port_switch_port_get(src_port) == TPM_DB_ERR_PORT_NUM)){
				TPM_OS_INFO(TPM_TPM_LOG_MOD, " src_port %d\n", src_port);
				continue;
			}
			tpm_db_igmp_get_port_frwd_mode(src_port, &_mode);

			if (TPM_IGMP_FRWD_MODE_SNOOPING == _mode) {

				if (TPM_SRC_PORT_WAN == src_port)
					ret_code |= tpm_proc_add_wan_port_igmp_rule(_mode);
				else
					ret_code |= tpm_proc_add_uni_port_igmp_rule(src_port, _mode);
			}
		}
	}

	return ret_code;
}
EXPORT_SYMBOL(tpm_set_igmp_cpu_rx_queue);

/*******************************************************************************
* tpm_get_igmp_cpu_rx_queue()
*
* DESCRIPTION:      Get queue number which IGMP packets are forwarded to.
*
* INPUTS:
* queue             - queue number
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_get_igmp_cpu_rx_queue(uint32_t *queue)
{
	if (!queue)
		return ERR_NULL_POINTER;

	tpm_db_igmp_get_cpu_queue(queue);

	return TPM_RC_OK;
}
EXPORT_SYMBOL(tpm_get_igmp_cpu_rx_queue);

/*******************************************************************************
* tpm_set_igmp_proxy_sa_mac()
*
* DESCRIPTION:      Set source mac address replaced for igmp proxy.
*
* INPUTS:
* sa_mac          - source mac
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_set_igmp_proxy_sa_mac(uint8_t *sa_mac)
{
	tpm_db_set_mc_igmp_proxy_sa_mac(sa_mac);

	return TPM_RC_OK;
}
EXPORT_SYMBOL(tpm_set_igmp_proxy_sa_mac);

/*******************************************************************************
* tpm_get_igmp_proxy_sa_mac()
*
* DESCRIPTION:      Get source mac address replaced for igmp proxy.
*
* INPUTS:
* sa_mac          - source mac
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_get_igmp_proxy_sa_mac(uint8_t *sa_mac)
{
	uint8_t mac[6], mac_valid;

	tpm_db_get_mc_igmp_proxy_sa_mac(mac, &mac_valid);
	if (mac_valid)
		memcpy(sa_mac, mac, 6 * sizeof(uint8_t));
	else
		return ERR_GENERAL;

	return TPM_RC_OK;
}
EXPORT_SYMBOL(tpm_get_igmp_proxy_sa_mac);

/*******************************************************************************
* tpm_oam_epon_get_channel()
*
* DESCRIPTION:      Retrieves OAM EPON management protocol channel information.
*
* INPUTS:
*  None.
*
* OUTPUTS:
* is_valid           -  indicates that the OMCI channel is valid or not.
* cpu_rx_queue       - for OAM (EPON) Rx frames - the CPU rx queue number.
* llid_num           - for OAM (EPON)Tx frames - the LLID number where to send the OMCI frames.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_oam_epon_get_channel(uint32_t *is_valid,
					  uint32_t *cpu_rx_queue,
					  tpm_trg_port_type_t *llid_num)
{
	uint32_t dummy_cpu_tx_queue;

	if (!is_valid || !cpu_rx_queue || !llid_num)
		return ERR_NULL_POINTER;

	tpm_db_oam_channel_get(is_valid, cpu_rx_queue, &dummy_cpu_tx_queue, llid_num);
	return TPM_RC_OK;
}
EXPORT_SYMBOL(tpm_oam_epon_get_channel);

/*******************************************************************************
* tpm_get_api_ownership()
*
* DESCRIPTION:      Retrieves OAM EPON management protocol channel information.
*
* INPUTS:
* api_type          - Specifies the API group whom owner_id is requested.
*
* OUTPUTS:
* owner_id           - specifies the ownerId of the application group
*
* RETURNS:
* On success, the function returns API_OWNERSHIP_SUCCESS.
* On error:     API_TYPE_UNKNOWN  when illegal API group
*               API_OWNERSHIP_ERROR  other errors.
*
* COMMENTS:
*
*******************************************************************************/
tpm_api_ownership_error_t tpm_get_api_ownership(uint32_t *owner_id,
						tpm_api_type_t api_type)
{
	printk(KERN_WARNING "\n  <<tpm_oam_epon_get_channel>> NOT implemented. \n");
	owner_id = 0;
	return API_OWNERSHIP_SUCCESS;
}
EXPORT_SYMBOL(tpm_get_api_ownership);

/******************************************************************************/
/************************** Packet modification APIs **************************/
/******************************************************************************/

/*******************************************************************************
* tpm_mod_entry_set()
*
* DESCRIPTION: The API sets a complete Modification table entry
*
* INPUTS:   trg_port    - target port
*           mod_bm      - set of flags described which fields in the packet
*                         to be changed
*           int_mod_bm  - set of internal flags
*           mod_data    - modification entry data
*
* OUTPUTS:
*           mod_entry   - start entry number which has been set for this
*                         modification
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_mod_entry_set(tpm_trg_port_type_t trg_port,
				   tpm_pkt_mod_bm_t mod_bm,
				   tpm_pkt_mod_int_bm_t int_mod_bm,
				   tpm_pkt_mod_t *mod_data,
				   uint32_t *mod_entry)
{
	tpm_error_code_t rc = TPM_RC_OK;
	tpm_gmacs_enum_t gmac_port;

	/*printk("tpm_mod_entry_set: trg_port = %d\n", trg_port); */

	tpm_proc_trg_port_gmac_map(trg_port, &gmac_port);
	if (gmac_port == TPM_INVALID_GMAC) {
		printk(KERN_ERR "ERROR: PKT modification not possible on this target port(%d) \n", trg_port);
		rc = ERR_ACTION_INVALID;
	} else
		rc = tpm_mod2_entry_set(TPM_MOD_OWNER_CPU, gmac_port, mod_bm, int_mod_bm, mod_data, mod_entry);

	return rc;
}
EXPORT_SYMBOL(tpm_mod_entry_set);

/*******************************************************************************
* tpm_mod_entry_get()
*
* DESCRIPTION: The API gets Modification table entries
*
* INPUTS:   trg_port    - target port
*           mod_entry   - start entry number (in the "jump" area)
*
* OUTPUTS:
*           valid_cmds  - number of valid commands in the chunk
*                             0 - there is no requested data
*           pnc_ref     - Number of references from Pnc entries to this modification rule
*           rule        - modification rule (set of modification entries)
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_mod_entry_get(tpm_trg_port_type_t trg_port,
				   uint32_t mod_entry,
				   uint16_t *valid_cmds,
				   uint16_t *pnc_ref,
				   tpm_mod_rule_t *rule)
{
	tpm_gmacs_enum_t gmac_port;
	tpm_error_code_t rc = TPM_RC_OK;

	/*printk("tpm_mod_entry_get: trg_port = %d mod_entry = %d\n", trg_port, mod_entry); */

	tpm_proc_trg_port_gmac_map(trg_port, &gmac_port);
	if (gmac_port == TPM_INVALID_GMAC) {
		printk(KERN_ERR "ERROR: PKT modification not possible on this target port(%d) \n", trg_port);
		rc = ERR_ACTION_INVALID;
	} else {
		*pnc_ref = 0;

		if (tpm_mod2_entry_get(gmac_port, mod_entry, valid_cmds, rule) != TPM_OK)
			rc = ERR_GENERAL;
	}

	return rc;
}
EXPORT_SYMBOL(tpm_mod_entry_get);

/*******************************************************************************
* tpm_mod_entry_del()
*
* DESCRIPTION: The API invalidates a modification entry chunk
*
* INPUTS:   trg_port    - target port
*           mod_entry   - start entry number (in the "jump" area)
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_mod_entry_del(tpm_trg_port_type_t trg_port,
				   uint32_t mod_entry)
{
	tpm_error_code_t rc = TPM_RC_OK;
	tpm_gmacs_enum_t gmac_port;

	/*printk("tpm_mod_entry_del: trg_port = %d mod_entry = %d\n", trg_port, mod_entry); */

	tpm_proc_trg_port_gmac_map(trg_port, &gmac_port);
	if (gmac_port == TPM_INVALID_GMAC) {
		printk(KERN_ERR "ERROR: PKT modification not possible on this target port(%d) \n", trg_port);
		rc = ERR_ACTION_INVALID;
	} else
		rc = tpm_mod2_entry_del(TPM_MOD_OWNER_CPU, gmac_port, mod_entry);

	return rc;
}
EXPORT_SYMBOL(tpm_mod_entry_del);

/*******************************************************************************
* tpm_mod_mac_inv()
*
* DESCRIPTION: The API resets Modification table for a specified port
*
* INPUTS:   trg_port    - target port
*
* OUTPUTS:
*           None
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_mod_mac_inv(tpm_trg_port_type_t trg_port)
{
	tpm_error_code_t rc = TPM_RC_OK;
	tpm_gmacs_enum_t gmac_port;

	/*printk("tpm_mod_mac_inv: trg_port = %d\n", trg_port); */

	tpm_proc_trg_port_gmac_map(trg_port, &gmac_port);
	if (gmac_port == TPM_INVALID_GMAC) {
		printk(KERN_ERR "ERROR: PKT modification not possible on this target port(%d) \n", trg_port);
		rc = ERR_ACTION_INVALID;
	} else {
		if (tpm_db_mod2_inv_mac_entries(gmac_port) != TPM_OK) {
			TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to reset database (%d) \n", gmac_port);
			rc = TPM_FAIL;
		}

		if (tpm_mod2_mac_inv(gmac_port) != TPM_OK) {
			TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to reset hardware (%d) \n", gmac_port);
			rc = TPM_FAIL;
		}
	}

	return rc;
}
EXPORT_SYMBOL(tpm_mod_mac_inv);

/*******************************************************************************
* tpm_rx_igmp_frame()
*
* DESCRIPTION: The API rx IGMP frames from either LAN or WAN side,
*              it will be blocked until a packet arrived.
*
* INPUTS:   owner_id    - owner id
*           buf         - Message buffer allocated by user
*           len         - Max len of this buffer
*
* OUTPUTS:
*           llid        - source LLID index in case this frame comes from WAN side
*           src_port    - source UNI port in case this frame comes from LAN side
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_rx_igmp_frame(uint32_t owner_id,
				   uint16_t *llid,
				   tpm_src_port_type_t *src_port,
				   uint8_t *buf,
				   uint32_t *len)
{
	static struct socket *sock = NULL;
	struct msghdr msg;
	struct iovec iov;
	int32_t length, error;
	mm_segment_t oldfs;
	static uint8_t temp_buf[MAX_FRAME_SIZE];

	if (NULL == sock) {
		/* First create a socket */
		error = sock_create(AF_PACKET, SOCK_RAW, htons(0xa000), &sock);
		if (error < 0) {
			printk(KERN_ERR "%s: sock_create(AF_PACKET, SOCK_RAW, htons(0xbaba)) failed. errno = %d\n",
			       __func__, error);
			return ERR_GENERAL;
		}
	}

	/* Set the msghdr structure */
	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;

	/* Set the iovec structure */
	iov.iov_base = (void *)&temp_buf[0];
	iov.iov_len = (size_t) MAX_FRAME_SIZE;

	/* Recieve the message */
	oldfs = get_fs();
	set_fs(KERNEL_DS);
	length = sock_recvmsg(sock, &msg, MAX_FRAME_SIZE, msg.msg_flags);
	set_fs(oldfs);

	if (length <= 2)
		return ERR_GENERAL;
	else {
		*len = length - 2;

		/* Set by MV_CUST_EXT IGMP RX */
		if (2 == temp_buf[0]) {
			*llid = (temp_buf[1] & 0x0F) - 1;
			*src_port = TPM_SRC_PORT_WAN;
		} else
			*src_port = tpm_db_phy_convert_port_index(temp_buf[1]);
			/* How to support GMAC1? */

		memcpy(buf, &(temp_buf[2]), length - 2);
	}

	return TPM_RC_OK;
}
EXPORT_SYMBOL(tpm_rx_igmp_frame);

/*******************************************************************************
* tpm_tx_igmp_frame()
*
* DESCRIPTION: The API tx IGMP frames to either LAN or WAN side
*
* INPUTS:   owner_id    - owner id
*           tgt_port    - target port, LLID0-7/TCONT0-7/UNI0-3
*           tgt_queue   - target queue, 0-7
*           gem_port    - target gem port ID
*           buf         - Message buffer allocated by user
*           len         - Len of this buffer
*
* OUTPUTS:
*           NONE
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_tx_igmp_frame(uint32_t owner_id,
				   tpm_trg_port_type_t tgt_port,
				   uint8_t tgt_queue,
				   uint16_t gem_port,
				   uint8_t *buf,
				   uint32_t len)
{
	static struct socket *sock = NULL;
	struct msghdr msg;
	struct iovec iov;
	int32_t length, error;
	mm_segment_t oldfs;
	static uint8_t temp_buf[MAX_FRAME_SIZE];

	if (NULL == sock) {
		/* First create a socket */
		error = sock_create(AF_PACKET, SOCK_RAW, htons(0xa000), &sock);
		if (error < 0) {
			printk(KERN_ERR "%s: sock_create(AF_PACKET, SOCK_RAW, htons(0xbaba)) failed. errno = %d\n",
			       __func__, error);
			return ERR_GENERAL;
		}
	}

	/* Set the msghdr structure */
	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;

	/* Set the iovec structure */
	iov.iov_base = (void *)&temp_buf[0];
	iov.iov_len = (size_t) (len + 4);

	memcpy(&temp_buf[4], buf, len);
	/* BYTE 0 to 3 will be used by MV_CUST_EXT */
	temp_buf[0] = tgt_port;
	temp_buf[1] = tgt_queue;
	temp_buf[2] = (gem_port & 0xff00) >> 8;
	temp_buf[3] = gem_port & 0xff;

	if ((tgt_port & TPM_TRG_TCONT_0) || (tgt_port & TPM_TRG_TCONT_1) ||
	    (tgt_port & TPM_TRG_TCONT_2) || (tgt_port & TPM_TRG_TCONT_3) ||
	    (tgt_port & TPM_TRG_TCONT_4) || (tgt_port & TPM_TRG_TCONT_5) ||
	    (tgt_port & TPM_TRG_TCONT_6) || (tgt_port & TPM_TRG_TCONT_7)) {
		struct net_device *dev = NULL;
		struct sockaddr sockAddr;
		struct sockaddr_ll *sockAddr_ll = (struct sockaddr_ll *)&sockAddr;

		/*strcpy(ifReq.ifr_name, "pon0"); */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
		dev = dev_get_by_name("pon0");
#else
		dev = dev_get_by_name(sock_net(sock->sk), "pon0");
#endif
		if (dev == NULL) {
			printk(KERN_ERR "%s: pon0 interface not found \n", __func__);
			return ERR_GENERAL;
		}
		sockAddr_ll->sll_ifindex = dev->ifindex;
		sockAddr_ll->sll_family = AF_PACKET;
		sockAddr_ll->sll_protocol = htons(0xa000);

		oldfs = get_fs();
		set_fs(KERNEL_DS);
		if (sock->ops->bind(sock, &sockAddr, sizeof(struct sockaddr_ll)) < 0) {
			set_fs(oldfs);
			printk(KERN_ERR "%s: sock->ops->ioctl(sock,SIOCGIFINDEX,&ifReq) failed.\n", __func__);
			return ERR_GENERAL;
		}
		set_fs(oldfs);

		if (tgt_port & TPM_TRG_TCONT_0) {
			temp_buf[0] = 0;
			oldfs = get_fs();
			set_fs(KERNEL_DS);
			length = sock_sendmsg(sock, &msg, len + 4);
			set_fs(oldfs);
		}
		if (tgt_port & TPM_TRG_TCONT_1) {
			temp_buf[0] = 1;
			oldfs = get_fs();
			set_fs(KERNEL_DS);
			length = sock_sendmsg(sock, &msg, len + 4);
			set_fs(oldfs);
		}
		if (tgt_port & TPM_TRG_TCONT_2) {
			temp_buf[0] = 2;
			oldfs = get_fs();
			set_fs(KERNEL_DS);
			length = sock_sendmsg(sock, &msg, len + 4);
			set_fs(oldfs);
		}
		if (tgt_port & TPM_TRG_TCONT_3) {
			temp_buf[0] = 3;
			oldfs = get_fs();
			set_fs(KERNEL_DS);
			length = sock_sendmsg(sock, &msg, len + 4);
			set_fs(oldfs);
		}
		if (tgt_port & TPM_TRG_TCONT_4) {
			temp_buf[0] = 4;
			oldfs = get_fs();
			set_fs(KERNEL_DS);
			length = sock_sendmsg(sock, &msg, len + 4);
			set_fs(oldfs);
		}
		if (tgt_port & TPM_TRG_TCONT_5) {
			temp_buf[0] = 5;
			oldfs = get_fs();
			set_fs(KERNEL_DS);
			length = sock_sendmsg(sock, &msg, len + 4);
			set_fs(oldfs);
		}
		if (tgt_port & TPM_TRG_TCONT_6) {
			temp_buf[0] = 6;
			oldfs = get_fs();
			set_fs(KERNEL_DS);
			length = sock_sendmsg(sock, &msg, len + 4);
			set_fs(oldfs);
		}
		if (tgt_port & TPM_TRG_TCONT_7) {
			temp_buf[0] = 7;
			oldfs = get_fs();
			set_fs(KERNEL_DS);
			length = sock_sendmsg(sock, &msg, len + 4);
			set_fs(oldfs);
		}
	} else {
		struct net_device *dev = NULL;
		struct sockaddr sockAddr;
		struct sockaddr_ll *sockAddr_ll = (struct sockaddr_ll *)&sockAddr;

		/*strcpy(ifReq.ifr_name, "pon0"); */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
		dev = dev_get_by_name("eth0");
#else
		dev = dev_get_by_name(sock_net(sock->sk), "eth0");
#endif
		if (dev == NULL) {
			printk(KERN_ERR "%s: eth0 interface not found \n", __func__);
			return ERR_GENERAL;
		}
		sockAddr_ll->sll_ifindex = dev->ifindex;
		sockAddr_ll->sll_family = AF_PACKET;
		sockAddr_ll->sll_protocol = htons(0xa000);

		oldfs = get_fs();
		set_fs(KERNEL_DS);
		if (sock->ops->bind(sock, &sockAddr, sizeof(struct sockaddr_ll)) < 0) {
			set_fs(oldfs);
			printk(KERN_ERR "%s: sock->ops->ioctl(sock,SIOCGIFINDEX,&ifReq) failed.\n", __func__);
			return ERR_GENERAL;
		}
		set_fs(oldfs);

		/* Always go to queue 7 */
		temp_buf[1] = 7;
		if (TPM_TRG_PORT_UNI_ANY & tgt_port)
			temp_buf[3] = 0;
		else
			temp_buf[3] = tpm_db_trg_port_switch_port_get(tgt_port);

		oldfs = get_fs();
		set_fs(KERNEL_DS);
		length = sock_sendmsg(sock, &msg, len + 4);
		set_fs(oldfs);

		/* How to support GMAC1? */
	}

	return TPM_RC_OK;
}
EXPORT_SYMBOL(tpm_tx_igmp_frame);

/*******************************************************************************
* tpm_add_cpu_wan_loopback()
*
* DESCRIPTION: The API add CPU egress loopback modification and PnC rules for
*              specific Tcont/queue/gem_port
*
* INPUTS:   owner_id    - APP owner id  should be used for all API calls.
*           pkt_frwd    - packet forwarding info: target tcont/LLID number, 0-7
*                         target queue, 0-7, target gem port ID
*
*
* OUTPUTS:
*           mod_idx     - Returned by HW modification, to fill Tx description
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_add_cpu_wan_loopback(uint32_t owner_id,
					  tpm_pkt_frwd_t *pkt_frwd,
					  uint32_t *mod_idx)
{
	tpm_error_code_t ret_code;
	ret_code = tpm_proc_add_cpu_wan_loopback(owner_id, pkt_frwd, mod_idx);
	return (ret_code);
}
EXPORT_SYMBOL(tpm_add_cpu_wan_loopback);

/*******************************************************************************
* tpm_del_cpu_wan_loopback()
*
* DESCRIPTION: The API delete CPU egress loopback modification and PnC rules for
*              specific Tcont/queue/gem_port
*
* INPUTS:   owner_id    - APP owner id  should be used for all API calls.
*           pkt_frwd    - packet forwarding info: target tcont/LLID number, 0-7
*                         target queue, 0-7, target gem port ID
*
* OUTPUTS:
*           NONE
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_del_cpu_wan_loopback(uint32_t owner_id,
					  tpm_pkt_frwd_t *pkt_frwd)
{
	tpm_error_code_t ret_code;
	ret_code = tpm_proc_del_cpu_wan_loopback(owner_id, pkt_frwd);
	return (ret_code);
}
EXPORT_SYMBOL(tpm_del_cpu_wan_loopback);

/*******************************************************************************
* tpm_get_pnc_lu_entry()
*
* DESCRIPTION: The API get least used PnC rule by in specific PnC range
*
* INPUTS:   owner_id        - APP owner id  should be used for all API calls
*           api_type       - TPM API group type
*           lu_num          - The required number of least used PnC entries
*           lu_reset        - Whether need to reset counter after read LU
*
*
* OUTPUTS:
*           valid_num       - The valid number of least used PnC entries
*           count_array     - The least used PnC entry index  and hit counter array
*           unrelated_num   - The unlelated number of least used PnC entries
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_get_pnc_lu_entry(uint32_t owner_id,
				      tpm_api_type_t api_type,
				      uint16_t lu_num,
				      uint8_t lu_reset,
				      uint16_t *valid_num,
				      tpm_api_entry_count_t *count_array,
				      uint16_t *unrelated_num)
{
	tpm_error_code_t ret_code;
	ret_code = tpm_count_get_pnc_lu_entry(owner_id, api_type, lu_num, lu_reset, valid_num, count_array,
					      unrelated_num);
	return (ret_code);
}
EXPORT_SYMBOL(tpm_get_pnc_lu_entry);
/*******************************************************************************
* tpm_get_pnc_all_hit_counters
*
* DESCRIPTION: The API returns all PnC hit counters per API type lower than a given threshold
*
* INPUTS:   owner_id            - APP owner id  should be used for all API calls
*           api_type            - TPM API group type
*           high_thresh_pkts    - High threashold watermark, counters lower than will be returned
*           counters_reset      - Reset API group type counters after read (0-false, 1-true)
*           valid_counters      - The count_array size (entry number, not byte count)
*
* OUTPUTS:  valid_counters      - The valid number of entries copied to count_array
*           count_array         - The PnC entries for the API type lower than high_thresh_pkts
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_get_pnc_all_hit_counters(uint32_t owner_id,
					      tpm_api_type_t api_type,
					      uint32_t high_thresh_pkts,
					      uint8_t counters_reset,
					      uint16_t *valid_counters,
					      tpm_api_entry_count_t *count_array)
{
	tpm_error_code_t ret_code;
	ret_code = tpm_count_get_pnc_all_hit_counters(owner_id, api_type, high_thresh_pkts,
						      counters_reset, valid_counters, count_array);
	return(ret_code);
}
/*******************************************************************************
* tpm_set_pnc_counter_mask()
*
* DESCRIPTION: The API set the least used scanner ability, to mask or enable it
*
* INPUTS:   owner_id     - APP owner id  should be used for all API calls
*           api_type    - TPM API group type
*           rule_idx     - The PnC rule index returned when created PnC rules
*           lu_rule_mask - The least used scanner mask, 1: maskscanner ,
*                          0:enable scanner
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_set_pnc_counter_mask(uint32_t owner_id,
					  tpm_api_type_t api_type,
					  uint32_t rule_idx,
					  uint32_t lu_rule_mask)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_count_set_pnc_counter_mask(owner_id, api_type, rule_idx, lu_rule_mask);
	return (ret_code);
}
EXPORT_SYMBOL(tpm_set_pnc_counter_mask);

/*******************************************************************************
* tpm_get_pnc_hit_count()
*
* DESCRIPTION: The API get the hit counter according to rule_idx
*
* INPUTS:   owner_id     - APP owner id  should be used for all API calls
*           api_type    - TPM API group type
*           rule_idx     - The PnC rule index returned when created PnC rules
*           hit_reset    - Should the API reset the hit counters after after reading
*
* OUTPUTS:
*           rule_idx     - The hitted times of specific PnC rule
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_get_pnc_hit_count(uint32_t owner_id,
				       tpm_api_type_t api_type,
				       uint32_t rule_idx,
				       uint8_t  hit_reset,
				       uint32_t *hit_count)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_count_get_pnc_hit_count(owner_id, api_type, rule_idx, hit_reset, hit_count);
	return (ret_code);
}
EXPORT_SYMBOL(tpm_get_pnc_hit_count);

/*******************************************************************************
* tpm_set_pnc_lu_threshold()
*
* DESCRIPTION: The API set the theshold packets number for least used scanner
*
* INPUTS:   owner_id       - APP owner id  should be used for all API calls
*           api_type      - TPM API group type
*           lu_thresh_pkts - The theshold number(hitted packets number) to be used
*                            for least used scanner
*
* OUTPUTS:
*            None
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_set_pnc_lu_threshold(uint32_t owner_id,
					  tpm_api_type_t api_type,
					  uint32_t lu_thresh_pkts)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_count_set_pnc_lu_threshold(owner_id, api_type, lu_thresh_pkts);
	return (ret_code);
}
EXPORT_SYMBOL(tpm_set_pnc_lu_threshold);

/*******************************************************************************
* tpm_reset_pnc_age_group()
*
* DESCRIPTION: The API reset the hitted counter of all the PnC entries of specific
*              group
*
* INPUTS:   owner_id      - APP owner id  should be used for all API calls
*           api_type     - TPM API group type
*
* OUTPUTS:
*            None
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_reset_pnc_age_group(uint32_t owner_id,
					 tpm_api_type_t api_type)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_count_reset_pnc_age_group(owner_id, api_type);
	return (ret_code);
}
EXPORT_SYMBOL(tpm_reset_pnc_age_group);

/*******************************************************************************
* tpm_set_mc_vid_port_vids()
*
* DESCRIPTION: The API sets the multicast vid configuration for each UNI port
*
* INPUTS:   owner_id - APP owner id  should be used for all API calls
*               mc_vid - multicast VID
*               mc_vid_uniports_config - VID configurations for different uni ports
*
* OUTPUTS:
*           None
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_set_mc_vid_port_vids(uint32_t owner_id,
					  uint32_t mc_vid,
					  tpm_mc_vid_port_vid_set_t *mc_vid_uniports_config)
{
	tpm_error_code_t ret;

	ret = tpm_proc_set_mc_vid_port_vids(owner_id, mc_vid, mc_vid_uniports_config);
	if (TPM_RC_OK != ret) {
		TPM_OS_ERROR(TPM_PNCL_MOD, "set VID error\n");
		return ERR_GENERAL;
	}

	return TPM_RC_OK;
}
EXPORT_SYMBOL(tpm_set_mc_vid_port_vids);
/*******************************************************************************
* tpm_set_mtu_enable()
*
* DESCRIPTION:      enable or disable MTU checking.
*
* INPUTS:
*           enable   - set MTU checking to enable or disable.
*
* OUTPUTS:
*           NONE
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_set_mtu_enable(tpm_init_mtu_setting_enable_t  enable)
{
	tpm_db_set_mtu_enable(enable);
	return TPM_RC_OK;
}

/*******************************************************************************
* tpm_set_mtu()
*
* DESCRIPTION:      set the MTU value for PNC to check.
*
* INPUTS:
*           mtu           - MTU value
*           ethertype     - IPv4 or IPv6
*           direction     - US or DS
* OUTPUTS:
*           NONE
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_set_mtu(uint32_t mtu,
			     tpm_mtu_ethertype_t ethertype,
			     uint32_t direction)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_set_mtu(ethertype, direction, mtu);
	return(ret_code);
}

/*******************************************************************************
* tpm_set_pppoe_mtu()
*
* DESCRIPTION:      set the PPPoE packets MTU value for PNC to check.
*
* INPUTS:
*           pppoe_mtu     - MTU value
*           ethertype     - IPv4 or IPv6
*           direction     - US or DS
* OUTPUTS:
*           NONE
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_set_pppoe_mtu(uint32_t		pppoe_mtu,
				   tpm_mtu_ethertype_t	ethertype,
				   uint32_t		direction)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_set_pppoe_mtu(ethertype, direction, pppoe_mtu);
	return(ret_code);
}

/*******************************************************************************
* tpm_add_ctc_cm_acl_rule()
*
* DESCRIPTION:      Creates a new CTC CnM ACL.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* src_port           - The packet originating source port, could be any UNI port:
* precedence         - precedence of this CnM rule, from 0 to 7
* l2_parse_rule_bm
* ipv4_parse_rule_bm
*                    - Bitmap containing the significant flags for parsing fields of the packet.
* l2_key
* ipv4_key
*                    - Information to create a parsing key for the rule.
*                      Some pointers may be NULL depending on the parse_rule_bm.
* pkt_frw            - Information for packet forwarding decision.
* pkt_act            - Action associated to the rule
*
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct precedence of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_add_ctc_cm_acl_rule(uint32_t owner_id,
					 tpm_src_port_type_t src_port,
					 uint32_t precedence,
					 tpm_parse_fields_t l2_parse_rule_bm,
					 tpm_parse_fields_t ipv4_parse_rule_bm,
					 tpm_l2_acl_key_t *l2_key,
					 tpm_ipv4_acl_key_t  *ipv4_key,
					 tpm_pkt_frwd_t *pkt_frwd,
					 tpm_pkt_action_t pkt_act,
					 uint32_t pbits)
{
	tpm_error_code_t ret_code, busy_ret_code;
	uint32_t rule_num;
	tpm_parse_fields_t ipv6_parse_rule_bm = 0;
	tpm_ipv6_acl_key_t ipv6_key;

	tpm_proc_calc_cnm_rule_num(src_port, precedence, &rule_num);

	/* Check API_section Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_CNM, rule_num);
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_ctc_cm_acl_rule_add(owner_id, src_port, precedence, l2_parse_rule_bm, ipv4_parse_rule_bm,
					   ipv6_parse_rule_bm, l2_key, ipv4_key, &ipv6_key, pkt_frwd, pkt_act, pbits);

	busy_ret_code = tpm_proc_api_busy_done(TPM_API_CNM, rule_num);
	RET_BUSY_ERROR(ret_code, busy_ret_code);
}
EXPORT_SYMBOL(tpm_add_ctc_cm_acl_rule);

/*******************************************************************************
* tpm_del_ctc_cm_acl_rule()
*
* DESCRIPTION:      Deletes an existing CTC CnM rule.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* src_port           - The packet originating source port, could be any UNI port:
* precedence         - precedence of this CnM rule, from 0 to 7
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
tpm_error_code_t tpm_del_ctc_cm_acl_rule(uint32_t owner_id,
					 tpm_src_port_type_t src_port,
					 uint32_t precedence)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_ctc_cm_acl_rule_del(owner_id, src_port, precedence);
	return ret_code;
}
EXPORT_SYMBOL(tpm_del_ctc_cm_acl_rule);
/*******************************************************************************
* tpm_flush_vtu()
*
* DESCRIPTION:      Flush VTU on the Switch.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
tpm_error_code_t tpm_flush_vtu(uint32_t owner_id)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_sw_flush_vtu(owner_id);
	return ret_code;
}
EXPORT_SYMBOL(tpm_flush_vtu);
/*******************************************************************************
* tpm_flush_atu()
*
* DESCRIPTION:      Flush ATU on the Switch.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* flush_type         - FLUSH all or FLUSH all dynamic
* db_num             - ATU DB Num, only 0 should be used, since there is only one ATU DB right now.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
tpm_error_code_t tpm_flush_atu(uint32_t owner_id, tpm_flush_atu_type_t flush_type, uint16_t db_num)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_sw_flush_atu(owner_id, flush_type, db_num);
	return ret_code;
}
EXPORT_SYMBOL(tpm_flush_atu);

/*******************************************************************************
* tpm_rule_self_check()
*
* DESCRIPTION:      Performs TPM self check
*
* INPUTS:
* owner_id           - ID of an application which requests ownership on a group of APIs.
* check_level        - The check level determines to correct bad tpm rule or not in the API call
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns API_OWNERSHIP_SUCCESS. On error, see tpm_api_ownership_error_t.
*
* COMMENTS: none
*
*******************************************************************************/
tpm_error_code_t tpm_rule_self_check(uint32_t owner_id,
				     tpm_self_check_level_enum_t check_level)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_self_check(owner_id, check_level);

	return (ret_code);
}
EXPORT_SYMBOL(tpm_rule_self_check);

/*******************************************************************************
* tpm_add_ctc_cm_ipv6_acl_rule()
*
* DESCRIPTION:      Creates a new CTC IPv6 CnM ACL.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* src_port           - The packet originating source port, could be any UNI port:
* precedence         - precedence of this CnM rule, from 0 to 7
* ipv6_parse_rule_bm
*                    - Bitmap containing the significant flags for parsing fields of the packet.
* ipv6_key
*                    - Information to create a parsing key for the rule.
*                      Some pointers may be NULL depending on the parse_rule_bm.
* pkt_frw            - Information for packet forwarding decision.
* pkt_act            - Action associated to the rule
*
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct precedence of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_add_ctc_cm_ipv6_acl_rule(uint32_t owner_id,
					      tpm_src_port_type_t src_port,
					      uint32_t precedence,
					      tpm_parse_fields_t ipv6_parse_rule_bm,
					      tpm_ipv6_acl_key_t  *ipv6_key,
					      tpm_pkt_frwd_t *pkt_frwd,
					      tpm_pkt_action_t pkt_act,
					      uint32_t pbits)
{
	tpm_error_code_t ret_code, busy_ret_code;
	uint32_t rule_num;

	tpm_proc_calc_cnm_rule_num(src_port, precedence, &rule_num);

	/* Check API_section Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_CNM, rule_num);
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_ctc_cm_ipv6_acl_rule_add(owner_id, src_port, precedence, ipv6_parse_rule_bm,
					        ipv6_key, pkt_frwd, pkt_act, pbits);

	busy_ret_code = tpm_proc_api_busy_done(TPM_API_CNM, rule_num);
	RET_BUSY_ERROR(ret_code, busy_ret_code);
}
EXPORT_SYMBOL(tpm_add_ctc_cm_ipv6_acl_rule);

/*******************************************************************************
* tpm_ctc_cm_set_ipv6_parse_window()
*
* DESCRIPTION:      Set IPv6 CnM rule parse window
*     Two sets of IPv6 parse window:
*       - first 24 bytes from IPv6 Header, include fields like: NH, TC, SIP
*       - second 24 bytes from IPv6 Header, include fields like: L4 ports and DIP
*
* INPUTS:
* owner_id           - ID of an application which requests ownership on a group of APIs.
* check_level        - The check level determines to correct bad tpm rule or not in the API call
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns API_OWNERSHIP_SUCCESS. On error, see tpm_api_ownership_error_t.
*
* COMMENTS: none
*
*******************************************************************************/
tpm_error_code_t tpm_ctc_cm_set_ipv6_parse_window(uint32_t owner_id,
				     		  tpm_ctc_cm_ipv6_parse_win_t ipv6_parse_window)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_ctc_cm_set_ipv6_parse_win(owner_id, ipv6_parse_window);

	return (ret_code);
}
EXPORT_SYMBOL(tpm_ctc_cm_set_ipv6_parse_window);
/*******************************************************************************
* tpm_add_mac_learn_rule()
*
* DESCRIPTION:      Add a PNC L2 rule for source MAC learn
*
* INPUTS:
* owner_id           - ID of an application which requests ownership on a group of APIs.
* src_mac_addr       - MAC addr to learn
* OUTPUTS:
* rule_idx           - Unique rule identification number, which is used when deleting the rule.
*                      (this is not the rule_num)
*
* RETURNS:
* On success, the function returns API_OWNERSHIP_SUCCESS. On error, see tpm_api_ownership_error_t.
*
* COMMENTS: none
*
*******************************************************************************/
tpm_error_code_t tpm_add_mac_learn_rule(uint32_t owner_id, tpm_l2_acl_key_t *src_mac_addr)
{
	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_section Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_MAC_LEARN, 0);
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_proc_add_static_mac_rule(owner_id, src_mac_addr);

	busy_ret_code = tpm_proc_api_busy_done(TPM_API_MAC_LEARN, 0);
	RET_BUSY_ERROR(ret_code, busy_ret_code);
}
EXPORT_SYMBOL(tpm_add_mac_learn_rule);

/*******************************************************************************
* tpm_del_mac_learn_rule()
*
* DESCRIPTION:      Del a PNC L2 rule for source MAC learn
*
* INPUTS:
* owner_id           - ID of an application which requests ownership on a group of APIs.
* src_mac_addr       - L2_key define the src mac addr for mac learn.
* OUTPUTS:
*
* RETURNS:
* On success, the function returns API_OWNERSHIP_SUCCESS. On error, see tpm_api_ownership_error_t.
*
* COMMENTS: none
*
*******************************************************************************/
tpm_error_code_t tpm_del_mac_learn_rule(uint32_t owner_id, tpm_l2_acl_key_t *src_mac_addr)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_del_static_mac_rule(owner_id, src_mac_addr);
	return (ret_code);
}
EXPORT_SYMBOL(tpm_del_mac_learn_rule);

/*******************************************************************************
* tpm_mac_learn_default_rule_act_set()
*
* DESCRIPTION:      Set the action for mac learn default rule
*
* INPUTS:
* owner_id           - ID of an application which requests ownership on a group of APIs.
* action             - rule action for mac learn default.
* OUTPUTS:
* On success, the function returns API_OWNERSHIP_SUCCESS. On error, see tpm_api_ownership_error_t.
*
* COMMENTS: none
*
*******************************************************************************/
tpm_error_code_t tpm_mac_learn_default_rule_act_set(uint32_t owner_id, tpm_unknown_mac_conf_t mac_conf)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_mac_learn_default_rule_act_set(owner_id, mac_conf);
	return (ret_code);
}
EXPORT_SYMBOL(tpm_mac_learn_default_rule_act_set);

/*******************************************************************************
* tpm_mac_learn_entry_num_get()
*
* DESCRIPTION:      Get the mac learn entry number currently in system
*
* INPUTS:
* None
* OUTPUTS:
* entry_num          - current MAC learn entry count in MAC_LEARN range, not including default one
* RETURNS:
* On success, the function returns API_OWNERSHIP_SUCCESS. On error, see tpm_api_ownership_error_t.
*
* COMMENTS: none
*
*******************************************************************************/
tpm_error_code_t tpm_mac_learn_entry_num_get(uint32_t *entry_num)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_proc_mac_learn_entry_num_get(entry_num);
	return (ret_code);
}
EXPORT_SYMBOL(tpm_mac_learn_entry_num_get);

/*******************************************************************************
* tpm_set_gmac_loopback()
*
* DESCRIPTION: The API enable/disable loopback mode of gmac.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* gmac                 -
* enable               - 1 for enable, 0 for disable
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_set_gmac_loopback (uint32_t	  owner_id,
						tpm_gmacs_enum_t  gmac,
						uint8_t		  enable)
{
	tpm_error_code_t ret_code = TPM_RC_OK;

	ret_code = tpm_init_gmac_loopback(gmac, enable);

	return (ret_code);
}
EXPORT_SYMBOL(tpm_set_gmac_loopback);

/*******************************************************************************
* tpm_add_ds_load_balance_rule()
*
* DESCRIPTION: The API adds DS load balance PnC rules to set target port to GMAC0 or GMAC1
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* rule_num           - Entry index to be added in the current ACL
* parse_rule_bm      - Bitmap containing the significant flags for parsing fields of the packet.
*                      possible values for L2 API:
*                        TPM_L2_PARSE_MAC_DA|TPM_L2_PARSE_MAC_SA|TPM_L2_PARSE_ONE_VLAN_TAG
*                        |TPM_L2_PARSE_TWO_VLAN_TAG|TPM_L2_PARSE_ETYPE|TPM_L2_PARSE_PPPOE_SES
*                        |TPM_L2_PARSE_PPP_PROT|TPM_L2_PARSE_GEMPORT)
* parse_flags_bm     - Bitmap containing the significant flags result of the primary ACL filtering.
*                      possible values for L2 API:
*                        TPM_PARSE_FLAG_TAG1_TRUE|TPM_PARSE_FLAG_TAG1_FLASE|
*                        TPM_PARSE_FLAG_TAG2_TRUE|TPM_PARSE_FLAG_TAG2_FALSE
* l2_key             - Information to create a parsing key for the rule.
*                      Some pointers may be NULL depending on the parse_rule_bm.
* tgrt_port          - target Port: GMAC0, GMAC1 or CPU
*
* OUTPUTS:
*  rule_idx         - Unique rule identification number which is used when deleting the rule.
*                     (this is not the rule_num)
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_add_ds_load_balance_rule(uint32_t owner_id,
						uint32_t rule_num,
						uint32_t *rule_idx,
						tpm_parse_fields_t parse_rule_bm,
						tpm_parse_flags_t parse_flags_bm,
						tpm_l2_acl_key_t *l2_key,
						tpm_ds_load_balance_tgrt_t tgrt_port)
{
	tpm_error_code_t ret_code, busy_ret_code;

	/* Check API_section Busy */
	ret_code = tpm_proc_check_api_busy(TPM_API_DS_LOAD_BALANCE, rule_num);
	if (ret_code != TPM_OK)
		return(ret_code);

	ret_code = tpm_proc_add_ds_load_balance_acl_rule(owner_id, rule_num, rule_idx,
		parse_rule_bm, parse_flags_bm, l2_key, tgrt_port);

	busy_ret_code = tpm_proc_api_busy_done(TPM_API_DS_LOAD_BALANCE, rule_num);
	RET_BUSY_ERROR(ret_code, busy_ret_code);
}
EXPORT_SYMBOL(tpm_add_ds_load_balance_rule);

/*******************************************************************************
* tpm_del_ds_load_balance_rule()
*
* DESCRIPTION: The API delete CPU egress loopback modification and PnC rules for
*              specific Tcont/queue/gem_port
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* rule_idx           - Unique rule idenitifcation number specifying the rule to be deleted.
*
* OUTPUTS:
*           NONE
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_del_ds_load_balance_rule(uint32_t owner_id, uint32_t rule_idx)
{
	tpm_error_code_t ret_code;
	ret_code = tpm_proc_del_ds_load_balance_acl_rule(owner_id, rule_idx, TPM_EXT_CALL);
	return (ret_code);
}
EXPORT_SYMBOL(tpm_del_ds_load_balance_rule);

/*******************************************************************************
* tpm_xlate_uni_2_switch_port()
*
* DESCRIPTION: The API translates TPM logic UNI port into Switch port.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* uni_port           - TPM logic port that need to be translated.
*
* OUTPUTS:
* switch_port      - switch port.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_xlate_uni_2_switch_port (uint32_t		  owner_id,
						     tpm_src_port_type_t  uni_port,
						     uint32_t		 *switch_port)
{
	tpm_error_code_t ret_code = TPM_RC_OK;

	*switch_port = tpm_db_eth_port_switch_port_get(uni_port);
	if (TPM_DB_ERR_PORT_NUM == *switch_port)
		ret_code = ERR_SRC_PORT_INVALID;

	return (ret_code);
}
EXPORT_SYMBOL(tpm_xlate_uni_2_switch_port);

/*******************************************************************************
* tpm_active_tcont()
*
* DESCRIPTION:    Function used to enable hwf to certain tcont.
*
* INPUTS:
* tcont_num
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_active_tcont(uint32_t tcont_num)
{
	tpm_error_code_t ret_code;

	/* active tcont hwf */
	ret_code = tpm_proc_hwf_admin_set(TPM_ENUM_PMAC, tcont_num, true);

	return ret_code;
}
EXPORT_SYMBOL(tpm_active_tcont);

/*******************************************************************************
* tpm_deactive_tcont()
*
* DESCRIPTION:    Function used to disable hwf to certain tcont.
*
* INPUTS:
* tcont_num
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_deactive_tcont(uint32_t tcont_num)
{
	tpm_error_code_t ret_code;
	/* deactive tcont hwf */
	ret_code = tpm_proc_hwf_admin_set(TPM_ENUM_PMAC, tcont_num, false);

	return ret_code;
}
EXPORT_SYMBOL(tpm_deactive_tcont);

#ifdef CONFIG_MV_INCLUDE_PON
/*******************************************************************************
* tpm_register_pon_callback()
*
* DESCRIPTION: Function used to register PON callback functions.
*
* INPUTS:
* pon_func: PON callback function
*
* OUTPUTS:
* None
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*******************************************************************************/
int tpm_register_pon_callback(PON_SHUTDOWN_FUNC pon_func)
{
	tpm_error_code_t ret_code;

	ret_code = tpm_db_register_pon_callback(pon_func);

	return ret_code;
}
EXPORT_SYMBOL(tpm_register_pon_callback);
#endif

/*******************************************************************************
* tpm_set_drop_precedence_mode()
*
* DESCRIPTION:	  Function used to set drop precedence mode.
*
* INPUTS:
* tcont_num
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_set_drop_precedence_mode(uint32_t owner_id,
				tpm_drop_precedence_t mode)
{
	tpm_error_code_t ret_code;
	/* set drop precedence mode */
	ret_code = tpm_proc_set_drop_precedence_mode(owner_id, mode);

	return ret_code;
}
EXPORT_SYMBOL(tpm_set_drop_precedence_mode);

/*******************************************************************************
* tpm_get_drop_precedence_mode()
*
* DESCRIPTION:	  Function used to get drop precedence mode.
*
* INPUTS:
* tcont_num
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_get_drop_precedence_mode(uint32_t owner_id,
				tpm_drop_precedence_t *mode)
{
	tpm_error_code_t ret_code;

	/* set drop precedence mode */
	ret_code = tpm_proc_get_drop_precedence_mode(owner_id, mode);

	return ret_code;
}
EXPORT_SYMBOL(tpm_get_drop_precedence_mode);
