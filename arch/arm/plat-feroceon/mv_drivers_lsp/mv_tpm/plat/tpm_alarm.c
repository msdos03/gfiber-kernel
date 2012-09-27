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
* tpm_alarm.c
*
*
*  MODULE : TPM Alarm
*
*  DESCRIPTION : This file config tpm alarm
*
*  MODIFICATION HISTORY:
*           12Dec2010   Victor  - initial version created.
*
* FILE REVISION NUMBER:
*       $Revision: 1.8 $
*******************************************************************************/

#include "tpm_common.h"
#include "tpm_header.h"

extern  uint32_t trace_sw_dbg_flag; 
static uint32_t gportLosPreviousAlarm = 0;

/*******************************************************************************
* tpm_alarm_get_eth_port
*
* DESCRIPTION:
*       This function gets eth port alarm.
*
* INPUTS:
*       owner_id	 - APP owner id , should be used for all API calls.
*
* OUTPUTS:
*       alarm_type  - eth alarm type
*		 port_bitmap	- port bitmap
* RETURNS:
*       On success -  TPM_RC_OK. 
*       On error different types are returned according to the case , see tpm_error_code_t.
*
* COMMENTS:
*        NONE.
*
*******************************************************************************/
tpm_error_code_t tpm_alarm_get_eth_port
(
	uint32_t owner_id,
	uint32_t *alarm_type,
	uint8_t *port_bitmap
)
{
	tpm_error_code_t retVal = TPM_RC_OK;
	uint32_t i;
	uint32_t ethPreviousAlarmState;
	uint32_t ethCurrentAlarmState;
	uint32_t ethChangedAlarm;
	uint8_t reportBitmap;
	uint8_t clearBitmap;

	if (trace_sw_dbg_flag)
	{
		printk(KERN_INFO"==ENTER==%s: owner_id[%d]",
			 	__FUNCTION__,owner_id);
	}

	*alarm_type = 0;
	
	/*Get eth port los alarm*/
	if(ONU_EPON_ALARM_ETH_DEF_STATE & ONU_EPON_ALARM_ETH_PORT_LOS)
	{
		/* get the prvious eth port los alarm status */
		ethPreviousAlarmState = gportLosPreviousAlarm;

		/* get the current eth port los alarm status */
		retVal = mv_switch_get_port_los_alarm(&ethCurrentAlarmState);
		if (retVal != TPM_RC_OK)
		{
			printk(KERN_ERR
			"%s:%d: function failed\r\n", __FUNCTION__,__LINE__);
		}

		/*port alarm exists, report or clear*/
		if((ethCurrentAlarmState ^ ethPreviousAlarmState) != 0)
		{
			if (trace_sw_dbg_flag)
			{
				printk(KERN_INFO"%s: owner_id[%d]  ethPreviousAlarmState[%08x] ethCurrentAlarmState[%08x]\r\n",
					 	__FUNCTION__,owner_id, ethPreviousAlarmState, ethCurrentAlarmState);
			}
			
			/*get changed alarm*/
			ethChangedAlarm = (ethCurrentAlarmState ^ ethPreviousAlarmState);

			/* check port los alarm*/	
			reportBitmap = 0;
			clearBitmap = 0;
			for(i=0; i<TPM_MAX_NUM_ETH_PORTS; i++)
			{
				if(ethChangedAlarm & (1 << i))
				{
					/*save report port los alarm*/
					if(ethCurrentAlarmState & (1 << i))
					{
						reportBitmap |= (1 << i);
					}
					else /*save clear port los alarm*/
					{
						clearBitmap |= (1 << i);
					}
				}
			}
			
			/*if report port los alarm exists*/
			if(reportBitmap)
			{
				*alarm_type |= ONU_EPON_ALARM_ETH_PORT_LOS;
				port_bitmap[MAX_ETH_PORT_ALARM+ONU_EPON_ALARM_ETH_PORT_LOS-1] = reportBitmap;
			}
			
			/*if clear port los alarm exists*/
			if(clearBitmap)
			{
				*alarm_type |= ONU_EPON_ALARM_ETH_PORT_LOS_CLEAR;
				port_bitmap[ONU_EPON_ALARM_ETH_PORT_LOS-1] = clearBitmap;
			}
		}

		/* save current eth port los alarm status */
		gportLosPreviousAlarm = ethCurrentAlarmState;
	}
	
	if (trace_sw_dbg_flag)
	{
		printk(KERN_INFO
			"==EXIT== %s: alarm_type=%08x, port_bitmap[%02x][%02x]\n\r",__FUNCTION__, *alarm_type,port_bitmap[ONU_EPON_ALARM_ETH_PORT_LOS-1],
			port_bitmap[MAX_ETH_PORT_ALARM+ONU_EPON_ALARM_ETH_PORT_LOS-1] );
	}

	return retVal;
}
