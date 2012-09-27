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
* mv_mac_learn_logic.h
*
* DESCRIPTION:
*
*
*******************************************************************************/
#ifndef __mv_mac_learn_logic_h__
#define __mv_mac_learn_logic_h__

/* GMAC MAC leanring ON
------------------------------------------------------------------------------*/
#define MAC_LEARN_ON_GMAC 0

/* Function protype
------------------------------------------------------------------------------*/
void mv_mac_learn_aging(unsigned long mac_learn_aging);
void mv_mac_learn_pnc_rule_add(struct work_struct *work);
int32_t mv_mac_learn_logic_static_add(char *static_mac_addr);
int32_t mv_mac_learn_logic_static_del(char *static_mac_addr);

#endif