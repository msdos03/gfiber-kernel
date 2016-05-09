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
**  FILE        : ponOnuBoard.c                                              **
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
S_OnuP2PDb onuP2PDb_s;
t_ponXvrFuncPtr ponXvrFunc;

/* Global functions
------------------------------------------------------------------------------*/
MV_U32 prbsUserDefinedPattern[3] = {0xA5A5A5A5, /* [31:00] */
				    0xA5A5A5A5, /* [63:32] */
				    0xA5A5};    /* [79:64] */

/* Local Variables
------------------------------------------------------------------------------*/
extern spinlock_t onuPonIrqLock;

/* Export Functions
------------------------------------------------------------------------------*/

/* Local Functions
------------------------------------------------------------------------------*/
MV_STATUS onuPonPatternBurstEnable(bool on);

#ifndef PON_FPGA

MV_U32  time_interval_up   = 0;
MV_U32  time_interval_down = 0;
MV_BOOL transmit_up;

/*******************************************************************************
**
**  onuPonLedInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function init led operation
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR
**
*******************************************************************************/
MV_STATUS onuPonLedInit(void)
{
	MV_STATUS status;
	MV_U32    gpioGroup, gpioMask;

	PON_GPIO_GET(BOARD_GPP_SYS_LED, gpioGroup, gpioMask);
	if (gpioMask != PON_GPIO_NOT_USED) {
		status = mvGppTypeSet(gpioGroup, gpioMask, 0/*output*/);
		if (status != MV_OK)
			return(status);
	}

	PON_GPIO_GET(BOARD_GPP_PON_LED, gpioGroup, gpioMask);
	if (gpioMask != PON_GPIO_NOT_USED) {
		status = mvGppTypeSet(gpioGroup, gpioMask, 0/*output*/);
		if (status != MV_OK)
			return(status);
	}

	status  = asicOntMiscRegWrite(mvAsicReg_PON_LED_BLINK_FREQ_A_ON, 0x30000000, 0);
	if (status != MV_OK)
		return(status);

	status  = asicOntMiscRegWrite(mvAsicReg_PON_LED_BLINK_FREQ_A_OFF, 0x30000000, 0);
	if (status != MV_OK)
		return(status);

	status  = asicOntMiscRegWrite(mvAsicReg_PON_LED_BLINK_FREQ_B_ON, 0x15000000, 0);
	if (status != MV_OK)
		return(status);

	status  = asicOntMiscRegWrite(mvAsicReg_PON_LED_BLINK_FREQ_B_OFF, 0x15000000, 0);

	return(status);
}

/*******************************************************************************
**
**  onuPonLedHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function handles led operation
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR
**
*******************************************************************************/
MV_STATUS onuPonLedHandler(MV_U32 led, MV_U32 action)
{
	MV_STATUS status;
	MV_U32    gpioGroup = 0;
	MV_U32    gpioMask = 0;

	if (led == ONU_PON_SYS_LED) {

		PON_GPIO_GET(BOARD_GPP_SYS_LED, gpioGroup, gpioMask);
		if (gpioMask != PON_GPIO_NOT_USED) {
			status = mvGppValueSet(gpioGroup, gpioMask, gpioMask);
			if (status != MV_OK)
				return(status);
		}

	} else if (led == ONU_PON_SYNC_LED) {

		PON_GPIO_GET(BOARD_GPP_PON_LED, gpioGroup, gpioMask);
		if (gpioMask != PON_GPIO_NOT_USED) {
			switch (action) {
			case ACTIVE_LED_OFF:
				status  = mvGppValueSet(gpioGroup, gpioMask, ~gpioMask);
				if (status != MV_OK)
					return(status);
				break;

			case ACTIVE_LED_BLINK_SLOW:
				status  = mvGppValueSet(gpioGroup, gpioMask, gpioMask);
				if (status != MV_OK)
					return(status);
				status  = mvGppBlinkEn(gpioGroup, gpioMask, gpioMask);
				if (status != MV_OK)
					return(status);
				status  = asicOntMiscRegWrite(mvAsicReg_PON_SYNC_LED_BLINK_FREQ, 0, 0);
				if (status != MV_OK)
					return(status);
				break;

			case ACTIVE_LED_BLINK_FAST:
				status  = mvGppValueSet(gpioGroup, gpioMask, gpioMask);
				if (status != MV_OK)
					return(status);
				status  = mvGppBlinkEn(gpioGroup, gpioMask, gpioMask);
				if (status != MV_OK)
					return(status);
				status  = asicOntMiscRegWrite(mvAsicReg_PON_SYNC_LED_BLINK_FREQ, 1, 0);
				if (status != MV_OK)
					return(status);
				break;

			case ACTIVE_LED_ON:
				status  = mvGppValueSet(gpioGroup, gpioMask, gpioMask);
				if (status != MV_OK)
					return(status);
				status  = mvGppBlinkEn(gpioGroup, gpioMask, ~gpioMask);
				if (status != MV_OK)
					return(status);
				break;
			}
		}
	}

	return(MV_OK);
}

/*******************************************************************************
**
**  onuPonDyingGaspProcess
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function process dying gasp function, kill VoIP
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuPonDyingGaspProcess(void)
{
	MV_STATUS status;
	MV_U32 boardType;

	/* GPON/EPON configuration of Dying Gasp */
	/* ===================================== */
	boardType = mvBoardIdGet();

	switch (boardType) {
	case RD_88F6510_SFU_ID:
	case RD_88F6560_GW_ID:
	case RD_88F6530_MDU_ID:
		status  = asicOntMiscRegWrite(mvAsicReg_PON_MPP_24, 0, 0); /* power down voip */
		break;
	default:
		status = MV_ERROR;
	}

	return(status);
}

/*******************************************************************************
**
**  onuPonDyingGaspExit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function forces system reset after dying gasp
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuPonDyingGaspExit(void)
{
	if (mvBoardIdGet() == DB_88F6535_BP_ID) {
		mvPonPrint(PON_PRINT_INFO, PON_INIT_MODULE, "\n\n\n");
		mvPonPrint(PON_PRINT_INFO, PON_INIT_MODULE, "=========================================\n");
		mvPonPrint(PON_PRINT_INFO, PON_INIT_MODULE, "======= Dying Gasp Detected =============\n");
		mvPonPrint(PON_PRINT_INFO, PON_INIT_MODULE, "======= System Need To Reboot !!!!! =====\n");
		mvPonPrint(PON_PRINT_INFO, PON_INIT_MODULE, "=========================================\n");
	}

	return(MV_OK);
}

/*******************************************************************************
**
**  mvOnuPonMacPrbsUserDefinedPatternSet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function init User defined PRBS pattern
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
void mvOnuPonMacPrbsUserDefinedPatternSet(MV_U32 *prbsUserPattern)
{
  prbsUserDefinedPattern[0] = prbsUserPattern[0];
  prbsUserDefinedPattern[1] = prbsUserPattern[1];
  prbsUserDefinedPattern[2] = prbsUserPattern[2];
}

/*******************************************************************************
**
**  mvOnuPonMacPrbsUserDefinedPatternGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function return User defined PRBS pattern
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
void mvOnuPonMacPrbsUserDefinedPatternGet(MV_U32 *prbsUserPattern)
{
  prbsUserPattern[0] = prbsUserDefinedPattern[0];
  prbsUserPattern[1] = prbsUserDefinedPattern[1];
  prbsUserPattern[2] = prbsUserDefinedPattern[2];
}

/*******************************************************************************
**
**  mvOnuPonMacBurstEnableInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function init Burst Enable MPP
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS mvOnuPonMacBurstEnableInit(void)
{
	MV_U32    gpioGroup, gpioMask;
	MV_STATUS status = MV_OK;

	if (MV_6601_DEV_ID == mvCtrlModelGet())
	{
	       /* PHY control register - output status set */
	       status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN, ONU_PHY_OUTPUT, 0);
	       if (status != MV_OK) {
		       mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
				  "ERROR: asicOntMiscRegWrite failed for PON phy ctrl enable - output\n\r");
		       return(MV_ERROR);
	       }

	       /* Set SW BEN control for MC tranciever */
	       status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_SW_HW_SELECT, 1, 0);
	       if (status != MV_OK) {
		       mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
			"ERROR: asicOntMiscRegWrite failed for PON phy SW ctrl select\n\r");
		       return(MV_ERROR);
	       }

	       /* PHY control register - force disable */
	       status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_SW_FORCE, 0, 0);
	       if (status != MV_OK) {
		       mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
			     "ERROR: asicOntMiscRegWrite failed for PON phy SW ctrl not force\n\r");
		       return(MV_ERROR);
	       }
	}
	else
	{
		if (mvCtrlRevGet() == ONU_ASIC_REV_Z2) {

		/* KW2 ASIC Rev Z2 */
		/* =============== */
		PON_GPIO_GET(BOARD_GPP_PON_XVR_TX, gpioGroup, gpioMask);
		if (gpioMask == PON_GPIO_NOT_USED)
			return MV_ERROR;

		status = mvGppTypeSet(gpioGroup, gpioMask, ~gpioMask/* 0-output allow transsmit*/);

		if (status == MV_OK)
			status = mvGppValueSet(gpioGroup, gpioMask, ~gpioMask/*0-disable signal*/);
		else
			return(status);

		if (status == MV_OK)
			status = mvGppTypeSet(gpioGroup, gpioMask, gpioMask /* 0-input NOT allow transsmit*/);

		}
		else if (mvCtrlRevGet() == ONU_ASIC_REV_A0) {

		/* KW2 ASIC Rev A0 */
		/* =============== */
		/* PHY control register - output status set */
			status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN, ONU_PHY_OUTPUT, 0);
			if (status != MV_OK) {
				mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
					   "ERROR: asicOntMiscRegWrite failed for PON phy ctrl enable - output\n\r");
				return(MV_ERROR);
			}

			/* PHY control register - force disable */
			status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_FORCE_BEN_IO_EN, 0, 0);
			if (status != MV_OK) {
				mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
					   "ERROR: asicOntMiscRegWrite failed for PON phy ctrl not force\n\r");
				return(MV_ERROR);
			}
		}
	}

	return(status);
}

MV_STATUS EponXvrSDPolarityHighStatus(MV_U32 interruptStatus, MV_U32 statusMask)
{
    return ((interruptStatus & statusMask) ? MV_FALSE : MV_TRUE);
}

MV_STATUS EponXvrSDPolarityLowStatus(MV_U32 interruptStatus, MV_U32 statusMask)
{
    return ((~interruptStatus & statusMask) ? MV_FALSE : MV_TRUE);
}

MV_STATUS GponXvrSDPolarityHighStatus(MV_U32 interruptStatus, MV_U32 statusMask)
{
    return ((interruptStatus & statusMask) ? MV_TRUE : MV_FALSE);
}

MV_STATUS GponXvrSDPolarityLowStatus(MV_U32 interruptStatus, MV_U32 statusMask)
{
    return ((~interruptStatus & statusMask) ? MV_TRUE : MV_FALSE);
}

t_ponXvrFuncPtr funcEponXvrSDStatus(MV_U32 polarity)
{
    if (polarity == 0)
    {
        return EponXvrSDPolarityHighStatus;
    }
    else
    {
        return EponXvrSDPolarityLowStatus;
    }
}

t_ponXvrFuncPtr funcGponXvrSDStatus(MV_U32 polarity)
{
    if (polarity == 0)
    {
        return GponXvrSDPolarityHighStatus;
    }
    else
    {
        return GponXvrSDPolarityLowStatus;
    }
}

/*******************************************************************************
**
**  onuPonPatternBurstTransmit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function turn on or off actuacl burst transmission
**
**  PARAMETERS:  when on  == true  transmission is on
**               when off == false transmission is off
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuPonPatternBurstTransmit(MV_BOOL on)
{
	MV_STATUS status;
	MV_U32    gpioGroup, gpioMask;
	MV_U32    trans_value = 0;

	if (mvCtrlRevGet() == ONU_ASIC_REV_Z2) {

		/* ASIC Rev Z2 */
		/* =========== */
		PON_GPIO_GET(BOARD_GPP_PON_XVR_TX, gpioGroup, gpioMask);
		if (gpioMask == PON_GPIO_NOT_USED)
			return(MV_ERROR);

		trans_value = (on == MV_TRUE ? ~gpioMask /*0*/: gpioMask /*1*/);

		status = mvGppTypeSet(gpioGroup, gpioMask, trans_value);
		if (status != MV_OK)
			return(MV_ERROR);

	} else if ((mvCtrlRevGet() == ONU_ASIC_REV_A0) || (MV_6601_DEV_ID == mvCtrlModelGet())) {

		/* ASIC Rev A0 and MC */
		/* ================== */
		status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN, ONU_PHY_OUTPUT, 0);
		if (status != MV_OK) {
			mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
				   "ERROR: asicOntMiscRegWrite failed for PON phy ctrl enable - output\n\r");
			return(MV_ERROR);
		}
	}

	transmit_up = on;
	return(MV_OK);
}

/*******************************************************************************
**
**  onuPonPatternBurstEnable
**  ____________________________________________________________________________
**
**  DESCRIPTION:
**
**  PARAMETERS:  bool on
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuPonPatternBurstEnable(bool on)
{
	MV_STATUS status;
	MV_U32    gpioGroup, gpioMask;
	MV_U32    trans_value = 0;
	MV_U32    polarity;

	if (MV_6601_DEV_ID == mvCtrlModelGet())
	{
	       /* PHY control register - force enable */
	       status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_SW_FORCE, 1, 0);
	       if (status != MV_OK) {
		       mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
			     "ERROR: asicOntMiscRegWrite failed for PON phy SW ctrl force\n\r");
		       return(MV_ERROR);
	       }

	       /* PHY control register - force enable value - according to polarity */
	       status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_SW_VALUE, 1, 0);
	       if (status != MV_OK) {
		       mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
				  "ERROR: asicOntMiscRegWrite failed for PON phy ctrl SW force value %d\n\r",
				  trans_value);
		       return(MV_ERROR);
	       }

	}
	else
	{
		if (mvCtrlRevGet() == ONU_ASIC_REV_Z2) {

			/* ASIC Rev Z2 */
			/* =========== */
			PON_GPIO_GET(BOARD_GPP_PON_XVR_TX, gpioGroup, gpioMask);
			if (gpioMask == PON_GPIO_NOT_USED)
				return MV_ERROR;

			trans_value = ((on == MV_TRUE) ? (gpioMask/*1*/) : (~gpioMask/*0*/));

			status = mvGppValueSet(gpioGroup, gpioMask, trans_value);
			if (status != MV_OK)
				return(MV_ERROR);
		}
		else if	(mvCtrlRevGet() == ONU_ASIC_REV_A0) {

			/* ASIC Rev A0 */
			/* =========== */
			/* PHY control register - force enable */
			status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_FORCE_BEN_IO_EN, 1, 0);
			if (status != MV_OK) {
				mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
					   "ERROR: asicOntMiscRegWrite failed for PON phy ctrl force\n\r");
				return(MV_ERROR);
			}

			polarity = onuP2PDbXvrBurstEnablePolarityGet();

			/* XVR polarity */
			/* XVR polarity == 0, Active High, transmit 1 to the line  */
			/* XVR polarity == 1, Active Low, transmit 0 to the line  */

			/* P2P mode */
			/* Force Value == 0, transmit 0 to the line  */
			/* Force Value == 1, transmit 1 to the line  */

			/* Setting P2P should be reversed from XVR polarity */
			/* XVR polarity == 0, Active High, write 1 for Force Value */
			/* XVR polarity == 1, Active Low, write 0 for Force Value */

			/* PHY control register - force enable value - according to polarity */
			status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_FORCE_BEN_IO_VAL, polarity, 0);
			if (status != MV_OK) {
				mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
					   "ERROR: asicOntMiscRegWrite failed for PON phy ctrl force value %d\n\r",
					   trans_value);
				return(MV_ERROR);
				}
		}
	}

	transmit_up = on;
	return(MV_OK);
}

/*******************************************************************************
**
**  onuPonPatternBurstOn
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function start TX transmissions of a selected pattern as periodic or static burst
**
**  PARAMETERS:
**      MV_U32 pattern      0x1     1T
**                          0x2     2T
**                          0x80    PRBS - 9
**                          0x82    PRBS - 15
**                          0x83    PRBS - 23
**     MV_BOOL isPeriodic   TX burst can be either static (in this case the following parameters has no effect)
**                          and this parameter is FALSE OR periodic in this case this parameters is TRUE
**     MV_U32 period        cycle time - the time in micro seconds between the start of on peak and the other
**                          (must be greater then the duration interval)
**     MV_U32 duration      peak time interval in micro seconds  (must be smaller then the duration interval)

**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuPonPatternBurstOn(MV_U32 pattern, MV_BOOL isPeriodic, MV_U32 period, MV_U32 duration)
{
	MV_STATUS  status;

	/*pattern validation*/
	if (!((pattern == ONU_PON_TX_PATTERN_TYPE_T1) ||
	      (pattern == ONU_PON_TX_PATTERN_TYPE_T2) ||
	      (pattern == ONU_PON_TX_PATTERN_TYPE_USER) ||
          (pattern == ONU_PON_TX_PATTERN_TYPE_PRBS_7) ||
	      (pattern == ONU_PON_TX_PATTERN_TYPE_PRBS_9) ||
	      (pattern == ONU_PON_TX_PATTERN_TYPE_PRBS_15) ||
	      (pattern == ONU_PON_TX_PATTERN_TYPE_PRBS_23))) {

		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE, "Wrong pattern type value %d \n", pattern);
		return MV_BAD_VALUE;
	}

	if ((isPeriodic == MV_TRUE) && (duration >= period)) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "Period value %d MUST be greater then duration value %d \n", period, duration);
		return MV_BAD_VALUE;
	}

	/*in case of sequential call to onuPonPatternBurstOn*/
	/*stop timer operation and transmissions*/
	onuPonTimerDisable(&(onuPonResourceTbl_s.onuPonPatternBurstTimerId));
	onuPonPatternBurstTransmit(MV_TRUE);
	onuPonPatternBurstEnable(MV_FALSE);
	time_interval_up     = 0;
	time_interval_down   = 0;

	if (mvCtrlRevGet() == ONU_ASIC_REV_Z2) {

		/* ASIC Rev Z2 */
		/* =========== */
		status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN, ONU_PHY_INPUT, 0);
		if (status != MV_OK)
			return(status);

	} else if ((mvCtrlRevGet() == ONU_ASIC_REV_A0) || (MV_6601_DEV_ID == mvCtrlModelGet())) {

		/* ASIC Rev A0 and MC */
		/* ================== */
		status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN, ONU_PHY_OUTPUT, 0);
		if (status != MV_OK)
			return(status);
	}

	status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_INTERNAL_EN_LOOP_TIMING, 0x0, 0);
	if (status != MV_OK)
		return(status);

	/*Set system in PHY mode, meaning disconnect the PON MAC*/
	status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_INTERNAL_OPEN_TX_DOOR_15, 0x1, 0);
	if (status != MV_OK)
		return(status);

	/*Set sedes power up in phy mode*/
	status = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_POWER_PHY_MODE, 0xfd04,0);
	if (status != MV_OK)
		return(status);

	/*set select on pattern*/
	status  = asicOntMiscRegWrite(mvAsicReg_PT_PATTERN_SELECT, 0x7, 0);
	if (status != MV_OK)
		return(status);

    /*set pattern type*/
    if (pattern != ONU_PON_TX_PATTERN_TYPE_USER)
    {
       status  = asicOntMiscRegWrite(mvAsicReg_PT_PATTERN_DATA, pattern, 0);
       if (status != MV_OK)
         return(status);
    }
    else
    {
       /*
       **  0xF10A2E6C <- PHY Test Data[15:0];  [UserData[8:0], Type = 5}
       **  0xF10A2E68 <- PHY Test Data[31:16]; [UserData[23:9]}
       **  0xF10A2E64 <- PHY Test Data[47:32]; [UserData[39:24]}
       **  0xF10A2E60 <- PHY Test Data[63:48]; [UserData[55:40]}
       **  0xF10A2E5C <- PHY Test Data[79:64]; [UserData[71:56]}
       **  0xF10A2E58 <- PHY Test Data[96:80]; [UserData[79:72]}
       */
       status  = asicOntMiscRegWrite(mvAsicReg_PT_PATTERN_DATA,         (((prbsUserDefinedPattern[0] &  0xFF) << 8)   | (pattern & 0xFF)), 0);
       status |= asicOntMiscRegWrite(mvAsicReg_PT_PATTERN_USER_DATA_01,  ((prbsUserDefinedPattern[0] >> 8)  & 0xFFFF), 0);
       status |= asicOntMiscRegWrite(mvAsicReg_PT_PATTERN_USER_DATA_02, (((prbsUserDefinedPattern[0] >> 24) & 0xFF)   | ((prbsUserDefinedPattern[1] & 0xFF) << 8)), 0);
       status |= asicOntMiscRegWrite(mvAsicReg_PT_PATTERN_USER_DATA_03,  ((prbsUserDefinedPattern[1] >> 8)  & 0xFFFF), 0);
       status |= asicOntMiscRegWrite(mvAsicReg_PT_PATTERN_USER_DATA_04, (((prbsUserDefinedPattern[1] >> 24) & 0xFF)   | ((prbsUserDefinedPattern[2] & 0xFF) << 8)), 0);
       status |= asicOntMiscRegWrite(mvAsicReg_PT_PATTERN_USER_DATA_05,  ((prbsUserDefinedPattern[2] >> 8)  & 0xFF), 0);
       if (status != MV_OK)
         return(status);
    }

	/*turn on selected pattern*/
	status  = asicOntMiscRegWrite(mvAsicReg_PT_PATTERN_ENABLED, 0x1, 0);
	if (status != MV_OK)
		return(status);


	onuPonPatternBurstEnable(MV_TRUE);
	if (isPeriodic == MV_TRUE) {
		/*periosic burst*/
		time_interval_up     = duration;
		time_interval_down   = (period - duration);
		transmit_up          = MV_TRUE;
		onuPonTimerUpdate(&(onuPonResourceTbl_s.onuPonPatternBurstTimerId),0,time_interval_up,1);

	}

	return(MV_OK);
}

/*******************************************************************************
**
**  onuPonPatternBurstOff
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function Turn off  pattern burst and reconnect PON MAC
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuPonPatternBurstOff(void)
{
	MV_STATUS  status;

	onuPonPatternBurstTransmit(MV_TRUE);
	onuPonPatternBurstEnable(MV_FALSE);

	status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_INTERNAL_EN_LOOP_TIMING, 0x1, 0);
	if (status != MV_OK)
		return(status);

	/*Set system in MAC mode, meaning connect the PON MAC*/
	status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_INTERNAL_OPEN_TX_DOOR_15, 0x0, 0);
	if (status != MV_OK)
		return(status);

	/*Set serdes power down in phy mode*/
	status = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_POWER_PHY_MODE, 0x8d00,0);
	if (status != MV_OK)
		return(status);


	/* Set the output enable of the GP_BEN IO to Output */
	status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN, ONU_PHY_OUTPUT, 0);
	if (status != MV_OK)
		return(status);

	/*turn off selected pattern*/
	status  = asicOntMiscRegWrite(mvAsicReg_PT_PATTERN_ENABLED, 0x0, 0);
	if (status != MV_OK)
		return(status);

	/*stop tranmission */
	status = onuPonPatternBurstTransmit(MV_FALSE);
	if (status != MV_OK)
		return(status);
	/*stop timer operation*/
	onuPonTimerDisable(&(onuPonResourceTbl_s.onuPonPatternBurstTimerId));

	time_interval_up     = 0;
	time_interval_down   = 0;

	return(MV_OK);
}

/*******************************************************************************
**
**  onuPonPatternBurstTimerHndl
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function is the timer callback when periodic burst is enabled
**
**  PARAMETERS:  when on  == true  transmission is on
**               when off == false transmission is off
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
void      onuPonPatternBurstTimerHndl(void)
{
	MV_U32  time_interval;
	MV_BOOL is_transmit_up;
	unsigned long flags;
	MV_STATUS status;

	spin_lock_irqsave(&onuPonIrqLock, flags);

	onuPonResourceTbl_s.onuPonPatternBurstTimerId.onuPonTimerActive = ONU_PON_TIMER_NOT_ACTIVE;

	time_interval  = (transmit_up == MV_TRUE ? time_interval_down : time_interval_up);
	is_transmit_up = (transmit_up == MV_TRUE ? MV_FALSE : MV_TRUE);

	if (onuPonResourceTbl_s.onuPonPatternBurstTimerId.onuPonTimerPeriodic == 0)
		onuPonResourceTbl_s.onuPonPatternBurstTimerId.onuPonTimerPeriodic = 1;

	onuPonPatternBurstTransmit(is_transmit_up);

	/*switch on/off selected pattern*/
	status = asicOntMiscRegWrite(mvAsicReg_PT_PATTERN_ENABLED, is_transmit_up, 0);
	if (status != MV_OK) {
		 mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
					"ERROR: asicOntMiscRegWrite failed in PON pattern burst timer handler\n\r");
	 }

	onuPonTimerUpdate(&(onuPonResourceTbl_s.onuPonPatternBurstTimerId),0,time_interval,1);

	spin_unlock_irqrestore(&onuPonIrqLock, flags);
}

/*******************************************************************************
**
**  onuPonTxPowerTimerStateSet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function enable / disable TX power off timer
**
**  PARAMETERS:  MV_BOOL mode - MV_TRUE  - start timer
**                              MV_FALSE - stop timer
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuPonTxPowerTimerStateSet(MV_BOOL mode)
{
  MV_STATUS rcode = MV_OK;

  if (mode == MV_TRUE)
  {
    onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonTxPwrTimerId));
  }
  else
  {
    onuPonTimerDisable(&(onuPonResourceTbl_s.onuPonTxPwrTimerId));
  }

  return (rcode);
}

/*******************************************************************************
**
**  onuPonTxPowerOn
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function turns ON/OFF TX tranciever power using MPP
**
**  PARAMETERS:  MV_BOOL txOn - MV_TRUE - turn TX XVR ON, othervise OFF
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuPonTxPowerOn(MV_BOOL txOn)
{
	MV_U8		gpioPolarity;
	MV_U32		gpioGroup, gpioMask;
	MV_U32		devId = mvCtrlModelGet();
	MV_STATUS	status = MV_OK;

	PON_GPIO_GET(BOARD_GPP_PON_XVR_TX_POWER, gpioGroup, gpioMask);
	if (gpioMask != PON_GPIO_NOT_USED) {
		gpioPolarity = mvBoardGpioPolarityGet(BOARD_GPP_PON_XVR_TX_POWER);
		if ((txOn && gpioPolarity) || (!txOn && !gpioPolarity))
			status = mvGppValueSet(gpioGroup, gpioMask, gpioMask);
		else
			status = mvGppValueSet(gpioGroup, gpioMask, 0);
	} else if (devId == MV_6601_DEV_ID)
		status = MV_ERROR;

	return(status);
}

/*******************************************************************************
**
**  onuPonTxPowerControlInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function initialyzes TX power control pins
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuPonTxPowerControlInit(void)
{
	MV_U32		gpioPinNum, gpioGroup, gpioMask;
	MV_U32		regVal, mppGroup;
	MV_GPP_HAL_DATA	halData;
	MV_U32		devId = mvCtrlModelGet();
	MV_STATUS	status = MV_OK;

	gpioPinNum = mvBoarGpioPinNumGet(BOARD_GPP_PON_XVR_TX_POWER, 0);
	if (gpioPinNum != MV_ERROR) {
		mppGroup = mvCtrlMppRegGet(gpioPinNum / 8);
		/* Set TX power MPP to GPP mode */
		regVal = MV_REG_READ(mppGroup);
		regVal &= ~(0xf << ((gpioPinNum % 8) * 4));
		MV_REG_WRITE(mppGroup, regVal);

		halData.ctrlRev = mvCtrlRevGet();

		status = mvGppInit(&halData);
		if (status == MV_OK) {
			/* Set TX power GPP pin direction to OUT */
			gpioGroup = gpioPinNum / 32;
			gpioMask = 1 << gpioPinNum;
			status = mvGppTypeSet(gpioGroup, gpioMask, (MV_GPP_OUT & gpioMask));
		}

	} else if (devId == MV_6601_DEV_ID)
		status = MV_ERROR;

	return(status);
}

/*******************************************************************************
**
**  onuPonTxLaserOn
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function turns ON/OFF TX laser using MPP
**
**  PARAMETERS:  MV_BOOL on - MV_TRUE - turn TX laser ON, otherwise OFF
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuPonTxLaserOn(MV_BOOL on)
{
	MV_STATUS status;
	MV_U32    gpioGroup, gpioMask;
	MV_U8     gpioPolarity;
	MV_U32    trans_value = 0;
	MV_U32    board_id = mvBoardIdGet();

	if (GFLT200_ID == board_id) {
		PON_GPIO_GET(BOARD_GPP_PON_XVR_TX, gpioGroup, gpioMask);
		if (gpioMask == PON_GPIO_NOT_USED)
			return(MV_ERROR);
		gpioPolarity = mvBoardGpioPolarityGet(BOARD_GPP_PON_XVR_TX);
		if ((on && gpioPolarity) || (!on && !gpioPolarity))
			status = mvGppValueSet(gpioGroup, gpioMask, gpioMask);
		else
			status = mvGppValueSet(gpioGroup, gpioMask, 0);
		transmit_up = on;
	}

	return(MV_OK);
}

/*******************************************************************************
**
**  onuP2PDbXvrBurstEnablePolaritySet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function sets EPON XVR polarity register value in the database
**
**  PARAMETERS:  MV_U32 val
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuP2PDbXvrBurstEnablePolaritySet(MV_U32 val)
{
    onuP2PDb_s.onuP2PGenTbl_s.xvrBurstEnPolarity = val;

    return(MV_OK);
}

/*******************************************************************************
**
**  onuP2PDbXvrBurstEnablePolarityGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function returns EPON XVR polarity register value
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_U32 mode
**
*******************************************************************************/
MV_U32 onuP2PDbXvrBurstEnablePolarityGet(void)
{
    return onuP2PDb_s.onuP2PGenTbl_s.xvrBurstEnPolarity;
}

#endif /* PON_FPGA */
