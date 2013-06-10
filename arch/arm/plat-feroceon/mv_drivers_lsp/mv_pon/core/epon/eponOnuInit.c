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
/**  FILE        : eponOnuInit.c                                             **/
/**                                                                          **/
/**  DESCRIPTION : This file contains ONU EPON init sequence definitions     **/
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
#include "ponOnuHeader.h"
#include "eponOnuHeader.h"

/* Local Constant
------------------------------------------------------------------------------*/
#define __FILE_DESC__ "mv_pon/core/epon/eponOnuInit.c"

/* Global Variables
------------------------------------------------------------------------------*/
extern MV_U32 dbaRprtT0StateVal;
extern MV_U32 dbaRprtT0StateInterval;
extern MV_U32 dbaRprtT1StateVal;
extern MV_U32 dbaRprtT1StateInterval;

/* Local Variables
------------------------------------------------------------------------------*/

/* Export Functions
------------------------------------------------------------------------------*/

/* Local Functions
------------------------------------------------------------------------------*/
MV_STATUS onuEponAsicInit(void);
#if 0 /* NOT USED */
MV_STATUS onuEponAsicFiFoReInit(void);
#endif /* NOT USED */
MV_STATUS onuGponAppInit(void);
void      onuGponStateAndEventTblInit(void);
void      onuGponInPmInit(void);

/*******************************************************************************
**
**  onuEponSetup
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function execute onu setup init sequence
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**               MV_ERROR
**
*******************************************************************************/
MV_STATUS onuEponSetup(void)
{
  MV_STATUS rcode;

  /* init onu base address */
  rcode = onuEponGlbAddrInit();
  if (rcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) Failed to init onu base address\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  if (GFLT200_ID == mvBoardIdGet())
    ponXvrFunc = EponXvrSDPolarityLowStatus;
  else
    ponXvrFunc = EponXvrSDPolarityHighStatus;
  /* init onu database */
  rcode = onuEponDbInit();
  if (rcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) Failed to init onu database\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* init onu Asic */
  rcode = onuEponAsicInit();
  if (rcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) Failed to init asic\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  return(MV_OK);
}

#ifndef PON_FPGA
/*******************************************************************************
**
**  onuEponAsicLedsInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function init onu leds
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuEponAsicLedsInit(void)
{
  onuPonLedInit();

  onuPonLedHandler(ONU_PON_SYNC_LED, ACTIVE_LED_OFF);
  onuPonLedHandler(ONU_PON_SYS_LED, ACTIVE_LED_ON);

  return(MV_OK);
}
#endif /* PON_FPGA */

/*******************************************************************************
**
**  onuEponAsicInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function init default onu EPON MAC configuration
**
**  PARAMETERS:  MV_BOOL initTime - init indication flag, true = init sequence
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuEponAsicInit(void)
{
	MV_STATUS status;
	MV_U32    macAddrHigh;
	MV_U32    macAddrLow;
#ifdef PON_Z2
	MV_U32    pcsConfig;
#endif /* PON_Z2 */
	MV_U32    macId;
	MV_U32    devId = mvCtrlModelGet();

#ifndef PON_FPGA
	status = onuEponSerdesInit();
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) onuEponSerdesInit\n", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

#endif /* PON_FPGA */

#ifndef PON_FPGA
	onuEponAsicLedsInit();
#endif /*PON_FPGA */

	/* Setup TX power ON/OFF GPP and turn TX power OFF */
	status = onuPonTxPowerControlInit();
	if (status == MV_OK)
		status = onuPonTxPowerOn(MV_FALSE);

	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) Unable to configure TX POWER GPP\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	/* Disable MAC */
	status = mvOnuEponMacOnuEnableSet(ONU_RX_DIS, ONU_TX_DIS);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvOnuEponMacOnuStateSet\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	/* MAC State */
	for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++) {
		status = mvOnuEponMacOnuStateSet(ONU_EPON_NOT_REGISTERD, macId);
		if (status != MV_OK) {
			mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
				   "ERROR: (%s:%d) mvOnuEponMacOnuStateSet\n\r", __FILE_DESC__, __LINE__);
			return(MV_ERROR);
		}
	}

	/* Broadcast Address */
	status = mvOnuEponMacGenBcastAddrSet(ONU_BROADCAST_ADDR_HIGH, ONU_BROADCAST_ADDR_LOW);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvOnuEponMacGenBcastAddrSet\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	/* Unicast Address */
	for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++) {
		onuEponDbOnuMacAddrGet(&macAddrLow, &macAddrHigh, macId);

		status = mvOnuEponMacGenUcastAddrSet(macAddrHigh, macAddrLow, macId);
		if (status != MV_OK) {
			mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
				   "ERROR: (%s:%d) mvOnuEponMacGenUcastAddrSet\n\r", __FILE_DESC__, __LINE__);
			return(MV_ERROR);
		}
	}

	status = mvOnuEponMacGenSyncTimeSet(ONU_DEF_SYNC_TIME,
					    ONU_DEF_SYNC_TIME_ADD,
					    ONU_DEF_SYNC_TIME_FORCE_SW,
					    ONU_DEF_SYNC_TIME_DIS_GATE_AUTO,
					    ONU_DEF_SYNC_TIME_DIS_DISCOVER_AUTO);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvOnuEponMacGenSyncTimeSet\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	status = mvOnuEponMacDdmDelaySet(ONU_DEF_DDM_DELAY);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvAsicReg_EPON_DDM_1814_CONFIG\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	status = mvOnuEponMacRxpPacketForwardSet(ONU_CTRL_FRAME_TO_DATA_QUEUE,
						 ONU_CTRL_FRAME_TO_CTRL_QUEUE,
						 ONU_RPRT_FRAME_TO_DATA_QUEUE,
						 ONU_RPRT_FRAME_TO_RPRT_QUEUE,
						 ONU_SLOW_FRAME_TO_RPRT_QUEUE,
						 ONU_SLOW_FRAME_TO_CTRL_QUEUE);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvOnuEponMacRxpPacketForwardSet\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	status = mvOnuEponMacOnuRegAutoUpdateStateSet(1);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvOnuEponMacOnuAutoUpdateStateSet\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	/* ONU_REPORT_AUTO_RES is always ON */
	/* In case of SW Report via Control FIFO, the HW will send empty report - keep alive */
	status = mvOnuEponMacGenOnuConfigSet(ONU_RX_PCS_FEC_DIS,
					     ONU_TX_PCS_FEC_DIS,
					     ONU_REPORT_AUTO_RES,
					     ONU_REG_ACK_AUTO_RES,
					     ONU_REG_REQ_AUTO_RES,
					     ONU_TX_FEC_DIS);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvOnuEponMacGenOnuConfigSet\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	status = mvOnuEponMacRxpPacketFilterSet(ONU_FORWARD_LLID_CRC_ERR_PKT,
						ONU_FORWARD_LLID_FCS_ERR_PKT,
						ONU_FORWARD_LLID_GMII_ERR_PKT,
						ONU_FORWARD_LLID_LEN_ERR_PKT,
						ONU_FORWARD_LLID_ALL_PKT,
						ONU_FORWARD_LLID_7FFF_MODE_0_PKT,
						ONU_FORWARD_LLID_7FFF_MODE_1_PKT,
						ONU_FORWARD_LLID_XXXX_MODE_1_PKT,
						ONU_DROP_LLID_NNNN_MODE_1_PKT);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvOnuEponMacRxpPacketFilterSet\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	status = mvOnuEponMacGenLaserParamSet(ONU_DEF_LASER_ON_OFF_TIME,
					      ONU_DEF_LASER_ON_TIME,
					      ONU_DEF_LASER_OFF_TIME);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvOnuEponMacGenLaserParamSet\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++) {
		status = mvOnuEponMacTxmLlidSet(ONU_DEF_TX_LLID, macId);
		if (status != MV_OK) {
			mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
				   "ERROR: (%s:%d) mvOnuEponMacTxmLlidSet\n\r", __FILE_DESC__, __LINE__);
			return(MV_ERROR);
		}

		status = mvOnuEponMacRxpLlidDataSet(ONU_DEF_RX_LLID, macId);
		if (status != MV_OK) {
			mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
				   "ERROR: (%s:%d) mvOnuEponMacRxpLlidDataSet\n\r", __FILE_DESC__, __LINE__);
			return(MV_ERROR);
		}
	}

#ifdef PON_Z1

	status = mvOnuEponMacTxmConfigSet(ONU_DEF_TXM_CFG_MODE,
					  ONU_DEF_TXM_CFG_ALIGNMENT,
					  ONU_DEF_TXM_CFG_PRIORITY);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvOnuEponMacTxmConfigSet\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

#else /* PON_Z2 and later */

	status = mvOnuEponMacTxmOverheadSet(0x0E01);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvOnuEponMacTxmOverheadSet\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	status = mvOnuEponMacTxmConfigSet(ONU_DEF_TXM_CFG_TFEC_1,
					  ONU_DEF_TXM_CFG_TFEC_2,
					  ONU_DEF_TXM_CFG_CTRL_FIFO_DIS_FCS_OFF,
					  ONU_DEF_TXM_CFG_MASK_GMII_OFF,
					  ONU_DEF_TXM_CFG_BLOCK_DATA_OFF,
					  ONU_DEF_TXM_CFG_IPG_ADD,
					  ONU_DEF_TXM_CFG_MODE_EPON,
					  ONU_DEF_TXM_CFG_ALIGNMENT_EVEN,
					  ONU_DEF_TXM_CFG_PRIORITY_CTRL_FIRST);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvOnuEponMacTxmConfigSet\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	pcsConfig = (ONU_DEF_PCS_CFG_RX_EN | ONU_DEF_PCS_CFG_TX_EN);
	status = mvOnuEponMacPcsConfigSet(pcsConfig);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvOnuEponMacPcsConfigSet\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	status = mvOnuEponMacDdmTxPolaritySet(ONU_DEF_DDM_CFG_TX_EN_OR,
					      ONU_DEF_DDM_CFG_POLARITY_HIGH);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvOnuEponMacDdmTxPolaritySet\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

    onuEponDbBurstEnablePolaritySet(ONU_DEF_DDM_CFG_POLARITY_HIGH);

	status = mvOnuEponMacGpmDiscoveryGrantLengthSet(0x26,  /* grantLength */
							0x06,  /* addOffsetForCalc */
							0x32); /* grantLengthMultiTq */
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvOnuEponMacGpmDiscoveryGrantLengthSet\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	status = mvOnuEponMacGenTimestampConfig(0x0,   /* gpmAddSyncTimeToTimestamp */
						0x760, /* gpmAimestampOffset */
						0x1,   /* txmAddSyncTimeToTimestamp */
						0x0,   /* txmUseTimestampImage */
						0x77B);/* txmTimestampOffset) */
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvOnuEponMacGenTimestampConfig\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	/* ==================================== */
	/* ==         SW REPORT MODE         == */
	/* ==================================== */
	for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++) {
		status  = mvOnuEponMacTxmCppReportConfig(0xFF,	/* Queueset2 not supported -
								 * set to 0xFF (queueReportQ2) */
							 0x1,	/* Queueset2 not supported - set to 0x1,
								 * Software (reportBitmapQ2) */
							 0xFF,	/* Queueset1 supported - set to 0, means all
								 * 8 queues are active (queueReport) */
							 1,	/* Number of queueSet -
								 * support for 1 queueset */
							 0x1,	/* Queueset1 supported - set to 0x1,
								 * Software (reportBitmap) */
							 macId);
		if (status != MV_OK) {
			mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
				   "ERROR: (%s:%d) mvOnuEponMacTxmCppReportConfig, macId=%d\n\r",
				   __FILE_DESC__, __LINE__, macId);
			return(MV_ERROR);
		}
	}

	if (devId == MV_6601_DEV_ID) {

#if (!defined PON_Z1) && (!defined PON_Z2)
		/* DDM TX Polarity */
		status = mvOnuEponMacDdmTxPolaritySet(EPON_DDM_TX_EN_MASK_DEFAULT,
						      EPON_DDM_TX_TX_POL_DEFAULT,
						      EPON_DDM_TX_SERDES_POL_DEFAULT,
						      EPON_DDM_TX_XVR_POL_DEFAULT,
						      EPON_DDM_TX_BURST_ENA_DEFAULT);
		if (status != MV_OK) {
			mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
				   "ERROR: (%s:%d) mvOnuEponMacDdmTxPolaritySet\n\r", __FILE_DESC__, __LINE__);
			return(MV_ERROR);
		}
#endif
	} /* devId == MV_6601_DEV_ID*/

	status = mvOnuEponMacGenUtmTcPeriodSet(0x100);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvOnuEponMacGenUtmTcPeriodSet\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	status = mvOnuEponMacGenUtmTcPeriodEnSet(0x1);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvOnuEponMacGenUtmTcPeriodEnSet\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	status = mvOnuPonMacBurstEnableInit();
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) mvOnuPonMacBurstEnableInit\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

#endif /* !defined PON_Z1 */

	return(MV_OK);
}

/*******************************************************************************
**
**  onuEponAsicReInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function re-init default onu EPON MAC configuration
**
**  PARAMETERS:  MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuEponAsicReInit(MV_U32 macId, MV_U32 silenceMode)
{
  MV_STATUS status;
  MV_U32    rxGenFecEn = 0;
  MV_U32    txGenFecEn = 0;
  MV_U32    txMacFecEn[8] = {0,0,0,0,0,0,0,0};

  status = mvOnuEponMacOnuStateSet(ONU_EPON_NOT_REGISTERD, macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) mvOnuEponMacOnuStateSet\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  status = mvOnuEponMacTxmLlidSet(ONU_DEF_TX_LLID, macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) mvOnuEponMacTxmLlidSet\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  status = mvOnuEponMacRxpLlidDataSet(ONU_DEF_RX_LLID, macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) mvOnuEponMacRxpLlidDataSet\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* handle silence mode */
  if ((onuEponDbOnuSilenceStateGet() != ONU_SILENCE_NOT_ACTIVE) &&
	  (silenceMode == ONU_SILENCE_ENABLED))
  {
	/* Clear onu Silence Timers */
	onuPonTimerDisable(&(onuPonResourceTbl_s.onuPonSilenceTimerId[macId]));

	status  = mvOnuEponMacGenOnuConfigAutoAckSet(ONU_REG_ACK_AUTO_RES);
	status |= mvOnuEponMacGenOnuConfigAutoRequestSet(ONU_REG_REQ_AUTO_RES);
	if (status != MV_OK)
	{
	  mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
				 "ERROR: (%s:%d) Failed to enable auto request / ack\n\r", __FILE_DESC__, __LINE__);
	  return(status);
	}

	onuEponDbOnuCfgAutoSet(ONU_REG_REQ_AUTO_RES, ONU_REG_ACK_AUTO_RES, ONU_REPORT_AUTO_RES);
  }

  mvPonPrint(PON_PRINT_DEBUG, PON_MNG_MODULE,
			 "DEBUG: (%s:%d) onuEponAsicReInit, rxGenFecEn(%d), txGenFecEn(%d), txMacFecEn(%d,%d,%d,%d,%d,%d,%d,%d)\n",
			 __FILE_DESC__, __LINE__, rxGenFecEn, txGenFecEn, txMacFecEn[0], txMacFecEn[1], txMacFecEn[2], txMacFecEn[3],
             txMacFecEn[4], txMacFecEn[5], txMacFecEn[6], txMacFecEn[7]);

  status = onuEponApiFecConfig(rxGenFecEn, txGenFecEn, txMacFecEn, ONU_FEC_CFG_RE_INIT);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
    		   "ERROR: (%s:%d) onuEponApiFecConfig\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* clear OAM SW FIFO - Tx */
  onuEponOamFlushAllMsg();

  mvPonPrint(PON_PRINT_DEBUG, PON_MNG_MODULE,
			 "DEBUG: (%s:%d) onuEponAsicReInit, onuEponOamFlushAllMsg\n\r", __FILE_DESC__, __LINE__);

#if 0 /* NOT USED */
  status = onuEponAsicQueueDisable(macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) onuEponAsicQueueDisable\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  if (macId == 0)
  {
    status = onuEponAsicFiFoReInit();
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
                 "ERROR: (%s:%d) onuEponMacFiFoReInit\n\r", __FILE_DESC__, __LINE__);
      return(MV_ERROR);
    }
  }
#endif  /* NOT USED */

  return(MV_OK);
}

#if 0 /* NOT USED */
/*******************************************************************************
**
**  onuEponAsicFiFoReInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function re-init onu EPON MAC Fifo
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuEponAsicFiFoReInit(void)
{
  MV_STATUS status;
  MV_U32    rxEnable;
  MV_U32    txEnable;

  /* Disable all TX  */
  rxEnable = 1;
  txEnable = 0;
  status = mvOnuEponMacOnuEnableSet(rxEnable, txEnable);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) mvOnuEponMacOnuEnableSet\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* disable onu pon interrupt mask */
  mvOnuEponMacPonInterruptMaskSet(0);

  /* Turn on GMII MASK */
  /* Switch TX port to Standard GE mode */
  status = mvOnuEponMacTxmConfigSet(ONU_DEF_TXM_CFG_TFEC_1,
                                    ONU_DEF_TXM_CFG_TFEC_2,
                                    ONU_DEF_TXM_CFG_CTRL_FIFO_DIS_FCS_OFF,
                                    ONU_DEF_TXM_CFG_MASK_GMII_ON,
                                    ONU_DEF_TXM_CFG_BLOCK_DATA_OFF,
                                    ONU_DEF_TXM_CFG_IPG_DONT_ADD,
                                    ONU_DEF_TXM_CFG_MODE_GE,
                                    ONU_DEF_TXM_CFG_ALIGNMENT_EVEN,
                                    ONU_DEF_TXM_CFG_PRIORITY_RPRT_FIRST);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) mvOnuEponMacTxmConfigSet\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* Enable all TX  */
  rxEnable = 1;
  txEnable = 0xFF;
  status = mvOnuEponMacOnuEnableSet(rxEnable, txEnable);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) mvOnuEponMacOnuEnableSet\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* Wait 10ms */
  mvOsDelay(10);

  /* Disable all TX  */
  rxEnable = 1;
  txEnable = 0;
  status = mvOnuEponMacOnuEnableSet(rxEnable, txEnable);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) mvOnuEponMacOnuEnableSet\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* Switch TX port to Standard EPON mode */
  /* Turn off GMII MASK */
  status = mvOnuEponMacTxmConfigSet(ONU_DEF_TXM_CFG_TFEC_1,
                                    ONU_DEF_TXM_CFG_TFEC_2,
                                    ONU_DEF_TXM_CFG_CTRL_FIFO_DIS_FCS_OFF,
                                    ONU_DEF_TXM_CFG_MASK_GMII_OFF,
                                    ONU_DEF_TXM_CFG_BLOCK_DATA_OFF,
                                    ONU_DEF_TXM_CFG_IPG_DONT_ADD,
                                    ONU_DEF_TXM_CFG_MODE_EPON,
                                    ONU_DEF_TXM_CFG_ALIGNMENT_EVEN,
                                    ONU_DEF_TXM_CFG_PRIORITY_RPRT_FIRST);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) mvOnuEponMacTxmConfigSet\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* enable onu pon interrupt mask */
  mvOnuEponMacPonInterruptMaskSet(0x120);

  /* Enable all TX  */
 rxEnable = 1;
 txEnable = 0xFF;
 status = mvOnuEponMacOnuEnableSet(rxEnable, txEnable);
 if (status != MV_OK)
 {
   mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
              "ERROR: (%s:%d) mvOnuEponMacOnuEnableSet\n\r", __FILE_DESC__, __LINE__);
   return(MV_ERROR);
 }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponAsicQueueEnable
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function enables all queues for the specific mac Id
**
**  PARAMETERS:  MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuEponAsicQueueEnable(MV_U32 macId)
{
  switch (macId)
  {
    case 0: asicOntMiscRegWrite(mvAsicReg_GUNIT_LLID0_QUEUES, 0x00FF, 0); break;
    case 1: asicOntMiscRegWrite(mvAsicReg_GUNIT_LLID1_QUEUES, 0x00FF, 0); break;
    case 2: asicOntMiscRegWrite(mvAsicReg_GUNIT_LLID2_QUEUES, 0x00FF, 0); break;
    case 3: asicOntMiscRegWrite(mvAsicReg_GUNIT_LLID3_QUEUES, 0x00FF, 0); break;
    case 4: asicOntMiscRegWrite(mvAsicReg_GUNIT_LLID4_QUEUES, 0x00FF, 0); break;
    case 5: asicOntMiscRegWrite(mvAsicReg_GUNIT_LLID5_QUEUES, 0x00FF, 0); break;
    case 6: asicOntMiscRegWrite(mvAsicReg_GUNIT_LLID6_QUEUES, 0x00FF, 0); break;
    case 7: asicOntMiscRegWrite(mvAsicReg_GUNIT_LLID7_QUEUES, 0x00FF, 0); break;
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponAsicQueueDisable
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function disables all queues for the specific mac Id
**               except queue 0
**
**  PARAMETERS:  MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuEponAsicQueueDisable(MV_U32 macId)
{
  switch (macId)
  {
    case 0: asicOntMiscRegWrite(mvAsicReg_GUNIT_LLID0_QUEUES, 0xFE00, 0); break;
    case 1: asicOntMiscRegWrite(mvAsicReg_GUNIT_LLID1_QUEUES, 0xFE00, 0); break;
    case 2: asicOntMiscRegWrite(mvAsicReg_GUNIT_LLID2_QUEUES, 0xFE00, 0); break;
    case 3: asicOntMiscRegWrite(mvAsicReg_GUNIT_LLID3_QUEUES, 0xFE00, 0); break;
    case 4: asicOntMiscRegWrite(mvAsicReg_GUNIT_LLID4_QUEUES, 0xFE00, 0); break;
    case 5: asicOntMiscRegWrite(mvAsicReg_GUNIT_LLID5_QUEUES, 0xFE00, 0); break;
    case 6: asicOntMiscRegWrite(mvAsicReg_GUNIT_LLID6_QUEUES, 0xFE00, 0); break;
    case 7: asicOntMiscRegWrite(mvAsicReg_GUNIT_LLID7_QUEUES, 0xFE00, 0); break;
  }

  return(MV_OK);
}
#endif  /* NOT USED */

/*******************************************************************************
**
**  onuEponSwitchOn
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function execute onu switchOn init sequence
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR
**
*******************************************************************************/
MV_STATUS onuEponSwitchOn(void)
{
  MV_STATUS rcode;

  /* init onu RTOS resources */
  rcode = onuEponRtosResourceInit();
  if (rcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) onuPonRtosResourceInit\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  onuEponAppInit();

  /* onu epon counters table */
  onuEponPmInit();

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponOperate
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function execute onu operate init sequence
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuEponOperate(void)
{
  MV_STATUS status;
  MV_U32    interruptMask;

  /* enable onu pon interrupt mask */
  interruptMask = (ONU_EPON_INTERRUPTS);
  interruptMask &= ~(ONU_EPON_TIMESTAMP_VALUE_MATCH_MASK);
  status = mvOnuEponMacPonInterruptMaskSet(interruptMask);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) Enable PON interrupt mask\n\r", __FILE_DESC__, __LINE__);
    return(status);
  }

  /* Enable MAC */
  status = mvOnuEponMacOnuRxEnableSet(ONU_RX_EN);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) mvOnuEponMacOnuRxEnableSet\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* Turn ON TX Power */
  status = onuPonTxPowerOn(MV_TRUE);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) onuPonTxPowerOn\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  status = mvOnuEponMacOnuTxEnableSet(ONU_TX_EN, 0);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) mvOnuEponMacOnuTxEnableSet\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN, ONU_PHY_OUTPUT, 0);
  if (status != MV_OK)
  {
	mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
	  		 "ERROR: (%s:%d) mvOnuEponMacOnuIOEnable\n\r", __FILE_DESC__, __LINE__);
	return(MV_ERROR);
  }

  /* start onu pon pon pm timer */
  onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonPmTimerId));

  /* start onu pon pon mpc timer */
  onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonMpcpTimerId));

  if (onuEponDbOnuSwRprtTimerTypeGet() == ONU_EPON_HW_DBA_RPRT_TIMER)
  {
    /* start onu pon pon tx module timer */
    onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonHwRprtTxModTimerId));

    /* Enable DBA Report message handling */
	onuEponIsrTimerHwReportStateSet(MV_TRUE, /* Enable */
									dbaRprtT0StateVal,
									dbaRprtT0StateInterval,
									dbaRprtT1StateVal,
									dbaRprtT1StateInterval);

  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function init ONU
**
**  PARAMETERS:  IOCTL Init data
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuEponInit(S_EponIoctlInit *ioctlInit)
{
  MV_STATUS status;

  /*re-set DBA mode */
  status = onuEponDbaModeInit(ONU_DBA_SW_RPRT_MODE);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) onuEponDbaModeInit\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* re-set DDM polarity */
  status = mvOnuEponMacDdmTxPolaritySet(ONU_DEF_DDM_CFG_TX_EN_OR, ioctlInit->ponXvrBurstEnPolarity);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) mvOnuEponMacDdmTxPolaritySet\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

    ponXvrFunc = funcEponXvrSDStatus(ioctlInit->ponXvrPolarity);

    onuP2PDbXvrBurstEnablePolaritySet(ioctlInit->p2pXvrBurstEnPolarity);

#ifndef PON_FPGA
  /* enable onu dying gasp interrupt mask */
  status = mvOnuPonMacDgInterruptEn(ioctlInit->dgPolarity);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) mvOnuPonMacDgInterruptEn\n", __FILE_DESC__, __LINE__);
    return(status);
  }
#endif /* PON_FPGA */

    status = mvEponApi2kSupportedSet(ioctlInit->pkt2kSupported);
    if (status != MV_OK)
    {
        mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
                   "ERROR: (%s:%d) mvEponApi2kSupportedSet\r\n", __FILE_DESC__, __LINE__);
        return(status);
    }

  return(MV_OK);
}

