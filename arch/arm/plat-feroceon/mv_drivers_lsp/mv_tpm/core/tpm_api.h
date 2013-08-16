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
/*******************************************************************************
* tpm_api.h
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

#ifndef _TPM_API_H_
#define _TPM_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tpm_types.h"


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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
* This function is used for error prevention and not as a security mechanism.
*
*******************************************************************************/
tpm_error_code_t tpm_create_ownerid(uint32_t *owner_id);

/*******************************************************************************
* tpm_request_api_ownership()
*
* DESCRIPTION:      Establishes an ownership between owner_id and a group of APIs.
*
* INPUTS:
* owner_id           - ID of an application which requests ownership on a group of APIs.
* api_type          - the API type for which ownership is requested.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns API_OWNERSHIP_SUCCESS.
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_api_ownership_error_t tpm_request_api_ownership(uint32_t 		owner_id,
						    tpm_api_type_t 	api_type);

/*******************************************************************************
* tpm_erase_section()
*
* DESCRIPTION:      Erases a section per an application group.
*
* INPUTS:
* owner_id          - API owner id  should be used for all API calls.
* api_type          - the API PnC section to be deleted.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_erase_section(uint32_t 		owner_id,
				   tpm_api_type_t 	api_type);

/*******************************************************************************
* tpm_get_section_free_size()
*
* DESCRIPTION:      Returns the free size of an application group.
*
* INPUTS:
* api_type          - the API type to retrieve section size for.
*
* OUTPUTS:
* cur_size          - number of free entries for the API type.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_get_section_free_size(tpm_api_type_t 	 api_type,
					   int32_t 		*cur_size);

/******************************************************************************/
/********************************** Data Forwarding APIs **********************/
/******************************************************************************/

/*******************************************************************************
* tpm_add_l2_rule()
*
* DESCRIPTION:      Creates a new L2 processing ACL. Supports Table mode and ACL mode.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter determines if the packet is arriving
*                      from the WAN port, a specific LAN port or ANY LAN port.
* rule_num           - Entry index to be added in the current ACL
* parse_rule_bm      - Bitmap containing the significant flags for parsing fields of the packet.
*                      possible values for L2 API:
*                        TPM_L2_PARSE_GEMPORT|
*                        TPM_L2_PARSE_MAC_DA|TPM_L2_PARSE_MAC_SA|
*                        TPM_L2_PARSE_ONE_VLAN_TAG|TPM_L2_PARSE_TWO_VLAN_TAG|
*                        TPM_L2_PARSE_ETYPE| (Note: ETYPE recommended in tpm_add_l3_type_acl_rule)
* parse_flags_bm     - Bitmap containing the significant flags result of the primary ACL filtering.
*                      possible values for L2 API:
*                        TPM_PARSE_FLAG_TAG1_TRUE|TPM_PARSE_FLAG_TAG1_FLASE|
*                        TPM_PARSE_FLAG_TAG2_TRUE|TPM_PARSE_FLAG_TAG2_FALSE
* l2_key             - Information to create a parsing key for the rule.
*                      Some pointers may be NULL depending on the parse_rule_bm.
* pkt_frwd           - Information for packet forwarding decision.
* pkt_mod            - Packet modification information.
* pkt_mod_bm         - Bitmap contains the set of packet fields to modify.
* rule_action        - Action associated to the rule = drop/set target/set packet modification/to CPU
*                      possible "next_phase" for L2 API   -> STAGE_L3_TYPE , STAGE_DONE
*
* OUTPUTS:
*  rule_idx         - If this API is in Table mode, this will equal to the rule_num,
*                     otherwise it will be a unique number acrosss all API's.
*
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_add_l2_rule(uint32_t 		owner_id,
				 tpm_src_port_type_t 	src_port,
				 uint32_t 		rule_num,
				 uint32_t 	        *rule_idx,
				 tpm_parse_fields_t 	parse_rule_bm,
				 tpm_parse_flags_t 	parse_flags_bm,
				 tpm_l2_acl_key_t      *l2_key,
				 tpm_pkt_frwd_t        *pkt_frwd,
				 tpm_pkt_mod_t         *pkt_mod,
				 tpm_pkt_mod_bm_t 	pkt_mod_bm,
				 tpm_rule_action_t     *rule_action);

/*******************************************************************************
* tpm_del_l2_rule()
*
* DESCRIPTION:      Deletes an existing L2 ACL primary rule.
*                   Any of the existing entries may be deleted.
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* rule_idx           - Rule idenitifcation number specifying the rule to be deleted.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_del_l2_rule(uint32_t owner_id,
				 uint32_t rule_idx);

/*******************************************************************************
* tpm_add_l3_type_rule()
*
* DESCRIPTION:      Creates a new L3 (ether_type or pppoe proto) processing ACL.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter determines if the packet is arriving
*                      from the WAN port, a specific LAN port or ANY LAN port.
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
* l3_key             - Structure for PPPoE proto or ether type.
* pkt_frwd           - Information for packet forwarding decision.
* rule_action        - Action associated to the rule = drop/set target/set packet modification/to CPU
*                      possible "next_phase" for L3 API   ->  STAGE_IPv4, STAGE_IPv6_GEN, STAGE_DONE
*
* OUTPUTS:
*  rule_idx         - Unique rule identification number, acrosss all API's.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_add_l3_type_rule(uint32_t 			owner_id,
				      tpm_src_port_type_t 	src_port,
				      uint32_t 			rule_num,
				      uint32_t 		       *rule_idx,
				      tpm_parse_fields_t 	parse_rule_bm,
				      tpm_parse_flags_t 	parse_flags_bm,
				      tpm_l3_type_key_t        *l3_key,
				      tpm_pkt_frwd_t           *pkt_frwd,
				      tpm_rule_action_t        *rule_action);

/*******************************************************************************
* tpm_del_l3_type_rule()
*
* DESCRIPTION:      Deletes an existing ethernet type or PPPoE proto access-list entry .
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* rule_idx           - rule idenitifcation number specifying the rule to be deleted.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_del_l3_type_rule(uint32_t owner_id,
				      uint32_t rule_idx);

/*******************************************************************************
* tpm_add_ipv4_rule()
*
* DESCRIPTION:      Creates a new IPv4 processing ACL. Supports Table mode and ACL mode.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter determines if the packet is arriving
*                      from the WAN port, a specific LAN port or ANY LAN port.
* rule_num           - Entry index to be added in the current ACL
* parse_rule_bm      - Bitmap containing the significant flags for parsing fields of the packet.
*                      possible values for IPv4 API:
*                        TPM_IPv4_PARSE_SIP|TPM_IPv4_PARSE_DIP|
*                        TPM_IPv4_PARSE_DSCP|TPM_IPv4_PARSE_PROTO|
*                        TPM_PARSE_L4_SRC|TPM_PARSE_L4_DST
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
*  rule_idx         - If this API is in Table mode, this will equal to the rule_num,
*                     otherwise it will be a unique number acrosss all API's.
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_add_ipv4_rule(uint32_t 		owner_id,
				   tpm_src_port_type_t 	src_port,
				   uint32_t 		rule_num,
				   uint32_t 	       *rule_idx,
				   tpm_parse_fields_t 	parse_rule_bm,
				   tpm_parse_flags_t 	parse_flags_bm,
				   tpm_ipv4_acl_key_t  *ipv4_key,
				   tpm_pkt_frwd_t      *pkt_frwd,
				   tpm_pkt_mod_t       *pkt_mod,
				   tpm_pkt_mod_bm_t 	pkt_mod_bm,
				   tpm_rule_action_t   *rule_action);

/*******************************************************************************
* tpm_del_ipv4_rule()
*
* DESCRIPTION:      Deletes an existing IPv4 ACL rule.
*                   Both rule number and key are compulsory.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* rule_idx           - rule idenitifcation number specifying the rule to be deleted.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_del_ipv4_rule(uint32_t owner_id,
				   uint32_t rule_idx);

/*******************************************************************************
* tpm_add_ipv6_nh_rule()
*
* DESCRIPTION:      Creates a new IPv6 NH processing ACL.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* rule_num           - Entry index to be added in the current ACL
* parse_flags_bm     - Bitmap containing the significant flags result of the primary ACL filtering.
*                      possible values for IPv6 NH API:
*                        TPM_PARSE_FLAG_MTM_TRUE|TPM_PARSE_FLAG_MTM_FALSE|
*                        TPM_PARSE_FLAG_TO_CPU_TRUE|TPM_PARSE_FLAG_TO_CPU_FALSE|
*                        TPM_PARSE_FLAG_PPPOE_TRUE|TPM_PARSE_FLAG_PPPOE_FALSE
* nh                 - Information to create a NH parsing key for the rule.
* pkt_frwd           - Information for packet forwarding decision.
* rule_action        - Action associated to the rule = drop/set target/set packet modification/to CPU
*                      and the next phase (for GWY only).
*                      possible "next_phase" for IPv6 NH API   ->  STAGE_IPV6_L4,STAGE_DONE
* OUTPUTS:
*  rule_idx         - Unique rule identification number, acrosss all API's.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
* This version does not support HWF for IPV6. Therefore (pkt_frwd) params are N/A in this version.
*
*******************************************************************************/
tpm_error_code_t tpm_add_ipv6_nh_rule(uint32_t 			owner_id,
				      uint32_t 			rule_num,
				      uint32_t 	       	       *rule_idx,
				      tpm_parse_flags_t 	parse_flags_bm,
				      tpm_nh_iter_t 		nh_iter,
				      uint32_t 			nh,
				      tpm_pkt_frwd_t   	       *pkt_frwd,
				      tpm_rule_action_t        *rule_action);

/*******************************************************************************
* tpm_del_ipv6_nh_rule()
*
* DESCRIPTION:      Deletes an existng IPv6 NH processing ACL.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* rule_idx           - rule idenitifcation number specifying the rule to be deleted.
*
* OUTPUTS:
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_del_ipv6_nh_rule(uint32_t owner_id,
				      uint32_t rule_idx);

/*******************************************************************************
* tpm_add_ipv6_gen_rule()
*
* DESCRIPTION:      Creates a new IPv6 gen processing ACL.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter determines if the packet is arriving
*                      from the WAN port, a specific LAN port or ANY LAN port.
* rule_num           - Entry index to be added in the current ACL
* parse_rule_bm      - Bitmap containing the significant flags for parsing fields of the packet.
*                      possible values for IPv6 GEN API:
*                        TPM_IPv6_PARSE_SIP|TPM_IPv6_PARSE_DSCP|TPM_IPv6_PARSE_HOPL
* parse_flags_bm     - Bitmap containing the significant flags result of the primary ACL filtering.
*                      possible values for IPv6 GEN API:
*                        TPM_PARSE_FLAG_TAG1_TRUE|TPM_PARSE_FLAG_TAG1_FLASE|
*                        TPM_PARSE_FLAG_MTM_TRUE|TPM_PARSE_FLAG_MTM_FALSE|
*                        TPM_PARSE_FLAG_TO_CPU_TRUE|TPM_PARSE_FLAG_TO_CPU_FALSE|
*                        TPM_PARSE_FLAG_PPPOE_TRUE|TPM_PARSE_FLAG_PPPOE_FALSE
* ipv6_gen_key     - Information to create an IPv6 gen parsing key for the rule.
*                      Some pointers may be NULL depending on the parse_rule_bm.
* pkt_frwd           - Information for packet forwarding decision.
* pkt_mod            - Packet modification information.
* pkt_mod_bm         - Bitmap containing the significant fields to modify
* rule_action        - Action associated to the rule = drop/set target/set packet modification/to CPU
*                      and the next phase (for GWY only).
*                      possible "next_phase" for IPv6 GEN API   ->  STAGE_IPv6_DIP, ,STAGE_DONE
*
* OUTPUTS:
*  rule_idx         - Unique rule identification number, acrosss all API's.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
* This version does not support HWF for IPV6. Therefore (pkt_mod_bm, pkt_mod, pkt_frwd) params are N/A in this version.
*
*******************************************************************************/
tpm_error_code_t tpm_add_ipv6_gen_rule(uint32_t 			owner_id,
					 tpm_src_port_type_t 		src_port,
					 uint32_t 			rule_num,
					 uint32_t 		       *rule_idx,
					 tpm_parse_fields_t 		parse_rule_bm,
					 tpm_parse_flags_t 		parse_flags_bm,
					 tpm_ipv6_gen_acl_key_t      *ipv6_gen_key,
					 tpm_pkt_frwd_t 	       *pkt_frwd,
					 tpm_pkt_mod_t 		       *pkt_mod,
					 tpm_pkt_mod_bm_t 		pkt_mod_bm,
					 tpm_rule_action_t             *rule_action);

/*******************************************************************************
* tpm_del_ipv6_gen_rule()
*
* DESCRIPTION:      Deletes an existng IPv6 gen processing ACL.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* rule_idx           - rule idenitifcation number specifying the rule to be deleted.
*
* OUTPUTS:
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_del_ipv6_gen_rule(uint32_t owner_id,
					 uint32_t rule_idx);

/*******************************************************************************
* tpm_add_ipv6_dip_rule()
*
* DESCRIPTION:      Creates a new IPv6 DIP processing ACL.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter determines if the packet is arriving
*                      from the WAN port, a specific LAN port or ANY LAN port.
* rule_num           - Entry index to be added in the current ACL
* parse_rule_bm      - Bitmap containing the significant flags for parsing fields of the packet.
*                      possible values for IPv6 DIP API:
*                        TPM_IPv6_PARSE_DIP
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
* pkt_mod_bm         - Bitmap containing the significant fields to modify
* rule_action        - Action associated to the rule = drop/set target/set packet modification/to CPU
*                      and the next phase (for GWY only).
*                      possible "next_phase" for IPv6 GEN API   ->  STAGE_IPv6_NHSTAGE_DONE
*
* OUTPUTS:
*  rule_idx          - Unique rule identification number, acrosss all API's.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
* This version does not support HWF for IPV6. Therefore (pkt_mod_bm, pkt_mod, pkt_frwd) params are N/A in this version.
*
*******************************************************************************/
tpm_error_code_t tpm_add_ipv6_dip_rule(uint32_t 		owner_id,
				       tpm_src_port_type_t 	src_port,
				       uint32_t 		rule_num,
				       uint32_t 	       *rule_idx,
				       tpm_parse_fields_t 	parse_rule_bm,
				       tpm_parse_flags_t 	parse_flags_bm,
				       tpm_ipv6_addr_key_t     *ipv6_dip_key,
				       tpm_pkt_frwd_t          *pkt_frwd,
				       tpm_pkt_mod_t           *pkt_mod,
				       tpm_pkt_mod_bm_t 	pkt_mod_bm,
				       tpm_rule_action_t       *rule_action);

/*******************************************************************************
* tpm_del_ipv6_dip_rule()
*
* DESCRIPTION:      Deletes an existng IPv6 DIP processing ACL.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* rule_idx           - rule idenitifcation number specifying the rule to be deleted.
*
* OUTPUTS:
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_del_ipv6_dip_rule(uint32_t owner_id,
				       uint32_t rule_idx);

/*******************************************************************************
* tpm_add_ipv6_l4_ports_rule()
*
* DESCRIPTION:      Creates a new IPv6 L4 processing ACL.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter determines if the packet is arriving
*                      from the WAN port, a specific LAN port or ANY LAN port.
* rule_num           - Entry index to be added in the current ACL
* parse_rule_bm      - Bitmap containing the significant flags for parsing fields of the packet.
*                      possible values for L4 API:
*                        TPM_PARSE_L4_SRC|TPM_PARSE_L4_DST
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
*  rule_idx          - Unique rule identification number, acrosss all API's.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
* This version does not support HWF for IPV6. Therefore (pkt_mod_bm, pkt_mod, pkt_frwd) params are N/A in this version.
*
*******************************************************************************/
tpm_error_code_t tpm_add_ipv6_l4_ports_rule(uint32_t 			owner_id,
					    tpm_src_port_type_t 	src_port,
					    uint32_t 			rule_num,
					    uint32_t 		       *rule_idx,
					    tpm_parse_fields_t 		parse_rule_bm,
					    tpm_parse_flags_t 		parse_flags_bm,
					    tpm_l4_ports_key_t 	       *l4_key,
					    tpm_pkt_frwd_t 	       *pkt_frwd,
					    tpm_pkt_mod_t 	       *pkt_mod,
					    tpm_pkt_mod_bm_t 		pkt_mod_bm,
					    tpm_rule_action_t 	       *rule_action);

/*******************************************************************************
* tpm_del_ipv6_l4_ports_rule()
*
* DESCRIPTION:      Deletes an existng L4 processing ACL.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* rule_idx           - rule idenitifcation number specifying the rule to be deleted.
*
* OUTPUTS:
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_del_ipv6_l4_ports_rule(uint32_t owner_id,
					    uint32_t rule_idx);


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
										tpm_rule_action_t *rule_action);

tpm_error_code_t tpm_del_ipv6_gen_5t_rule(uint32_t owner_id, uint32_t rule_idx);

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
										tpm_rule_action_t *rule_action);

tpm_error_code_t tpm_del_ipv6_dip_5t_rule(uint32_t owner_id, uint32_t rule_idx);

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
											tpm_rule_action_t *rule_action);

tpm_error_code_t tpm_del_ipv6_l4_ports_5t_rule(uint32_t owner_id, uint32_t rule_idx);


/******************************************************************************/
/********************************** MC handling APIs **************************/
/******************************************************************************/

/*******************************************************************************
* tpm_add_ipv4_mc_stream()
*
* DESCRIPTION:      Creates a new IPv4 MC stream.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* stream_num         - MC stream number.
* igmp_mode          - Defines if stream is in snooping or in proxy mode. Not relevant when sending stream to CPU.
*                        snooping_mode: Vlan_translation only
*                        proxy_mode   : snooping_mode + Replace SA_MAC, and reduce TTL.
*                                       For pppoe stream, replace unicast DA to MC DA, and delete pppoe header.
* mc_stream_pppoe    - The stream's underlying L2 protocol.  0(IPv4oE), 1(IPv4oPPPoE)
* vid                - VLAN ID (0-4095).
*                      If set to 0xFFFF - do not care about vid.
* ipv4_src_add       - IPv4 source IP address in network order.
* ipv4_src_add       - IPv4 destination IP address in network order.
* ignore_ipv4_src    - when set to 1 - the IP source is not part of the key.
* dest_port_bm       - bitmap which includes all destination UNI ports.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*                   It is APIs caller responsibility to maintain the correct number of
*
*******************************************************************************/
tpm_error_code_t tpm_add_ipv4_mc_stream(uint32_t		 owner_id,
					uint32_t 		stream_num,
					tpm_mc_igmp_mode_t 	igmp_mode,
					uint8_t 		mc_stream_pppoe,
					uint16_t 		vid,
					uint8_t 		ipv4_src_add[4],
					uint8_t 		ipv4_dst_add[4],
					uint8_t 		ignore_ipv4_src,
					tpm_trg_port_type_t 	dest_port_bm);

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
					tpm_trg_port_type_t dest_port_bm);

/*******************************************************************************
* tpm_updt_ipv4_mc_stream()
*
* DESCRIPTION:      Updates an existing IPv4 MC stream.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* stream_num         - MC stream number.
* dest_port_bm       - bitmap which includes all destination UNI ports.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_updt_ipv4_mc_stream(uint32_t 		owner_id,
					 uint32_t 		stream_num,
					 tpm_trg_port_type_t 	dest_port_bm);

/*******************************************************************************
* tpm_del_ipv4_mc_stream()
*
* DESCRIPTION:      Deletes an existing IPv4 MC stream.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* stream_num         - MC stream number.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_del_ipv4_mc_stream(uint32_t owner_id,
					uint32_t stream_num);

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
					tpm_trg_port_type_t dest_port_bm);

/*******************************************************************************
* tpm_add_ipv6_mc_stream_set_queue()
*
* DESCRIPTION:      Creates a new ipv6 MC stream with dest Queue.
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
					tpm_trg_port_type_t dest_port_bm);

/*******************************************************************************
* tpm_updt_ipv6_mc_stream()
*
* DESCRIPTION:      Updates an existing ipv6 MC stream.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* stream_num         - MC stream number.
* dest_port_bm       - bitmap which includes all destination UNI ports.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_updt_ipv6_mc_stream(uint32_t 		owner_id,
					 uint32_t 		stream_num,
					 tpm_trg_port_type_t 	dest_port_bm);

/*******************************************************************************
* tpm_del_ipv6_mc_stream()
*
* DESCRIPTION:      Deletes an existing ipv6 MC stream.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* stream_num         - MC stream number.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_del_ipv6_mc_stream(uint32_t owner_id,
					uint32_t stream_num);

/*******************************************************************************
* tpm_set_mc_vid_port_vids()
*
* DESCRIPTION:         Sets the port_participation for a Multicast VID.
*                      Each uni_port can be excluded, or member in (transparent, vlan_strip, vlan_translate) mode.
* INPUTS:
* owner_id                      - API owner id  should be used for all API calls.
* mc_vid                        - The Multicast VID.
* mc_vid_uniports_config        - Array of per_uni_port configurations.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_set_mc_vid_port_vids(uint32_t 			owner_id,
					  uint32_t 			mc_vid,
					  tpm_mc_vid_port_vid_set_t    *mc_vid_uniports_config);

/*******************************************************************************
* tpm_set_port_igmp_frwd_mode()
*
* DESCRIPTION:      Set the IGMP status of a UNI port or WAN port.
*
* INPUTS:
* src_port         - source port to set IGMP behavior for, valid values (WAN, UNI_0/1/2/3/4)
* mode             - how to process IGMP packets (drop, forward, snoop_to_cpu)
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_set_port_igmp_frwd_mode(tpm_src_port_type_t  src_port,
					     tpm_igmp_frwd_mode_t mode);

/*******************************************************************************
* tpm_set_igmp_cpu_rx_queue()
*
* DESCRIPTION:      Set queue number which IGMP packets are forwarded to.
*
* INPUTS:
* queue             - cpu queue number to send IGMP packets to.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_set_igmp_cpu_rx_queue(uint32_t queue);

/*******************************************************************************
* tpm_mib_reset()
*
* DESCRIPTION:      Performs MIB reset
*
* INPUTS:
* owner_id           - ID of an application which requests ownership on a group of APIs.
* reset_level        - The reset level determines the which elements will not be reset in the API call
*                      (by comparing to the element's reset level)
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
tpm_error_code_t tpm_mib_reset(uint32_t 		owner_id,
			       tpm_reset_level_enum_t	reset_level);

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
* owner_id           - API owner id  should be used for all API calls.
* gem_port           - for OMCI Rx frames - the gem port wherefrom the OMCI frames are received.
* cpu_rx_queue       - for OMCI Rx frames - the CPU rx queue number.
* tcont_num          - for OMCI Tx frames - the TCONT number where to send the OMCI frames.
* cpu_tx_queue       - for OMCI Tx frames - the CPU tx queue number.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_omci_add_channel(uint32_t 			owner_id,
				      tpm_gem_port_key_t 	gem_port,
				      uint32_t 			cpu_rx_queue,
				      tpm_trg_port_type_t 	tcont_num,
				      uint32_t 			cpu_tx_queue);

/*******************************************************************************
* tpm_omci_del_channel()
*
* DESCRIPTION:      Deletes an existing communication channel for the OMCI management protocol.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_omci_del_channel(uint32_t owner_id);

/*******************************************************************************
* tpm_oam_epon_add_channel()
*
* DESCRIPTION:      Establishes a communication channel for the OAM EPON management protocol.
*                   The API sets the Rx input queue in the CPU, and the
*                   Tx T-CONT and queue parameters, which are configured in the driver.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
* cpu_rx_queue       - for OAM (EPON) Rx frames - the CPU rx queue number.
* llid_num           - for OAM (EPON)Tx frames - the LLID number where to send the OMCI frames.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_oam_epon_add_channel(uint32_t		owner_id,
					  uint32_t		cpu_rx_queue,
					  tpm_trg_port_type_t	llid_num);

/*******************************************************************************
* tpm_oam_epon_del_channel()
*
* DESCRIPTION:      Deletes an existing communication channel for the OAM EPON management protocol.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_oam_epon_del_channel(uint32_t owner_id);

/*******************************************************************************
* tpm_oam_loopback_add_channel()
*
* DESCRIPTION:      Establishes a communication channel for the EPON OAM remote loopback.
*
*
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_oam_loopback_add_channel(uint32_t owner_id);

/*******************************************************************************
* tpm_oam_loopback_del_channel()
*
* DESCRIPTION:      Delete the communication channel for the OAM remote loopback .
*
*
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_oam_loopback_del_channel(uint32_t owner_id);

/*******************************************************************************
* tpm_loop_detect_add_channel()
*
* DESCRIPTION:      Establishes a communication channel for loop detection application.
*
* INPUTS:
* owner_id           - API owner id  should be used for all API calls.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_loop_detect_add_channel(uint32_t owner_id, tpm_ether_type_key_t ety);

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
tpm_error_code_t tpm_loop_detect_del_channel(uint32_t owner_id);

/******************************************************************************/
/********************************** Configuration retrieval APIs **************/
/******************************************************************************/

/*******************************************************************************
* tpm_get_next_valid_rule()
*
* DESCRIPTION:      General purpose API to retrieve the internal configuration of an existing ACL.
*
* INPUTS:
* owner_id            - API owner id  should be used for all API calls.
* current_index       - the entry index in the section (rule_num/stream_num). In case it is (-1) - the get next function
*                      will point to the first ruleof the section.
* api_type            - TPM API type
*
* OUTPUTS:
* next_index         - returns the first following index (rule_num/stream_num) after the index in the current_index
*                      parameter. It is invalid, if there is no next_index.
* rule_idx           - Unique rule identification number. Equals to stream_num for API's that are operating
*                      in table mode.
* tpm_rule           - points to a structure holding the information of a single rule,
*                      of the type specified in the rule_type param.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_get_next_valid_rule(uint32_t		owner_id,
					 int32_t 		current_index,
					 tpm_api_type_t 	api_type,
					 int32_t       	       *next_index,
					 uint32_t	       *rule_idx,
					 tpm_rule_entry_t      *tpm_rule);

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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_omci_get_channel(uint32_t 			*is_valid,
				      tpm_gem_port_key_t 	*gem_port,
				      uint32_t 			*cpu_rx_queue,
				      tpm_trg_port_type_t 	*tcont_num,
				      uint32_t 			*cpu_tx_queue);

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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_get_port_igmp_frwd_mode(tpm_src_port_type_t   src_port,
					     tpm_igmp_frwd_mode_t *mode);

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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_get_igmp_cpu_rx_queue(uint32_t *queue);

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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_set_igmp_proxy_sa_mac(uint8_t *sa_mac);

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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_get_igmp_proxy_sa_mac(uint8_t *sa_mac);

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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_oam_epon_get_channel(uint32_t		*is_valid,
					  uint32_t 		*cpu_rx_queue,
					  tpm_trg_port_type_t 	*llid_num);

/*******************************************************************************
* tpm_get_api_ownership()
*
* DESCRIPTION:      Retrieves OAM EPON management protocol channel information.
*
* INPUTS:
* api_group          - Specifies the API group whom owner_id is requested.
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
tpm_api_ownership_error_t tpm_get_api_ownership(uint32_t      *owner_id,
						tpm_api_type_t api_type);

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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_mod_entry_set(tpm_trg_port_type_t 		trg_port,
				   tpm_pkt_mod_bm_t 		mod_bm,
				   tpm_pkt_mod_int_bm_t 	int_mod_bm,
				   tpm_pkt_mod_t 	       *mod_data,
				   uint32_t 		       *mod_entry);

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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_mod_entry_get(tpm_trg_port_type_t	trg_port,
				   uint32_t 		mod_entry,
				   uint16_t 	       *valid_cmds,
				   uint16_t	       *pnc_ref,
				   tpm_mod_rule_t      *rule);

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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_mod_entry_del(tpm_trg_port_type_t 	trg_port,
				   uint32_t 		mod_entry);

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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_mod_mac_inv(tpm_trg_port_type_t trg_port);

/******************************************************************************/
/*************************** Least Used & Hit Counters APIs *******************/
/******************************************************************************/

/*******************************************************************************
* tpm_get_pnc_lu_entry()
*
* DESCRIPTION: The API get least used PnC rule by in specific PnC range
*
* INPUTS:   owner_id        - API owner id  should be used for all API calls
*           api_type        - TPM API type
*           lu_num          - The required number of least used PnC entries
*           lu_reset        - Should the API reset the counters after after reading the LU Counters
*
*
* OUTPUTS:
*           valid_num       - Number of valid least used PnC entries
*           count_array     - Array of the returned least used PnC entry rule_idx  and hit_counter.
*           unrelated_num   - Number of unrelated  least used PnC entries
*                             (PnC entries which were not part of this API_type)
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_get_pnc_lu_entry(uint32_t			owner_id,
				      tpm_api_type_t 		api_type,
				      uint16_t 			lu_num,
				      uint8_t 			lu_reset,
				      uint16_t 		       *valid_num,
				      tpm_api_entry_count_t    *count_array,
				      uint16_t 		       *unrelated_num);
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
* OUTPUTS:
*           valid_counters      - The valid number of entries copied to count_array
*           count_array         - The PnC entries for the API type lower than high_thresh_pkts
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_get_pnc_all_hit_counters(uint32_t                owner_id,
					      tpm_api_type_t          api_type,
					      uint32_t                high_thresh_pkts,
					      uint8_t                 counters_reset,
					      uint16_t               *valid_counters,
					      tpm_api_entry_count_t  *count_array);

/*******************************************************************************
* tpm_set_pnc_counter_mask()
*
* DESCRIPTION: The API set the ability to mask or unmask a specific API entry from being LU scanned.
*
* INPUTS:   owner_id     - API owner id  should be used for all API calls
*           api_type     - TPM API type
*           rule_idx     - The rule index of the requested entry.
*           lu_rule_mask - The least used scanner mask
*                            0:enable_scanner, 1: mask_from_scanner
*
* OUTPUTS:
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_set_pnc_counter_mask(uint32_t		owner_id,
					  tpm_api_type_t 	api_type,
					  uint32_t 		rule_idx,
					  uint32_t 		lu_rule_mask);

/*******************************************************************************
* tpm_get_pnc_hit_count()
*
* DESCRIPTION: The API get the hit counter according to rule_idx
*
* INPUTS:   owner_id     - API owner id  should be used for all API calls
*           api_type     - TPM API type
*           rule_idx     - The rule index of the requested entry.
*           hit_reset    - Should the API reset the hit counters after after reading
*
* OUTPUTS:
*           hit_count    - The number of hits of the rule.
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_get_pnc_hit_count(uint32_t 	owner_id,
				       tpm_api_type_t 	api_type,
				       uint32_t 	rule_idx,
				       uint8_t		hit_reset,
				       uint32_t        *hit_count);

/*******************************************************************************
* tpm_set_pnc_lu_threshold()
*
* DESCRIPTION: The API set the theshold packets number for least used scanner
*
* INPUTS:   owner_id       - API owner id  should be used for all API calls
*           api_type       - TPM API type
*           lu_thresh_pkts - The least_used theshold to be used for the lu_scanner, in number of packets.
*                            Hit_counts above this threshold are not returned by the scanner,
*                            even if they are the lowest amoung their peers.
* OUTPUTS:
*            None
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_set_pnc_lu_threshold(uint32_t 		owner_id,
					  tpm_api_type_t 	api_type,
					  uint32_t 		lu_thresh_pkts);

/*******************************************************************************
* tpm_reset_pnc_age_group()
*
* DESCRIPTION: This API resets the hit counters of all the PnC entries of a specific
*              API. It may reset other PnC entries as well, if they share the same hit_group.
*
* INPUTS:   owner_id       - API owner id  should be used for all API calls
*           api_type       - TPM API type
*
* OUTPUTS:
*            None
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_reset_pnc_age_group(uint32_t 	owner_id,
					 tpm_api_type_t api_type);

/*******************************************************************************
* tpm_add_cpu_wan_loopback()
*
* DESCRIPTION: The API allows the CPU to set the target for packets that will be
*              loopbacked to the WAN port to a HWF queue, by means of packet_modification.
*              It returns to the CPU the mod_idx to put in the Tx_Descriptor, so that the
*              packet will be forwarded to the correct target. The cpu_loopback mechanism is
*              done by use of GMAC1, it is assumed that GMAC0&GMAC1 are both connected to the Internal Switch.
*
* INPUTS:   owner_id    - owner id
*           pkt_frwd    - packet forwarding info: target tcont/LLID number, 0-7
*                         target queue, 0-7, target gem port ID
*
* OUTPUTS:
*           mod_idx     - Returned by HW modification, to fill Tx description
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_add_cpu_wan_loopback(uint32_t 		owner_id,
					  tpm_pkt_frwd_t       *pkt_frwd,
					  uint32_t 	       *mod_idx);

/*******************************************************************************
* tpm_del_cpu_wan_loopback()
*
* DESCRIPTION: The API delete CPU egress loopback modification and PnC rules for
*              specific Tcont/queue/gem_port
*
* INPUTS:   owner_id    - owner id
*           pkt_frwd    - packet forwarding info: target tcont/LLID number, 0-7
*                         target queue, 0-7, target gem port ID
*
* OUTPUTS:
*           NONE
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_del_cpu_wan_loopback(uint32_t 	  owner_id,
					  tpm_pkt_frwd_t *pkt_frwd);

/*******************************************************************************
* tpm_rx_igmp_frame()
*
* DESCRIPTION: The API receives IGMP frames from either LAN or WAN side,
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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_rx_igmp_frame(uint32_t 		owner_id,
				   uint16_t 	       *llid,
				   tpm_src_port_type_t *src_port,
				   uint8_t 	       *buf,
				   uint32_t 	       *len);
/*******************************************************************************
* tpm_tx_igmp_frame()
*
* DESCRIPTION: The API sends IGMP frames to either LAN or WAN side
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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
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
				   uint32_t len);

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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_set_mtu_enable(tpm_init_mtu_setting_enable_t enable);

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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_set_mtu(uint32_t mtu,
			     tpm_mtu_ethertype_t ethertype,
			     uint32_t direction);

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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_set_pppoe_mtu(uint32_t pppoe_mtu,
				   tpm_mtu_ethertype_t ethertype,
				   uint32_t direction);

tpm_error_code_t tpm_rule_self_check(uint32_t owner_id,
				     tpm_self_check_level_enum_t check_level);

/******************************************************************************/
/********************************** Switch APIs *******************************/
/******************************************************************************/

/*******************************************************************************
* tpm_sw_add_static_mac
*
* DESCRIPTION:
*       This function creates a static MAC entry in the MAC address table for a
*       specific lport in the integrated switch
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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_add_static_mac(uint32_t owner_id,
				       tpm_src_port_type_t src_port,
				       uint8_t static_mac[6]);

/*******************************************************************************
* tpm_sw_del_static_mac
*
* DESCRIPTION:
*       This function removes an existing static MAC entry from the MAC address
*       table  in the integrated switch.
*
* INPUTS:
*       owner_id    - APP owner id should be used for all API calls.
*       static_mac  - 6byte network order MAC source address.
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_del_static_mac(uint32_t owner_id,
				       uint8_t static_mac[6]);

/*******************************************************************************
* tpm_sw_set_port_max_macs
*
* DESCRIPTION:
*       This function limits the number of MAC addresses per lport.
*
* INPUTS:
*       owner_id     - APP owner id  should be used for all API calls.
*       src_port     - Source port in UNI port index, UNI0, UNI1...UNI4.
*       mac_per_port - maximum number of MAC addresses per port (1-255).
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       The following care is needed when enabling this feature:
*            1) disable learning on the ports
*            2) flush all non-static addresses in the ATU
*            3) define the desired limit for the ports
*            4) re-enable learing on the ports
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_port_max_macs(uint32_t owner_id,
					  tpm_src_port_type_t src_port,
					  uint8_t mac_per_port);

/*******************************************************************************
* tpm_sw_clear_dynamic_mac
*
* DESCRIPTION:
*       Clear all dynamic MAC.
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None
*
*******************************************************************************/
tpm_error_code_t tpm_sw_clear_dynamic_mac(uint32_t owner_id);

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
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_port_mirror(uint32_t owner_id,
					uint32_t sport,
					uint32_t dport,
					tpm_sw_mirror_type_t mode,
					bool enable);
/*******************************************************************************
* tpm_sw_set_trunk_ports
*
* DESCRIPTION:
*       This function creates trunk ports and trunk id on switch
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
);
/*******************************************************************************
* tpm_sw_set_trunk_mask
*
* DESCRIPTION:
*       This function sets trunk mask on switch
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
);

/*******************************************************************************
* tpm_sw_get_port_mirror
*
* DESCRIPTION:
*       Get port mirror status.
*
* INPUTS:
*       owner_id   - APP owner id , should be used for all API calls.
*       sport      - Source port.
*       dport      - Destination port.
*       mode       - mirror mode.
*
* OUTPUTS:
*       enable     - enable/disable mirror.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_port_mirror(uint32_t owner_id,
					uint32_t sport,
					uint32_t dport,
					tpm_sw_mirror_type_t mode,
					bool *enable);

/*******************************************************************************
* tpm_sw_set_isolate_eth_port_vector()
*
* DESCRIPTION:      Isolate port vector.
*
* INPUTS:
*       owner_id    - APP owner id should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*       port_vector - port vector.
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_isolate_eth_port_vector(uint32_t owner_id,
						    tpm_src_port_type_t src_port,
						    uint32_t port_vector);

/*******************************************************************************
* tpm_sw_get_isolate_eth_port_vector()
*
* DESCRIPTION:      Isolate port vector.
*
* INPUTS:
*       owner_id    - APP owner id should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       port_vector - port vector.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_isolate_eth_port_vector(uint32_t owner_id,
						    tpm_src_port_type_t src_port,
						    uint32_t *port_vector);

/*******************************************************************************
* tpm_set_mtu_size
*
* DESCRIPTION:
*       Set switch MTU size.
*
* INPUTS:
*       owner_id   - APP owner id , should be used for all API calls.
*       type       - MRU type:GMAC0, GMAC1, PONMAC, switch
*       mtu        - MTU size.
*
* OUTPUTS:
*       None
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None
*
*******************************************************************************/
tpm_error_code_t tpm_set_mtu_size(uint32_t owner_id,
				  tpm_mru_type_t type,
				  uint32_t mtu);

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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None
*
*******************************************************************************/
tpm_error_code_t tpm_get_mtu_size(uint32_t owner_id,
				  tpm_mru_type_t type,
				  uint32_t *mtu);

/*******************************************************************************
* tpm_sw_set_port_tagged
*
* DESCRIPTION:
*       The API allows or drops tagged packets on a per lport basis.
*
* INPUTS:
*       owner_id     - APP owner id should be used for all API calls.
*       src_port     - Source port in UNI port index, UNI0, UNI1...UNI4.
*       drop_tagged - set to 1 = drop tagged packets per lport
*                      set to 0 = allow tagged packets per lport.
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_port_tagged(uint32_t owner_id,
					tpm_src_port_type_t src_port,
					uint8_t drop_tagged);

/*******************************************************************************
* tpm_sw_get_port_tagged
*
* DESCRIPTION:
*       This routine gets DiscardTagged bit for the given lport.
*
* INPUTS:
*       owner_id   - APP owner id should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       mode       - MV_TRUE if DiscardTagged bit is set, MV_FALSE otherwise
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_port_tagged(uint32_t owner_id,
					tpm_src_port_type_t src_port,
					uint32_t *mode);

/*******************************************************************************
* tpm_sw_set_port_untagged
*
* DESCRIPTION:
*       The API allows or drops untagged packets on a per UNI port basis.
*
* INPUTS:
*       owner_id       - APP owner id should be used for all API calls.
*       src_port       - Source port in UNI port index, UNI0, UNI1...UNI4.
*       drop_untagged - set to 1 = drop untagged packets per lport
*                        set to 0 = alow untagged packets per lport.
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_port_untagged(uint32_t owner_id,
					  tpm_src_port_type_t src_port,
					  uint8_t drop_untagged);

/*******************************************************************************
* tpm_sw_get_port_untagged
*
* DESCRIPTION:
*       This routine gets DiscardUntagged bit for the given lport.
*
* INPUTS:
*       owner_id    - APP owner id should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       mode        - MV_TRUE if DiscardUntagged bit is set, MV_FALSE otherwise
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_port_untagged(uint32_t owner_id,
					  tpm_src_port_type_t src_port,
					  uint32_t *mode);

/*******************************************************************************
* tpm_sw_set_port_def_vlan
*
* DESCRIPTION:
*       The API sets port default vlan id.
*
* INPUTS:
*       owner_id   - APP owner id should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*       vid        - the port vlan id.
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_port_def_vlan(uint32_t owner_id,
					  tpm_src_port_type_t src_port,
					  uint16_t vid);

/*******************************************************************************
* tpm_sw_get_port_def_vlan
*
* DESCRIPTION:
*       The API gets port default vlan id.
*
* INPUTS:
*       owner_id   - APP owner id should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       vid        - the port vlan id
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_port_def_vlan(uint32_t owner_id,
					  tpm_src_port_type_t src_port,
					  uint16_t *vid);

/*******************************************************************************
* tpm_sw_set_port_def_pri
*
* DESCRIPTION:
*       The API sets port default priority.
*
* INPUTS:
*       owner_id   - APP owner id should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*       pri        - the port priority.
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_port_def_pri(uint32_t owner_id,
					 tpm_src_port_type_t src_port,
					 uint8_t pri);

/*******************************************************************************
* tpm_sw_get_port_def_pri
*
* DESCRIPTION:
*       The API gets port default priority.
*
* INPUTS:
*       owner_id   - APP owner id should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       pri        - the port priority.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_port_def_pri(uint32_t owner_id,
					 tpm_src_port_type_t src_port,
					 uint8_t *pri);

/*******************************************************************************
* tpm_sw_port_add_vid
*
* DESCRIPTION:
*       The API adds a VID to the list of the allowed VIDs per lport.
*
* INPUTS:
*       owner_id   - APP owner id should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*       vid        - VLAN id.
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       see the example sample802_1qSetup().
*
*******************************************************************************/
tpm_error_code_t tpm_sw_port_add_vid(uint32_t owner_id,
				     tpm_src_port_type_t src_port,
				     uint16_t vid);

/*******************************************************************************
* tpm_sw_port_add_vid_group()
*
* DESCRIPTION:  Add a group of VID to the list of the allowed VIDs per port,
*               and set the egress mode correspondingly.
*
* INPUTS:
*       owner_id - APP owner id should be used for all API calls.
*       src_port - Source port in UNI port index, UNI0, UNI1...UNI4.
*       mode     - VLAN egress mode.
*       min_vid  - min VLAN ID.
*       max_vid  - max VLAN ID.
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_port_add_vid_group(uint32_t owner_id,
					   tpm_src_port_type_t src_port,
					   uint8_t mode,
					   uint16_t min_vid,
					   uint16_t max_vid);

/*******************************************************************************
* tpm_sw_port_del_vid_group()
*
* DESCRIPTION:      Delete a group of VID to the list of the allowed VIDs per port,
*                    and set the egress mode correspondingly.
*
* INPUTS:
*       owner_id - APP owner id should be used for all API calls.
*       src_port - Source port in UNI port index, UNI0, UNI1...UNI4.
*       min_vid  - min VLAN ID.
*       max_vid  - max VLAN ID.
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_port_del_vid_group(uint32_t owner_id,
					   tpm_src_port_type_t src_port,
					   uint16_t min_vid,
					   uint16_t max_vid);

/*******************************************************************************
* tpm_sw_port_del_vid
*
* DESCRIPTION:
*       The API delete and existing VID from the list of VIDs allowed per lport.
*
* INPUTS:
*       owner_id   - APP owner id should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*       vid        - VLAN id.
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_port_del_vid(uint32_t owner_id,
				     tpm_src_port_type_t src_port,
				     uint16_t vid);

/*******************************************************************************
* tpm_sw_clear_vid_per_port
*
* DESCRIPTION:
*       The API delete all VID from the list of VIDs allowed per lport.
*
* INPUTS:
*       owner_id   - APP owner id should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_clear_vid_per_port(uint32_t owner_id,
					   tpm_src_port_type_t src_port);

/*******************************************************************************
* tpm_sw_add_all_vid_per_port
*
* DESCRIPTION:
*       The API adds all allowed VIDs from 1 to 4095 per lport.
*
* INPUTS:
*       owner_id - APP owner id should be used for all API calls.
*       src_port - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_add_all_vid_per_port(uint32_t owner_id,
					     tpm_src_port_type_t src_port);

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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       MEMBER_EGRESS_UNMODIFIED - 0
*       NOT_A_MEMBER             - 1
*       MEMBER_EGRESS_UNTAGGED   - 2
*       MEMBER_EGRESS_TAGGED     - 3
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_port_vid_egress_mode(uint32_t owner_id,
						 tpm_src_port_type_t src_port,
						 uint16_t vid,
						 uint8_t eMode);

/*******************************************************************************
* tpm_sw_port_set_vid_filter
*
* DESCRIPTION:
*       The API sets the filtering mode of a certain lport.
*       If the lport is in filtering mode, only the VIDs added by the
*       tpm_sw_port_add_vid API will be allowed to ingress and egress the lport.
*
* INPUTS:
*       owner_id    - APP owner id should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*       vid_filter  - set to 1 - means the lport will DROP all packets which are NOT in
*                    the allowed VID list (built using API tpm_sw_port_add_vid).
*                    set to  0 - means that the list of VIDs allowed
*                    per lport has no significance (the list is not deleted).
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_port_set_vid_filter(uint32_t owner_id,
					    tpm_src_port_type_t src_port,
					    uint8_t vid_filter);

/*******************************************************************************
* tpm_sw_set_mac_age_time
*
* DESCRIPTION:
*       This function Sets the MAC address aging time.
*
* INPUTS:
*       owner_id   - APP owner id - should be used for all API calls.
*       time_out   - MAC address aging time
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_mac_age_time(uint32_t owner_id,
					 uint32_t time_out);

/*******************************************************************************
* tpm_sw_get_mac_age_time
*
* DESCRIPTION:
*       This function Sets the MAC address aging time.
*
* INPUTS:
*       owner_id   - APP owner id should be used for all API calls.
*
* OUTPUTS:
*       time_out   - time out value.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_mac_age_time(uint32_t owner_id,
					 uint32_t *time_out);

/*******************************************************************************
*  tpm_sw_set_mac_learn
*
* DESCRIPTION:
*       Enable/disable automatic learning of new source MAC addresses on port
*       ingress.
*
* INPUTS:
*       owner_id   - APP owner id , should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*       enable     - GT_TRUE for enable  or GT_FALSE otherwise
*
* OUTPUTS:
*       None
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_mac_learn(uint32_t owner_id,
				      tpm_src_port_type_t src_port,
				      bool enable);

/*******************************************************************************
*  tpm_sw_get_mac_learn
*
* DESCRIPTION:
*       Enable/disable automatic learning of new source MAC addresses on port
*       ingress.
*
* INPUTS:
*       owner_id   - APP owner id , should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       enable     - GT_TRUE for enable  or GT_FALSE otherwise
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_mac_learn(uint32_t owner_id,
				      tpm_src_port_type_t src_port,
				      bool *enable);

/*******************************************************************************
* tpm_sw_set_port_flooding()
*
* DESCRIPTION:      permit or not the flooding per port
*
* INPUTS:
*   owner_id    - APP owner id should be used for all API calls.
*   src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*   flood_mode  - flooding mode.
*   allow_flood - set to 1 = permit flooding of unknown DA.
*
* OUTPUTS:
*   None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_port_flooding(uint32_t owner_id,
					  tpm_src_port_type_t src_port,
					  tpm_flood_type_t flood_mode,
					  uint8_t allow_flood);

/*******************************************************************************
* tpm_sw_get_port_flooding()
*
* DESCRIPTION:      permit or not the flooding per port
*
* INPUTS:
*   owner_id    - APP owner id should be used for all API calls.
*   src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*   flood_mode  - flooding mode
*
* OUTPUTS:
*   allow_flood - set to 1 = permit flooding .
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_port_flooding(uint32_t owner_id,
					  tpm_src_port_type_t src_port,
					  tpm_flood_type_t flood_mode,
					  uint8_t *allow_flood);

/******************************************************************************/
/*                            MAC security APIs                               */
/******************************************************************************/

/******************************************************************************/
/********************************** Traffic management switch APIs ************/
/******************************************************************************/
/*******************************************************************************
* tpm_sw_set_uni_sched()
*
* DESCRIPTION:      Configures the scheduling mode per Ethernet port.
*
* INPUTS:
* owner_id          - APP owner id  should be used for all API calls.
* uni_port          - UNI port for setting the scheduling mode
* sched_mode        - scheduler mode per port
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_uni_sched(uint32_t owner_id,
				      tpm_src_port_type_t uni_port,
				      tpm_sw_sched_type_t sched_mode);

/*******************************************************************************
* tpm_sw_set_uni_q_weight()
*
* DESCRIPTION:      Configures the weight of a queue for all Ethernet UNI ports
*                   in the integrated switch.
*
* INPUTS:
* owner_id          - APP owner id  should be used for all API calls.
* queue_id          - queueId for setting the weight.
* weight            - weight value per queue.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_uni_q_weight(uint32_t owner_id,
					 uint8_t queue_id,
					 uint8_t weight);

/*******************************************************************************
* tpm_sw_set_uni_ingr_police_rate
*
* DESCRIPTION:
*       The API Configures an ingress policing function for an Ethernet UNI lport.
*
* INPUTS:
*       owner_id   - APP owner id, should be used for all API calls.
*       uni_port   - uni lport for configuring the ingress policer function.
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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_uni_ingr_police_rate(uint32_t owner_id,
						 tpm_src_port_type_t uni_port,
						 tpm_limit_mode_t count_mode,
						 uint32_t cir,
						 uint32_t cbs,
						 uint32_t ebs);

/*******************************************************************************
* tpm_sw_get_uni_ingr_police_rate
*
* DESCRIPTION:
*       The API gets an ingress policing function for an Ethernet UNI lport.
*
* INPUTS:
*       owner_id   - APP owner id, should be used for all API calls.
*       uni_port   - uni lport for configuring the ingress policer function.
*
*
* OUTPUTS:
*       count_mode - count mode:
*                               TPM_SW_LIMIT_FRAME
*                               TPM_SW_LIMIT_LAYER1
*                               TPM_SW_LIMIT_LAYER2
*                               TPM_SW_LIMIT_LAYER3
*       cir        - comited info rate.
*       cbs        - Committed Burst Size limit (expected to be 2kBytes)
*       ebs        - Excess Burst Size limit ( 0 ~ 0xFFFFFF)
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_uni_ingr_police_rate(uint32_t owner_id,
						 tpm_src_port_type_t uni_port,
						 tpm_limit_mode_t *count_mode,
						 uint32_t *cir,
						 uint32_t *cbs,
						 uint32_t *ebs);

/*******************************************************************************
* tpm_sw_set_uni_tc_ingr_police_rate()
*
* DESCRIPTION:      Configures an policer function for a traffic class.
*
* INPUTS:
* owner_id          - APP owner id  should be used for all API calls.
* uni_port          - UNI port.
* tc                - traffic class
* cir
* cbs
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_uni_tc_ingr_police_rate(uint32_t owner_id,
						    tpm_src_port_type_t uni_port,
						    uint32_t tc,
						    uint32_t cir,
						    uint32_t cbs);

/*******************************************************************************
* tpm_sw_set_uni_egr_rate_limit()
*
* DESCRIPTION:      Configures the egress rate limit of an Ethernet UNI port.
*
* INPUTS:
*       owner_id        - APP owner id  should be used for all API calls.
*       trg_port        - UNI port for fonfig the egress rate limit
*       limit_mode      - limit mode:  TPM_SW_LIMIT_FRAME,  TPM_SW_LIMIT_LAYER1,
*                                      TPM_SW_LIMIT_LAYER2, TPM_SW_LIMIT_LAYER3
*       rate_limit_val  - egress rate limit value
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_set_uni_egr_rate_limit(uint32_t wner_id,
					       tpm_src_port_type_t trg_port,
					       tpm_limit_mode_t limit_mode,
					       uint32_t rate_limit_val);

/*******************************************************************************
* tpm_sw_get_uni_egr_rate_limit
*
* DESCRIPTION:
*       The API return the egress frame rate limit of an Ethernet UNI lport
* INPUTS:
*       owner_id        - APP owner id , should be used for all API calls.
*       trg_lport        - uni lport for configuring the egress rate limit.

*
* OUTPUTS:
*      limit_mode      - limit mode:    TPM_SW_LIMIT_FRAME, TPM_SW_LIMIT_LAYER1,
*                                            TPM_SW_LIMIT_LAYER2, TPM_SW_LIMIT_LAYER3
*      rate_limit_val    - egress rate limit value
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       Valid rate_limit_val values are:
*                                    7600,..., 9600,
*                                    10000, 20000, 30000, 40000, ..., 100000,
*                                    110000, 120000, 130000, ..., 1000000.

*
*******************************************************************************/
tpm_error_code_t tpm_sw_get_uni_egr_rate_limit(uint32_t owner_id,
					       tpm_src_port_type_t trg_port,
					       tpm_limit_mode_t *limit_mode,
					       uint32_t *rate_limit_val);

/******************************************************************************/
/********************************** Switch PHY port management  APIs **********/
/******************************************************************************/

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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_phy_convert_port_index(uint32_t owner_id,
					    uint32_t switch_port,
					    tpm_src_port_type_t *extern_port);

/*******************************************************************************
* tpm_phy_set_port_autoneg_mode
*
* DESCRIPTION:
*       The API Configures the auto negotiation state of an Ethernet  lport.
* INPUTS:
*       owner_id       - APP owner id should be used for all API calls.
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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
tpm_error_code_t tpm_phy_set_port_autoneg_mode(uint32_t owner_id,
					       tpm_src_port_type_t src_port,
					       bool autoneg_state,
					       tpm_autoneg_mode_t autoneg_mode);

/*******************************************************************************
* tpm_phy_get_port_autoneg_mode
*
* DESCRIPTION:
*       The API return the the auto negotiation state of an Ethernet  lport.
* INPUTS:
*       owner_id       - APP owner id should be used for all API calls.
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
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
tpm_error_code_t tpm_phy_get_port_autoneg_mode(uint32_t owner_id,
					       tpm_src_port_type_t src_port,
					       bool *autoneg_state,
					       tpm_autoneg_mode_t *autoneg_mode);

/*******************************************************************************
* tpm_phy_restart_port_autoneg
*
* DESCRIPTION:
*       The API restart the auto negotiation of an Ethernet  lport.
* INPUTS:
*       owner_id   - APP owner id should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*
*
* OUTPUTS:
*        NONE.
*                                .
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
tpm_error_code_t tpm_phy_restart_port_autoneg(uint32_t owner_id,
					      tpm_src_port_type_t src_port);

/*******************************************************************************
* tpm_phy_set_port_admin_state
*
* DESCRIPTION:
*       The API Configures the PHY port  state of an Ethernet  lport.
* INPUTS:
*       owner_id        - APP owner id should be used for all API calls.
*       src_port        - Source port in UNI port index, UNI0, UNI1...UNI4.
*       phy_port_state  - PHY port  state to set.
*                         0:normal state
*                         1:power down
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
tpm_error_code_t tpm_phy_set_port_admin_state(uint32_t owner_id,
					      tpm_src_port_type_t src_port,
					      bool phy_port_state);

/*******************************************************************************
* tpm_phy_get_port_admin_state
*
* DESCRIPTION:
*       The API return the PHY port  state of an Ethernet  lport.
* INPUTS:
*       owner_id        - APP owner id should be used for all API calls.
*       src_port        - Source port in UNI port index, UNI0, UNI1...UNI4.
*
*
* OUTPUTS:
*       phy_port_state  -  0:normal state
*                          1:power down                                    .
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
tpm_error_code_t tpm_phy_get_port_admin_state(uint32_t owner_id,
					      tpm_src_port_type_t src_port,
					      bool *phy_port_state);

/*******************************************************************************
* tpm_phy_get_port_link_status
*
* DESCRIPTION:
*       The API return realtime port link status of an Ethernet  lport.
* INPUTS:
*       owner_id    - APP owner id should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
*
* OUTPUTS:
*       port_link_status  -  0:port link is ON
*                            1:port link is DOWN                                    .
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
tpm_error_code_t tpm_phy_get_port_link_status(uint32_t owner_id,
					      tpm_src_port_type_t src_port,
					      bool *port_link_status);

/*******************************************************************************
* tpm_phy_get_port_duplex_status
*
* DESCRIPTION:
*       The API return realtime port duplex status of an Ethernet  lport.
* INPUTS:
*       owner_id    - APP owner id should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
*
* OUTPUTS:
*       port_duplex_status  -  0:half deplex mode
*                              1:full deplex mode                    .
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
tpm_error_code_t tpm_phy_get_port_duplex_status(uint32_t owner_id,
						tpm_src_port_type_t src_port,
						bool *port_duplex_status);

/*******************************************************************************
* tpm_phy_get_port_speed_mode
*
* DESCRIPTION:
*       The API return realtime port speed mode of an Ethernet  lport.
* INPUTS:
*       owner_id    - APP owner id should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
*
* OUTPUTS:
*       port_duplex_status  -  0:10M
*                              1:100M
*                              2:1000M
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*
*******************************************************************************/
tpm_error_code_t tpm_phy_get_port_speed_mode(uint32_t owner_id,
					     tpm_src_port_type_t src_port,
					     uint32_t *speed);

/*******************************************************************************
* tpm_phy_set_port_flow_control_support
*
* DESCRIPTION:
*       This routine will set the pause bit in Autonegotiation Advertisement
*        Register. And restart the autonegotiation.
*
* INPUTS:
*       owner_id    - APP owner id should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*       state       - false:port pause is off.
*                     true:port pause is on.
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
* COMMENTS:
* data sheet register 4.10 Autonegotiation Advertisement Register
*******************************************************************************/

tpm_error_code_t tpm_phy_set_port_flow_control_support(uint32_t owner_id,
						       tpm_src_port_type_t src_port,
						       bool state);

/*******************************************************************************
* tpm_phy_get_port_flow_control_support
* DESCRIPTION:
*       This routine will get the pause bit in Autonegotiation Advertisement
*        Register.
*
* INPUTS:
*       owner_id   - APP owner id should be used for all API calls.
*       src_port   - Source port in UNI port index, UNI0, UNI1...UNI4.
*
* OUTPUTS:
*       state      -  false:port pause is off.
*                     true:port pause is on.
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
* COMMENTS:
* data sheet register 4.10 Autonegotiation Advertisement Register
*******************************************************************************/

tpm_error_code_t tpm_phy_get_port_flow_control_support(uint32_t owner_id,
						       tpm_src_port_type_t src_port,
						       bool *state);

/*******************************************************************************
* tpm_phy_get_port_flow_control_state
*
* DESCRIPTION:
*     This routine will get the current pause state.
*        Register.
*
* INPUTS:
*       owner_id    - APP owner id should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*
*
* OUTPUTS:
*       state       -
*                   false: MAC Pause not implemented in the link partner or in MyPause
*                   rue:  MAC Pause is implemented in the link partner and in MyPause
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
* COMMENTS:
*       None.
*******************************************************************************/
tpm_error_code_t tpm_phy_get_port_flow_control_state(uint32_t owner_id,
						     tpm_src_port_type_t src_port,
						     bool *state);

/*******************************************************************************
* tpm_phy_set_port_loopback
*
*
* INPUTS:
*       owner_id    - APP owner id should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*       mode        - Internal or external loopback
*       enable      - If true, enable loopback mode
*                     If false, disable loopback mode
*
* OUTPUTS:
* None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
* data sheet register 0.14 - Loop_back
*
*******************************************************************************/
tpm_error_code_t tpm_phy_set_port_loopback(uint32_t owner_id,
					   tpm_src_port_type_t src_port,
					   tpm_phy_loopback_mode_t mode,
					   bool enable);

/*******************************************************************************
* tpm_phy_get_port_loopback
*
* INPUTS:
*       owner_id    - APP owner id should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*       mode        - Internal or external loopback
*
* OUTPUTS:
*       enable      - If GT_TRUE,  loopback mode is enabled
*                     If GT_FALSE,  loopback mode is disabled
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
* data sheet register 0.14 - Loop_back
*
*******************************************************************************/
tpm_error_code_t tpm_phy_get_port_loopback(uint32_t owner_id,
					   tpm_src_port_type_t src_port,
					   tpm_phy_loopback_mode_t mode,
					   bool *enable);

/*******************************************************************************
* tpm_phy_set_port_duplex_mode
*
* DESCRIPTION:
*        Sets duplex mode for a specific logical port. This function will keep
*        the speed and loopback mode to the previous value, but disable others,
*        such as Autonegotiation.
*
* INPUTS:
*       owner_id    - APP owner id should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.
*       enable      - Enable/Disable dulpex mode
*
* OUTPUTS:
*       None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*         data sheet register 0.8 - Duplex Mode
*
*******************************************************************************/
tpm_error_code_t tpm_phy_set_port_duplex_mode(uint32_t owner_id,
					      tpm_src_port_type_t src_port,
					      bool enable);

/*******************************************************************************
* tpm_phy_get_port_duplex_mode
*
* DESCRIPTION:
*         Gets duplex mode for a specific logical port.
*
* INPUTS:
*       owner_id    - APP owner id should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.

*
* OUTPUTS:
*       enable      - Enable/Disable dulpex mode
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*         data sheet register 0.8 - Duplex Mode
*
*******************************************************************************/
tpm_error_code_t tpm_phy_get_port_duplex_mode(uint32_t owner_id,
					      tpm_src_port_type_t src_port,
					      bool *enable);

/*******************************************************************************
* tpm_phy_set_port_speed
*
* DESCRIPTION:
*       This routine will disable auto-negotiation and set the PHY port speed .
*
* INPUTS:
*       owner_id    - APP owner id should be used for all API calls.
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
* On success - TPM_RC_OK
* On error different types are returned according to the case, see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_phy_set_port_speed(uint32_t owner_id,
					tpm_src_port_type_t src_port,
					tpm_phy_speed_t speed);

/*******************************************************************************
* tpm_phy_get_port_speed
*
* DESCRIPTION:
*       This routine will get current PHY port speed .
*
* INPUTS:
*       owner_id    - APP owner id should be used for all API calls.
*       src_port    - Source port in UNI port index, UNI0, UNI1...UNI4.

*
*
* OUTPUTS:
*        speed      -    PHY_SPEED_10_MBPS   -10Mbps
*                        PHY_SPEED_100_MBPS  -100Mbps
*                        PHY_SPEED_1000_MBPS -1000Mbps.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_phy_get_port_speed(uint32_t owner_id,
					tpm_src_port_type_t src_port,
					tpm_phy_speed_t *speed);

/******************************************************************************/
/**                       Traffic management packet processor APIs           **/
/******************************************************************************/

/*******************************************************************************
* tpm_tm_set_wan_egr_queue_sched()
*
* DESCRIPTION:      Configures the scheduling mode per WAN queue.
*
* INPUTS:
* owner_id          - APP owner id  should be used for all API calls.
* sched_ent         - entity for setting the scheduling mode: ex:TPM_TRG_PORT_WAN
* sched_mode        - scheduler mode per port: strict(0) / wrr(1)
* queue_id          - queue number
* wrr_weight        - weight value when WRR scheduling (1-256)
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_tm_set_wan_egr_queue_sched(uint32_t owner_id,
						tpm_trg_port_type_t sched_ent,
						tpm_pp_sched_type_t sched_mode,
						uint8_t queue_id,
						uint16_t wrr_weight);

/*******************************************************************************
* tpm_tm_set_wan_ingr_queue_sched()
*
* DESCRIPTION:      Configures the scheduling mode per all downstream traffic from the WAN.
*
* INPUTS:
* owner_id          - APP owner id  should be used for all API calls.
* sched_mode        - scheduler mode per port: strict(0) / wrr(1)
* queue_id          - queue number
* wrr_weight        - weight value when WRR scheduling (1-256)
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_tm_set_wan_ingr_queue_sched(uint32_t owner_id,
						 tpm_pp_sched_type_t sched_mode,
						 uint8_t queue_id,
						 uint16_t wrr_weight);

/*******************************************************************************
* tpm_tm_set_wan_sched_egr_rate_lim()
*
* DESCRIPTION:      Configures the egress rate limit of upstream traffic.
*
* INPUTS:
* owner_id          - APP owner id  should be used for all API calls.
* sched_ent         - entity for setting the rate limit: ex:TPM_TRG_PORT_WAN
* rate_limit_val    - egress rate limit value
* bucket_size       - bucket size value
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_tm_set_wan_sched_egr_rate_lim(uint32_t owner_id,
						   tpm_trg_port_type_t sched_ent,
						   uint32_t rate_limit_val,
						   uint32_t bucket_size);

/*******************************************************************************
* tpm_tm_set_wan_queue_egr_rate_lim()
*
* DESCRIPTION:      Configures the upstream traffic egress rate limit for a specific
*                   queue of an upstream scheduling entity.
*
* INPUTS:
* owner_id          - APP owner id  should be used for all API calls.
* sched_ent         - entity for setting the rate limit: ex:TPM_TRG_PORT_WAN
* queue_id          - queue number
* rate_limit_val    - egress rate limit value
* bucket_size       - bucket size value
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_tm_set_wan_queue_egr_rate_lim(uint32_t owner_id,
						   tpm_trg_port_type_t sched_ent,
						   uint32_t queue_id,
						   uint32_t rate_limit_val,
						   uint32_t bucket_size);

/*******************************************************************************
* tpm_tm_set_wan_ingr_rate_lim()
*
* DESCRIPTION:      Configures the rate limit of all downstream traffic from the WAN.
*
* INPUTS:
* owner_id          - APP owner id  should be used for all API calls.
* rate_limit_val    - ingress rate limit value
* bucket_size       - bucket size value
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_tm_set_wan_ingr_rate_lim(uint32_t owner_id,
					      uint32_t rate_limit_val,
					      uint32_t bucket_size);

/*******************************************************************************
* tpm_tm_set_wan_q_ingr_rate_lim()
*
* DESCRIPTION:      Configures the egress rate limit of a specific queue for
*                   downstream traffic.
*
* INPUTS:
* owner_id          - APP owner id  should be used for all API calls.
* queue_id          - queue number
* rate_limit_val    - ingress rate limit value
* bucket_size       - bucket size value
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_tm_set_wan_q_ingr_rate_lim(uint32_t owner_id,
						uint32_t queue_id,
						uint32_t rate_limit_val,
						uint32_t bucket_size);

/*******************************************************************************
* tpm_alarm_get_eth_port
*
* DESCRIPTION:
*       This function gets eth port alarm.
*
* INPUTS:
*       owner_id     - APP owner id , should be used for all API calls.
*
* OUTPUTS:
*       alarm_type  - eth alarm type
*         port_bitmap    - port bitmap
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
tpm_error_code_t tpm_alarm_get_eth_port(uint32_t owner_id,
					uint32_t *alarm_type,
					uint8_t *port_bitmap);

/*******************************************************************************
* tpm_sw_pm_1_read
*
*
* INPUTS:
*      owner_id          - APP owner id  should be used for all API calls.
*       port              - The logical port number
*      tpm_swport_pm_3   - Holds PM data
*
* OUTPUTS:
* PM data is supplied structure.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_pm_1_read(uint32_t owner_id,
				  tpm_src_port_type_t port,
				  tpm_swport_pm_1_t *tpm_swport_pm_1);

/*******************************************************************************
* tpm_sw_pm_3_read
*
*
* INPUTS:
*      owner_id          - APP owner id  should be used for all API calls.
*       port              - The logical port number
*      tpm_swport_pm_3_all_t   - Holds PM data
*
* OUTPUTS:
* PM data is supplied structure.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_pm_3_read(uint32_t owner_id,
				  tpm_src_port_type_t port,
				  tpm_swport_pm_3_all_t *tpm_swport_pm_3);

/*******************************************************************************
* tpm_sw_clear_port_counter
*
* DESCRIPTION:
*       The API clear port pm counter.
*
* INPUTS:
*       owner_id   - APP owner id should be used for all API calls.
*
*
* OUTPUTS:
*       none.
*
* RETURNS:
* On success - TPM_RC_OK
* On error different types are returned according to the case - see tpm_error_code_t.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
tpm_error_code_t tpm_sw_clear_port_counter(uint32_t owner_id,
					   tpm_src_port_type_t port);
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
tpm_error_code_t	tpm_add_ctc_cm_acl_rule(uint32_t	owner_id,
        tpm_src_port_type_t   src_port,
        uint32_t              precedence,
        tpm_parse_fields_t    l2_parse_rule_bm,
        tpm_parse_fields_t    ipv4_parse_rule_bm,
        tpm_l2_acl_key_t      *l2_key,
        tpm_ipv4_acl_key_t    *ipv4_key,
        tpm_pkt_frwd_t        *pkt_frwd,
        tpm_pkt_action_t      pkt_act,
        uint32_t              pbits);

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
* It is APIs caller responsibility to maintain the correct precedence of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_del_ctc_cm_acl_rule(uint32_t owner_id,
	tpm_src_port_type_t   src_port,
	uint32_t			  precedence);

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
					      uint32_t pbits);
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
				     		  tpm_ctc_cm_ipv6_parse_win_t ipv6_parse_window);
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
tpm_error_code_t tpm_flush_vtu(uint32_t owner_id);
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
tpm_error_code_t tpm_flush_atu(uint32_t owner_id, tpm_flush_atu_type_t flush_type, uint16_t db_num);

/*******************************************************************************
* tpm_add_mac_learn_rule()
*
* DESCRIPTION:      Add a PNC L2 rule for source MAC learn
*
* INPUTS:
* owner_id           - ID of an application which requests ownership on a group of APIs.
* rule_num           - Entry index to be added in the current ACL
* queue              - GMAC1 queue send packet to
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
tpm_error_code_t tpm_add_mac_learn_rule(uint32_t owner_id, tpm_l2_acl_key_t *src_mac_addr);

/*******************************************************************************
* tpm_del_mac_learn_rule()
*
* DESCRIPTION:      Del a PNC L2 rule for source MAC learn
*
* INPUTS:
* owner_id           - ID of an application which requests ownership on a group of APIs.
* rule_idx           - Unique rule identification number, which is used when deleting the rule.
*                      (this is not the rule_num)
* OUTPUTS:
*
* RETURNS:
* On success, the function returns API_OWNERSHIP_SUCCESS. On error, see tpm_api_ownership_error_t.
*
* COMMENTS: none
*
*******************************************************************************/
tpm_error_code_t tpm_del_mac_learn_rule(uint32_t owner_id, tpm_l2_acl_key_t *src_mac_addr);

/*******************************************************************************
* tpm_mac_learn_default_rule_act_set()
*
* DESCRIPTION:      Set the action for mac learn default rule
*
* INPUTS:
* owner_id           - ID of an application which requests ownership on a group of APIs.
* mac_conf           - MAC conf determine rule action for mac learn default.
* OUTPUTS:
*
* RETURNS:
* On success, the function returns API_OWNERSHIP_SUCCESS. On error, see tpm_api_ownership_error_t.
*
* COMMENTS: none
*
*******************************************************************************/
tpm_error_code_t tpm_mac_learn_default_rule_act_set(uint32_t owner_id, tpm_unknown_mac_conf_t mac_conf);

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
tpm_error_code_t tpm_mac_learn_entry_num_get(uint32_t *entry_num);

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
						tpm_ds_load_balance_tgrt_t tgrt_port);

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
tpm_error_code_t tpm_del_ds_load_balance_rule(uint32_t owner_id, uint32_t rule_idx);

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
				    tpm_gmacs_enum_t active_wan);
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
				    tpm_eth_complex_profile_t profile_id);

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
					     uint32_t		 *switch_port);

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
						uint8_t		  enable);

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
tpm_error_code_t tpm_sw_port_add_vid_set_egrs_mode (uint32_t            owner_id,
                                                    tpm_src_port_type_t port,
                                                    uint16_t            vid,
                                                    uint8_t             eMode);
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
tpm_error_code_t tpm_active_tcont(uint32_t tcont_num);
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
tpm_error_code_t tpm_deactive_tcont(uint32_t tcont_num);

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
);


/* OLD API functions */
#define tpm_add_l2_prim_acl_rule                            tpm_add_l2_rule
#define tpm_add_l3_type_acl_rule                            tpm_add_l3_type_rule
#define tpm_add_ipv4_acl_rule                               tpm_add_ipv4_rule
#define tpm_add_ipv6_step1_acl_rule                         tpm_add_ipv6_gen_rule
#define tpm_add_ipv6_dip_acl_rule                           tpm_add_ipv6_dip_rule
#define tpm_add_ipv6_nh_acl_rule                            tpm_add_ipv6_nh_rule
#define tpm_add_ipv6_l4_ports_acl_rule                      tpm_add_ipv6_l4_ports_rule

#define tpm_del_l2_prim_acl_rule(owner, src, idx)           tpm_del_l2_rule(owner, idx)
#define tpm_del_l3_type_acl_rule(owner, src, idx)           tpm_del_l3_type_rule(owner, idx)
#define tpm_del_ipv4_acl_rule(owner, src, idx)              tpm_del_ipv4_rule(owner, idx)
#define tpm_del_ipv6_step1_acl_rule(owner, src, idx)        tpm_del_ipv6_gen_rule(owner, idx)
#define tpm_del_ipv6_dip_acl_rule(owner, src, idx)          tpm_del_ipv6_dip_rule(owner, idx)
#define tpm_del_ipv6_nh_acl_rule                            tpm_del_ipv6_nh_rule
#define tpm_del_ipv6_l4_ports_acl_rule(owner, src, idx)     tpm_del_ipv6_l4_ports_rule(owner, idx)

#ifdef __cplusplus
}
#endif
#endif
