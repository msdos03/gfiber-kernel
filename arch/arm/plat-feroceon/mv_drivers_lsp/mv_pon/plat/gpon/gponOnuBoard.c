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
**  FILE        : gponOnuBoard.c                                             **
**                                                                           **
**  DESCRIPTION : This file implements ONU Board specific                    **
*******************************************************************************
*                                                                             *
*  MODIFICATION HISTORY:                                                      *
*                                                                             *
*   29Oct06  Oren Ben Hayun   created                                         *
* =========================================================================== *
******************************************************************************/

/* Include Files
------------------------------------------------------------------------------*/
#include "ponOnuHeader.h"

/* Local Constant
------------------------------------------------------------------------------*/

/* Global Variables
------------------------------------------------------------------------------*/

/* Global functions
------------------------------------------------------------------------------*/

/* Local Variables
------------------------------------------------------------------------------*/

/* Export Functions
------------------------------------------------------------------------------*/

/* Local Functions
------------------------------------------------------------------------------*/

/*******************************************************************************
**
**  onuGponDisableSnSetHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function saves disable SN state to non-volatile memory
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR
**
*******************************************************************************/
MV_STATUS onuGponDisableSnSetHandler(MV_BOOL state)
{
  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponDisableSnGetHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function extract disable SN state from non-volatile memory
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR
**
*******************************************************************************/
MV_STATUS onuGponDisableSnGetHandler(MV_BOOL *state)
{
  *state = MV_FALSE;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponPasswordHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function extract password from external source
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR
**
*******************************************************************************/
MV_STATUS onuGponPasswordHandler(MV_U8 *password)
{
  MV_U8 defaultPassword[10] = ONU_GPON_PASSWORD_DEFAULT;

  memcpy(password, defaultPassword, 10);

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponSerialNumberHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function extract serial number from external source
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR
**
*******************************************************************************/
MV_STATUS onuGponSerialNumberHandler(MV_U8 *serialNumber)
{
  MV_U8 defaultSerialNumber[8] = ONU_GPON_SN_DEFAULT;

  memcpy(serialNumber, defaultSerialNumber, 8);

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponAdminStateSet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure PON Admin state
**
**  PARAMETERS:  mode 0-Enable, 1 Disable
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR
**
*******************************************************************************/
MV_STATUS onuGponAdminStateSet(MV_U32 mode)
{
  MV_STATUS status;

  status = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN, mode, 0);
  if (status != MV_OK)
	return(status);

  return(MV_OK);
}


#ifndef PON_FPGA
/*******************************************************************************
**
**  onuGponSerdesPowerUpSeq
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function set serdes
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuGponSerdesPowerUpSeq(void)
{
  MV_STATUS status;
  MV_U32    rxReady = 0;
  MV_U32    txReady = 0;
  MV_U32    initDone;
  MV_U32    temp;

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0_RST, 0x1, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0_PU_Pll, 0x0, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0_PU_RX, 0x0, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0_PU_TX, 0x0, 0);
  if (status != MV_OK)
    return(status);

  // asicOntMiscRegRead(mvAsicReg_PON_SERDES_PHY_CTRL_0, &temp, 0);
  // temp &= ~(0x7);
  // status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0, temp, 0);
  // if (status != MV_OK)
  //   return(status);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0_SEL_GEPON, 0x0, 0);
 if (status != MV_OK)
   return(status);

  mvOsDelay(40);
  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0_REF_CLK_25M, 0x1, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0_RST, 0x0, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  asicOntMiscRegRead(mvAsicReg_PON_SERDES_INTERNAL_PASSWORD, &temp, 0);
  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_INTERNAL_PASSWORD, temp | 0x76, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_INTERNAL_EN_LOOP_TIMING, 0x1, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_INTERNAL_PON_SELECT, 0x0, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  // status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0_PU_Pll, 0x1, 0);
  // if (status != MV_OK)
  //   return(status);
  //
  // mvOsDelay(40);
  //
  // status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0_PU_RX, 0x1, 0);
  // if (status != MV_OK)
  //   return(status);
  //
  // mvOsDelay(10);
  //
  // status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0_PU_TX, 0x1, 0);
  // if (status != MV_OK)
  //   return(status);

  asicOntMiscRegRead(mvAsicReg_PON_SERDES_PHY_CTRL_0, &temp, 0);
  temp |= (0x7);
  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0, temp, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  temp = 0;
  do
  {
	temp++;

	status  = asicOntMiscRegRead(mvAsicReg_PON_SERDES_PHY_CTRL_0_READY_TX, &txReady, 0);
    if (status != MV_OK)
      return(status);

    mvOsDelay(40);

    status  = asicOntMiscRegRead(mvAsicReg_PON_SERDES_PHY_CTRL_0_READY_RX, &rxReady, 0);
    if (status != MV_OK)
      return(status);

	if ((temp % 10) == 0)
	{
	  return (MV_FAIL);
	}

  } while ((txReady == 0) || (rxReady == 0));

  mvOsDelay(40);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0_RX_INIT, 0x1, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  temp = 0;
  do
  {
	temp++;

	status  = asicOntMiscRegRead(mvAsicReg_PON_SERDES_PHY_CTRL_0_INIT_DONE, &initDone, 0);
    if (status != MV_OK)
      return(status);

	if ((temp % 10) == 0)
	{
	  return (MV_FAIL);
	}

   mvOsDelay(40);

  } while (initDone == 0);

  mvOsDelay(40);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0_RX_INIT, 0x0, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponSerdesInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function set serdes
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuGponSerdesInit(void)
{
  MV_STATUS status;
  MV_U32    loop = 0;

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_CLK_EN, 0x0, 0);
  if (status != MV_OK)
   return(status);

  mvOsDelay(40);
  status  = asicOntMiscRegWrite(mvAsicReg_PON_MAC_GPON_CLK_EN, 0x0, 0);
  if (status != MV_OK)
   return(status);

  mvOsDelay(40);

  /* PON MAC init to EPON mode */
  /* ========================= */

  /* Put PON MAC to Reset */
  status = asicOntMiscRegWrite(mvAsicReg_PON_MAC_SW_RESET_CTRL, 0x0, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  /* Switch to new PON MAC */
  status = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_XPON_CTRL, 1, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_CLK_SEL, 0x0, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_MAC_GPON_CLK_EN, 0x1, 0);
  if (status != MV_OK)
   return(status);

  mvOsDelay(40);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_CLK_EN, 0x1, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  /* Take PON MAC out of Reset */
  status = asicOntMiscRegWrite(mvAsicReg_PON_MAC_SW_RESET_CTRL, 0x1, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  /* PON MAC init to GPON mode */
  /* ========================= */

  /* Put PON MAC to Reset */
  status = asicOntMiscRegWrite(mvAsicReg_PON_MAC_SW_RESET_CTRL, 0x0, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  /* Switch to new PON MAC */
  status = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_XPON_CTRL, 0, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_CLK_SEL, 0x0, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_MAC_GPON_CLK_EN, 0x1, 0);
  if (status != MV_OK)
   return(status);

  mvOsDelay(40);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_CLK_EN, 0x1, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  /* Take PON MAC out of Reset */
  status = asicOntMiscRegWrite(mvAsicReg_PON_MAC_SW_RESET_CTRL, 0x1, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);


  /*    GPON  configuration of XVR SD is done by LSP code (mvBoardEnvSpec.h, mvCtrlEnvLib.c) */
  /* ================================= */

  /*    GPON configuration/SerDes power up and init sequence   */
  /* ========================================================= */

  do
  {
	loop++;

	status = onuGponSerdesPowerUpSeq();
	if      (status == MV_ERROR) return(status);
	else if (status == MV_OK)    break;

  } while (loop < 10);

  if (loop >= 10)
  {
	mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE, "======================================\n");
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE, "========= Serdes Not Sync !!! ========\n");
	mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE, "======================================\n");
  }

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0_RST_TX_DOUT, 0x0, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_INTERNAL_OPEN_TX_DOOR, 0x0, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_CLK_EN, 0x0, 0);
  if (status != MV_OK)
      return(status);

  mvOsDelay(40);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_CLK_SEL, 0x1, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_CLK_EN, 0x1, 0);
  if (status != MV_OK)
    return(status);

  mvOsDelay(40);

 // status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN, 0x0, 0);
 // if (status != MV_OK)
 //   return(status);
 //
 // mvOsDelay(10);

  return(MV_OK);
}

#if 0 /* only for pon mode switch without Reset */
/*******************************************************************************
**
**  onuGponSwitchOver
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function switch between EPON and GPON
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuGponSwitchOver(void)
{
  MV_STATUS status;

  /* STOP Traffic */


  /* Close TX EN */
  printk("onuPonSwitchOver - Close TX EN\n\r");
  status = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN, 0x1, 0);
  if (status != MV_OK)
    return(status);

  /* Put PON MAC to Reset */
  printk("onuPonSwitchOver - Put PON MAC to Reset\n\r");
  status = asicOntMiscRegWrite(mvAsicReg_PON_MAC_SW_RESET_CTRL, 0x0, 0);
  if (status != MV_OK)
    return(status);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_CLK_EN, 0x0, 0);
  if (status != MV_OK)
   return(status);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_CLK_SEL, 0x0, 0);
  if (status != MV_OK)
    return(status);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_CLK_EN, 0x1, 0);
  if (status != MV_OK)
    return(status);

  /* Switch to new PON MAC */
  printk("onuPonSwitchOver - Switch to new PON MAC - GPON\n\r");
  status = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_XPON_CTRL, 0, 0);
  if (status != MV_OK)
    return(status);

  /* Take PON MAC out of Reset */
  printk("onuPonSwitchOver - Take PON MAC out of Reset\n\r");
  status = asicOntMiscRegWrite(mvAsicReg_PON_MAC_SW_RESET_CTRL, 0x1, 0);
  if (status != MV_OK)
    return(status);

  /* Re-init Serdes */
  printk("onuPonSwitchOver - Re-init Serdes\n\r");

  printk("onuPonSwitchOver - Put PON MAC to Reset\n\r");
  status = asicOntMiscRegWrite(mvAsicReg_PON_MAC_SW_RESET_CTRL, 0x0, 0);
  if (status != MV_OK)
    return(status);


  printk("onuPonSerdeSwitchOverInit - Serdes CLK EN\n\r");
  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_CLK_EN, 0x0, 0);
  if (status != MV_OK)
      return(status);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_CLK_SEL, 0x1, 0);
  if (status != MV_OK)
    return(status);

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_CLK_EN, 0x1, 0);
  if (status != MV_OK)
    return(status);

  /* Take PON MAC out of Reset */
  printk("onuPonSwitchOver - Take PON MAC out of Reset\n\r");
  status = asicOntMiscRegWrite(mvAsicReg_PON_MAC_SW_RESET_CTRL, 0x1, 0);
  if (status != MV_OK)
	return(status);

  /* Open TX EN */
  printk("onuPonSwitchOver - Open TX EN\n\r");
  status = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN, 0x0, 0);
  if (status != MV_OK)
    return(status);

  return(MV_OK);
}
#endif /* 0 */

#endif /* PON_FPGA */
