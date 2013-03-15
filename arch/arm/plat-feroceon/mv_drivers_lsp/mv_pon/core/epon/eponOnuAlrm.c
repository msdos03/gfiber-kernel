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

/******************************************************************************/
/**                                                                          **/
/**  MODULE      : ONU EPON                                                  **/
/**                                                                          **/
/**  FILE        : eponOnuAlrm.c                                             **/
/**                                                                          **/
/**  DESCRIPTION : This file implements ONU EPON Alarm and Statistics        **/
/**                functionality                                             **/
/**                                                                          **/
/******************************************************************************
 *                                                                            *                              
 *  MODIFICATION HISTORY:                                                     *
 *                                                                            *
 *   26Jan10  oren_ben_hayun    created                                       *  
 * ========================================================================== *      
 *                                                                     
 ******************************************************************************/

/* Include Files
------------------------------------------------------------------------------*/
#include "eponOnuHeader.h"

/* Local Constant
------------------------------------------------------------------------------*/                                               
#define __FILE_DESC__ "mv_pon/core/epon/eponOnuAlrm.c"

/* Global Variables
------------------------------------------------------------------------------*/

/* Local Variables
------------------------------------------------------------------------------*/
MV_U32 onuEponPreviousAlarm = 0;
MV_U32 onuEponCurrentAlarm  = 0;  
MV_U32 onuEponLosAlarm      = 0;  


/* Export Functions
------------------------------------------------------------------------------*/

/* Local Functions
------------------------------------------------------------------------------*/
void onuEponAlarmProcess(void);

/******************************************************************************/
/* ========================================================================== */
/*                         Alarm Section                                      */
/* ========================================================================== */
/******************************************************************************/

/*******************************************************************************
**
**  onuEponAlarmSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu epon alarm 
**               
**  PARAMETERS:  MV_U32  alarm
**               MV_BOOL status
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK 
**                   
*******************************************************************************/
MV_STATUS onuEponAlarmSet(MV_U32 alarm, MV_BOOL state)
{
  if ((alarm & ONU_EPON_ALARM_MASK) == 0)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_ALARM_MODULE, 
               "ERROR: (%s:%d) invalid alarm(%d) type\n\r", __FILE_DESC__, __LINE__, alarm);
    return(MV_ERROR);
  }

  if (state == MV_FALSE)
  {
    onuEponCurrentAlarm &= ~(alarm);
  }
  else
  {
    onuEponCurrentAlarm |=  (alarm);
  }

  onuEponAlarmProcess();

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponAlarmProcess
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function process the current alarm state and notify screen
**               
**  PARAMETERS:  MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     None 
**                   
*******************************************************************************/
void onuEponAlarmProcess(void)
{
  MV_U32 onuEponPreviousAlarmState;
  MV_U32 onuEponCurrentAlarmState;
  MV_U32 onuEponChangeAlarm;

  /* get the ASIC prvious alarm status */
  onuEponPreviousAlarmState = onuEponPreviousAlarm;

  /* get the ASIC current alarm status */
  onuEponCurrentAlarmState = onuEponCurrentAlarm;

  /* Init PON los alarm */
  onuEponLosAlarm = 0;  
  
  /* alarm changed */
  if ((onuEponCurrentAlarmState ^ onuEponPreviousAlarmState) != 0)
  {
    onuEponChangeAlarm = (onuEponCurrentAlarmState ^ onuEponPreviousAlarmState);

    if ((onuEponChangeAlarm & onuEponCurrentAlarmState) != 0)
    {
      if (onuEponChangeAlarm & ONU_EPON_XVR_SD_MASK)
      {
        mvPonPrint(PON_PRINT_INFO, PON_ALARM_MODULE, 
                   "INFO: (%s:%d) === XVR SD Off ===\n\r", __FILE_DESC__, __LINE__);
        onuEponLosAlarm |= 0x01;
      }

      if (onuEponChangeAlarm & ONU_EPON_SERDES_SD_MASK)
      {
        mvPonPrint(PON_PRINT_INFO, PON_ALARM_MODULE, 
                   "INFO: (%s:%d) === SERDES SD Off ===\n\r", __FILE_DESC__, __LINE__);
        onuEponLosAlarm |= 0x02;           
      }
    }
    else /* ((onuEponChangeAlarm & onuEponCurrentAlarmState) == 0) */
    {
      LINKSTATUSFUNC linkStatusCallback;
      if (onuEponChangeAlarm & ONU_EPON_XVR_SD_MASK)
      {
        mvPonPrint(PON_PRINT_INFO, PON_ALARM_MODULE, 
                   "INFO: (%s:%d) === XVR SD On ===\n\r", __FILE_DESC__, __LINE__);
        onuEponLosAlarm |= 0x01<<8;           
      }

      if (onuEponChangeAlarm & ONU_EPON_SERDES_SD_MASK)
      {
        mvPonPrint(PON_PRINT_INFO, PON_ALARM_MODULE, 
                   "INFO: (%s:%d) === SERDES SD On ===\n\r", __FILE_DESC__, __LINE__);

        onuEponLosAlarm |= 0x02<<8;
      }
      /* TODO(kedong): There is no OAM message between ONU and OLT, thus the
       * link will be always down if we don't explicitly bring up the interface.
       * Here we hook the link status update code to the XVR alarm off code to
       * always indicate the link up if the XVR signal is detected. Once the
       * wavelength selection code is ready, we should remove the follow link
       * status check code.
       */
      linkStatusCallback = onuEponDbLinkStatusCallbackGet();
      if (linkStatusCallback != NULL)
      {
        linkStatusCallback(MV_TRUE);
      }
      onuPonTxLaserOn(MV_TRUE);
    }

    onuEponPreviousAlarm = onuEponCurrentAlarmState;  
  }
}

/*******************************************************************************
**
**  onuEponAlarmGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return current alarm status
**               
**  PARAMETERS:  MV_U32 *alarm
**
**  OUTPUTS:     None
**
**  RETURNS:     None 
**                   
*******************************************************************************/
void onuEponAlarmGet(MV_U32 *alarm)
{
  alarm[0] = ((onuEponCurrentAlarm & ONU_EPON_XVR_SD_MASK)    != 0) ? (1) : (0); 
  alarm[1] = ((onuEponCurrentAlarm & ONU_EPON_SERDES_SD_MASK) != 0) ? (1) : (0);
}

/*******************************************************************************
**
**  onuEponLosAlarmGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return current PON los alarm status
**               
**  PARAMETERS:  MV_U32 *alarm
**
**  OUTPUTS:     None
**
**  RETURNS:     None 
**                   
*******************************************************************************/
void onuEponLosAlarmGet(MV_U32 *alarm)
{
  *alarm = onuEponLosAlarm;

  /* Set PON los alarm to default value */
  //onuEponLosAlarm = 0;
}
