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
**  FILE        : eponOnuLnxKsUI.c                                           **
**                                                                           **
**  DESCRIPTION : This file implements ONU GPON CLI functionality            **
*******************************************************************************
*                                                                             *
*  MODIFICATION HISTORY:                                                      *
*                                                                             *
*   29Oct06  Oren Ben Hayun   created                                         *
* =========================================================================== *
******************************************************************************/

/* Include Files
------------------------------------------------------------------------------*/
#include "eponOnuHeader.h"
#ifndef PON_FPGA
#include "mvSysPonConfig.h"
#endif /* PON_FPGA */

/* Local Constant
------------------------------------------------------------------------------*/
#define __FILE_DESC__ "mv_pon/perf/epon/eponOnuLnxKsUI.c"

/* Global Variables
------------------------------------------------------------------------------*/
extern MV_U32 llidQueueState[8][4][8];
extern MV_U32 llidQueueThreshold[8][4][8];
extern MV_U32 llidNumOfQueuesets[8];
extern MV_U32 llidNumOfQueues[8];
extern MV_U32 llidNumOfReportBytes[8];

extern MV_U32 g_OnuEponRandomCount;
extern MV_U32 g_OnuEponRandomValues[];

extern MV_U32 regRandomMaskThreshold;

/* Local Variables
------------------------------------------------------------------------------*/
MV_BOOL infoEponCntReadClearInd = MV_TRUE;

/* Export Functions
------------------------------------------------------------------------------*/
extern MV_STATUS onuEponMiDbaConfig(S_EponIoctlDba *ioctlDba);
extern MV_STATUS onuEponMiHoldoverConfig(S_EponIoctlHoldOver *ioctlHoldover);
extern MV_STATUS onuEponMiSilenceConfig(S_EponIoctlSilence *ioctlSilence);
extern MV_STATUS onuEponMiAlarmGet(MV_U32 *alarm);
extern void      onuEponMiCfgMacTxEn(MV_U32 enable, MV_U32 macId);

extern void rprtThresholdSet(MV_U32 lowThCnt, MV_U32 lowThVal, MV_U32 hiThCnt, MV_U32 hiThVal);

/* Local Functions
------------------------------------------------------------------------------*/
/* ========================================================================== */
/*                            Info Commands                                   */
/* ========================================================================== */
/******************************************************************************/

/*******************************************************************************
**
**  onuEponUiInfoShow
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print onu information
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     message length
**
*******************************************************************************/
int onuEponUiInfoShow(char* buf)
{
  MV_STATUS status;
  S_EponIoctlInfo info[EPON_MAX_MAC_NUM];
  MV_U8           *stateDesc[] = {"", "Pending", "Active"};
  int             off = 0;
  int             macId;

  for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++)
  {
    status = onuEponApiInformationGet(&info[macId], macId);
    if (status != MV_OK)
    {
      off += mvOsSPrintf(buf+off, "Failed read mac information!!!\n");
      return(off);
    }
  }

  off += mvOsSPrintf(buf+off, "\n");
  off += mvOsSPrintf(buf+off, "ONT Information:\n");
  off += mvOsSPrintf(buf+off, "---------------------------------------------------------------------------------\n");
  off += mvOsSPrintf(buf+off, "     State    Ether    Unicast             Bcast               Tx LLId  Tx LLId  \n");
  off += mvOsSPrintf(buf+off, "              Type     Mac Addr            Mac Addr                              \n");
  off += mvOsSPrintf(buf+off, " MAC                                                                             \n");
  off += mvOsSPrintf(buf+off, "---------------------------------------------------------------------------------\n");

  for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++)
  {
      off += mvOsSPrintf(buf+off, "[%02d] %-8s %8x %02x:%02x:%02x:%02x:%02x:%02x %02x:%02x:%02x:%02x:%02x:%02x %8u %8u\n",
                        macId,
                        stateDesc[info[macId].onuEponState],
                        info[macId].onuEponCtrlType,
                        info[macId].onuEponMacAddr[0], info[macId].onuEponMacAddr[1], info[macId].onuEponMacAddr[2],
                        info[macId].onuEponMacAddr[3], info[macId].onuEponMacAddr[4], info[macId].onuEponMacAddr[5],
                        info[macId].onuEponBcastAddr[0], info[macId].onuEponBcastAddr[1], info[macId].onuEponBcastAddr[2],
                        info[macId].onuEponBcastAddr[3], info[macId].onuEponBcastAddr[4], info[macId].onuEponBcastAddr[5],
                        info[macId].onuEponTxLLID,
                        info[macId].onuEponRxLLID);
  }
  off += mvOsSPrintf(buf+off, "--------------------------------------------------\n\r");

  return(off);
}

/*******************************************************************************
**
**  onuEponUiAlarmShow
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print onu alarms
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     message length
**
*******************************************************************************/
int onuEponUiAlarmShow(char* buf)
{
  MV_U32 alarm[2];
  MV_U8  *statusText[] = {"OFF", "ON "};
  int    off = 0;

  onuEponAlarmGet(alarm);

  off += mvOsSPrintf(buf+off, "\n");
  off += mvOsSPrintf(buf+off, "-------------\n");
  off += mvOsSPrintf(buf+off, "|ONU ALARMS |\n");
  off += mvOsSPrintf(buf+off, "-------------------------\n");
  off += mvOsSPrintf(buf+off, "| XVR SD    | SERDES SD |\n");
  off += mvOsSPrintf(buf+off, "-------------------------\n");
  off += mvOsSPrintf(buf+off, "| %9s | %9s |\n", statusText[alarm[0]], statusText[alarm[1]]);
  off += mvOsSPrintf(buf+off, "-------------------------\n");

  return(off);
}

/*******************************************************************************
**
**  onuEponUiDebugReadReg
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function test requested address space
**
**  PARAMETERS:  MV_U32 readOffset
**               MV_U32 readEntry
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponUiDebugReadReg(MV_U32 readOffset, MV_U32 readEntry)
{
  MV_U32 regValue;

#ifndef PON_FPGA
  regValue = MV_REG_READ((MV_EPON_MAC_REGS_BASE) + readOffset + (readEntry * sizeof (MV_U32)));
#else /* PON_FPGA */
  regValue = MV_REG_READ((MV_GPON_REG_BASE) + readOffset + (readEntry * sizeof (MV_U32)));
#endif /* PON_FPGA */

  mvPonPrint(PON_PRINT_INFO, PON_CLI_MODULE,
             "Read register offset 0x%x, entry %d = [0x%08X]\n\r", readOffset, readEntry, regValue);
}

/*******************************************************************************
**
**  onuEponUiDebugWriteReg
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function test requested address space
**
**  PARAMETERS:  MV_U32 writeOffset
**               MV_U32 writeEntry
**               MV_U32 regValue
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponUiDebugWriteReg(MV_U32 writeOffset, MV_U32 writeEntry, MV_U32 regValue)
{
#ifndef PON_FPGA
  MV_REG_WRITE((MV_EPON_MAC_REGS_BASE) + writeOffset + (writeEntry * sizeof (MV_U32)), regValue);
#else /* PON_FPGA */
  MV_REG_WRITE((MV_GPON_REG_BASE) + writeOffset + (writeEntry * sizeof (MV_U32)), regValue);
#endif /* PON_FPGA */

  mvPonPrint(PON_PRINT_INFO, PON_CLI_MODULE,
             "Write  register offset 0x%x, entry %d = [0x%08X]\n\r", writeOffset, writeEntry, regValue);
}

/*******************************************************************************
**
**  onuEponUiInfoHelpShow
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print info help
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     message length
**
*******************************************************************************/
int onuEponUiInfoHelpShow(char* buf)
{
  int off = 0;

  off += mvOsSPrintf(buf+off, "============================================================================\n");
  off += mvOsSPrintf(buf+off, "Configuration Commands[HEX]\n");
  off += mvOsSPrintf(buf+off, "============================================================================\n");
  off += mvOsSPrintf(buf+off, " echo [Offset] [Entry]           > readReg   - read onu reg\n");
  off += mvOsSPrintf(buf+off, " echo [Offset] [Entry] [Value]   > writeReg  - write onu reg\n");
  off += mvOsSPrintf(buf+off, "============================================================================\n");
  off += mvOsSPrintf(buf+off, "Display Commands: cat <file>\n");
  off += mvOsSPrintf(buf+off, "============================================================================\n");
  off += mvOsSPrintf(buf+off, " cat info  - dump onu information\n");
  off += mvOsSPrintf(buf+off, " cat alarm - dump onu alarms\n");
  off += mvOsSPrintf(buf+off, "============================================================================\n");

  return(off);
}

/******************************************************************************/
/* ========================================================================== */
/*                            Pm Commands                                     */
/* ========================================================================== */
/******************************************************************************/

/*******************************************************************************
**
**  onuEponUiRxCountersShow
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print onu rx counters
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     message length
**
*******************************************************************************/
int onuEponUiRxCountersShow(char* buf)
{
  MV_STATUS       status;
  S_EponIoctlRxPm rxCounters;
  int             off = 0;

  status = onuEponApiRxPmGet(&rxCounters, infoEponCntReadClearInd, 0);
  if (status != MV_OK)
  {
    off += mvOsSPrintf(buf+off, "Failed read Rx pm information!!!\n");
    return(off);
  }

  off += mvOsSPrintf(buf+off, "\n");
  off += mvOsSPrintf(buf+off, "Rx Counters\n");
  off += mvOsSPrintf(buf+off, "-------------------------------------------------------------------\n");
  off += mvOsSPrintf(buf+off, "     FCS      Short    Long     Data     Ctrl     Rprt     Gate    \n");
  off += mvOsSPrintf(buf+off, "     Frames   Frames   Frames   Frames   Frames   Frames   Frames  \n");
  off += mvOsSPrintf(buf+off, " MAC Rece     Rece     Rece     Rece     Rece     Rece     Rece    \n");
  off += mvOsSPrintf(buf+off, "-------------------------------------------------------------------\n");
  off += mvOsSPrintf(buf+off, "[00] %8u %8u %8u %8u %8u %8u %8u\n",
                     rxCounters.fcsErrorFramesCnt,
                     rxCounters.shortFramesCnt,
                     rxCounters.longFramesCnt,
                     rxCounters.dataFramesCnt,
                     rxCounters.ctrlFramesCnt,
                     rxCounters.reportFramesCnt,
                     rxCounters.gateFramesCnt);
  off += mvOsSPrintf(buf+off, "-------------------------------------------------------------------\n\r");

  return(off);
}

/*******************************************************************************
**
**  onuEponUiTxCountersShow
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print onu tx counters
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     message length
**
*******************************************************************************/
int onuEponUiTxCountersShow(char* buf)
{
  MV_STATUS       status;
  S_EponIoctlTxPm txCounters[EPON_MAX_MAC_NUM];
  int             macId;
  int             off = 0;

  for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++)
  {
    status = onuEponApiTxPmGet(&txCounters[macId], infoEponCntReadClearInd, macId);
    if (status != MV_OK)
    {
      off += mvOsSPrintf(buf+off, "Failed read Tx pm information!!!\n");
      return(off);
    }
  }

  off += mvOsSPrintf(buf+off, "\n");
  off += mvOsSPrintf(buf+off, "Tx Counters\n");
  off += mvOsSPrintf(buf+off, "--------------------------------------------------\n");
  off += mvOsSPrintf(buf+off, "     Register Register Report   Data     Tx Byte  \n");
  off += mvOsSPrintf(buf+off, "     Request  Ack      Frames   Frames   Allow    \n");
  off += mvOsSPrintf(buf+off, " MAC Frames   Frames                              \n");
  off += mvOsSPrintf(buf+off, "--------------------------------------------------\n");

  for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++)
  {
      off += mvOsSPrintf(buf+off, "[%02d] %8u %8u %8u %8u %8u\n",
                        macId,
                        txCounters[macId].ctrlRegReqFramesCnt,
                        txCounters[macId].ctrlRegAckFramesCnt,
                        txCounters[macId].reportFramesCnt,
                        txCounters[macId].dataFramesCnt,
                        txCounters[macId].txAllowedBytesCnt);
  }
  off += mvOsSPrintf(buf+off, "--------------------------------------------------\n\r");

  return(off);
}

/*******************************************************************************
**
**  onuEponUiSwCountersShow
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print onu sw counters
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     message length
**
*******************************************************************************/
int onuEponUiSwCountersShow(char* buf)
{
	MV_STATUS       status;
	S_EponIoctlSwPm swCounters[EPON_MAX_MAC_NUM];
	int             macId;
	int             off = 0;

	for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++) {
		status = onuEponApiSwPmGet(&swCounters[macId], infoEponCntReadClearInd, macId);
		if (status != MV_OK) {
			off += mvOsSPrintf(buf+off, "Failed read Sw pm information!!!\n");
			return(off);
		}
	}

	off += mvOsSPrintf(buf+off, "\n");
	off += mvOsSPrintf(buf+off, "Sw Counters\n");
	off += mvOsSPrintf(buf+off, "----------------------------------------------------------------------------\n");
	off += mvOsSPrintf(buf+off, "     Total    Total    Total    Total    Total    Total    Total    Total   \n");
	off += mvOsSPrintf(buf+off, "     MPC Tx   MPC Tx   MPC Rx   MPC Rx   MPC Reg  MPC Reg  MPC Reg  MPC Reg \n");
	off += mvOsSPrintf(buf+off, "     Frames   Error    Frames   Error    Ack      Nack     De-Reg   Re-Reg  \n");
	off += mvOsSPrintf(buf+off, " MAC          Frames            Frames   Frames   Frames   Frames   Frames  \n");
	off += mvOsSPrintf(buf+off, "----------------------------------------------------------------------------\n");

	for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++) {
		off += mvOsSPrintf(buf+off, "[%02d] %8u %8u %8u %8u %8u %8u %8u %8u\n",
						   macId,
						   swCounters[macId].swTxCnt[TOTAL_MPCP_TX_FRAME_CNT],
						   swCounters[macId].swTxCnt[TOTAL_MPCP_TX_ERROR_FRAME_CNT],
						   swCounters[macId].swRxCnt[TOTAL_MPCP_RX_FRAME_CNT],
						   swCounters[macId].swRxCnt[TOTAL_MPCP_RX_ERROR_FRAME_CNT],
						   swCounters[macId].swRxCnt[TOTAL_MPCP_REGISTER_ACK_CNT],
						   swCounters[macId].swRxCnt[TOTAL_MPCP_REGISTER_NACK_CNT],
						   swCounters[macId].swRxCnt[TOTAL_MPCP_REGISTER_DEREG_FRAME_CNT],
						   swCounters[macId].swRxCnt[TOTAL_MPCP_REGISTER_REREG_FRAME_CNT]);
	}
	off += mvOsSPrintf(buf+off, "----------------------------------------------------------------------------\n");

	return(off);
}

/*******************************************************************************
**
**  onuEponUiGpmCountersShow
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print onu gpm counters
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     message length
**
*******************************************************************************/
int onuEponUiGpmCountersShow(char* buf)
{
  MV_STATUS        status;
  S_EponIoctlGpmPm gpmCounters[EPON_MAX_MAC_NUM];
  int              macId;
  int              off = 0;

  for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++)
  {
    status = onuEponApiGpmPmGet(&gpmCounters[macId], infoEponCntReadClearInd, macId);
    if (status != MV_OK)
    {
      off += mvOsSPrintf(buf+off, "Failed read Gpm pm information!!!\n");
      return(off);
    }
  }

  off += mvOsSPrintf(buf+off, "\n");
  off += mvOsSPrintf(buf+off, "GPM Counters\n");
  off += mvOsSPrintf(buf+off, "----------------------------------------------------------\n");
  off += mvOsSPrintf(buf+off, "     Valid    Grant    Min Proc Length   Discover Fifo    \n");
  off += mvOsSPrintf(buf+off, "     Grant    Max Time Time     Error    And      Full    \n");
  off += mvOsSPrintf(buf+off, " MAC          Error    Error             Register Error   \n");
  off += mvOsSPrintf(buf+off, "----------------------------------------------------------\n");

  for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++)
  {
     off += mvOsSPrintf(buf+off, "[%02d] %8u %8u %8u %8u %8u %8u\n",
                        macId,
                        gpmCounters[macId].grantValidCnt,
                        gpmCounters[macId].grantMaxFutureTimeErrorCnt,
                        gpmCounters[macId].minProcTimeErrorCnt,
                        gpmCounters[macId].lengthErrorCnt,
                        gpmCounters[macId].discoveryAndRegCnt,
                        gpmCounters[macId].fifoFullErrorCnt);
  }
  off += mvOsSPrintf(buf+off, "----------------------------------------------------------\n");
  off += mvOsSPrintf(buf+off, "     OPC      OPC      OPC      OPC      OPC      OPC     \n");
  off += mvOsSPrintf(buf+off, "     Discover Register Discover Drop     Hidden   Back to \n");
  off += mvOsSPrintf(buf+off, "     Not      Not      Not      Grant    Grant    Back    \n");
  off += mvOsSPrintf(buf+off, "     Register Discover Register                           \n");
  off += mvOsSPrintf(buf+off, "     Bcast             Or Bcast                           \n");
  off += mvOsSPrintf(buf+off, " MAC                                                      \n");
  off += mvOsSPrintf(buf+off, "----------------------------------------------------------\n");


  for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++)
  {
     off += mvOsSPrintf(buf+off, "[%02d] %8u %8u %8u %8u %8u %8u\n",
                        macId,
                        gpmCounters[macId].opcDiscoveryNotRegBcastCnt,
                        gpmCounters[macId].opcRegisterNotDiscoveryCnt,
                        gpmCounters[macId].opcDiscoveryNotRegNotBcastCnt,
                        gpmCounters[macId].opcDropGrantCnt,
                        gpmCounters[macId].opcHiddenGrantCnt,
                        gpmCounters[macId].opcBackToBackCnt);
  }
  off += mvOsSPrintf(buf+off, "----------------------------------------------------------\n\r");

  return(off);
}

/*******************************************************************************
**
**  onuEponUiAllCountersShow
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print all onu counters
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     message length
**
*******************************************************************************/
int onuEponUiAllCountersShow(char* buf)
{
	int              off = 0;

	off += onuEponUiRxCountersShow(buf);
	off += onuEponUiTxCountersShow(buf+off);
	off += onuEponUiSwCountersShow(buf+off);
	off += onuEponUiGpmCountersShow(buf+off);

	return(off);
}

/*******************************************************************************
**
**  onuEponUiPmHelpShow
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print pm help
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     message length
**
*******************************************************************************/
int onuEponUiPmHelpShow(char* buf)
{
	int off = 0;

	off += mvOsSPrintf(buf+off, "============================================================================\n");
	off += mvOsSPrintf(buf+off, "Configuration Commands[DEC]\n");
	off += mvOsSPrintf(buf+off, "============================================================================\n");
	off += mvOsSPrintf(buf+off, " echo [state]               > cntRdClrState - onu counters flag, Read[0], "
												"Read Clear[1]\n");
	off += mvOsSPrintf(buf+off, " echo [llid][Queue][#Bytes] > addDbaBytesPerLlidQueue - add bytes per LLID "
												"/ Queue\n");
	off += mvOsSPrintf(buf+off, " echo [llid][#Bytes]        > addDbaBytesPerLlidFifo - add bytes per LLID FIFO\n");
	off += mvOsSPrintf(buf+off, "============================================================================\n");
	off += mvOsSPrintf(buf+off, "Display Commands: cat <file>\n");
	off += mvOsSPrintf(buf+off, "============================================================================\n");
	off += mvOsSPrintf(buf+off, " cat rxCnt                    - dump Rx counters\n");
	off += mvOsSPrintf(buf+off, " cat txCnt                    - dump Tx counters\n");
	off += mvOsSPrintf(buf+off, " cat swCnt                    - dump Sw counters\n");
	off += mvOsSPrintf(buf+off, " cat gpmCnt                   - dump GPM counters\n");
	off += mvOsSPrintf(buf+off, " cat allCnt                   - dump All counters\n");
	off += mvOsSPrintf(buf+off, " cat showDbaBytesPerLlidQueue - dump byres addition per LLID / Queue\n");
	off += mvOsSPrintf(buf+off, " cat showDbaBytesPerLlidFifo  - dump byres addition per LLID FIFO\n");
	off += mvOsSPrintf(buf+off, " cat showRandom               - dump random generator counters\n");
	off += mvOsSPrintf(buf+off, " cat showPcsStat              - dump accumulates PCS statistics counters\n");
	off += mvOsSPrintf(buf+off, "============================================================================\n");

	return(off);
}

/*******************************************************************************
**
**  onuEponUiCountersReadClearFlag
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function config onu clear counters flag
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCountersReadClearFlag(MV_BOOL clear)
{
	infoEponCntReadClearInd = clear;
}

/*******************************************************************************
**
**  onuEponUiAddLlidQueueBytes
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function add bytes for LLID queue for DBA report
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiAddLlidQueueBytes(MV_U32 llId, MV_U32 queueId, MV_U32 counter)
{
  onuEponPmDbaAddBytePerLlidAndQueue(llId, queueId, counter);
}

/*******************************************************************************
**
**  onuEponUiAddLlidQueueBytes
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function show bytes added for LLID queue for DBA report
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
int onuEponUiShowLlidQueueBytes(char* buf)
{
  MV_U32 llldIndex;
  MV_U32 queueIndex;
  MV_U32 counter;
  int off = 0;

  off += mvOsSPrintf(buf+off, "\n");
  off += mvOsSPrintf(buf+off, "Additional Bytes per Queue\n");
  off += mvOsSPrintf(buf+off, "---------------------------------------------------------------------------\n");
  off += mvOsSPrintf(buf+off, "LLID Queue0   Queue1   Queue2   Queue3   Queue4   Queue5   Queue6   Queue7  \n");

  for (llldIndex = 0; llldIndex < EPON_MAX_MAC_NUM; llldIndex++)
  {
    off += mvOsSPrintf(buf+off, "[%02d]", llldIndex);

	for (queueIndex = 0; queueIndex < EPON_MAX_QUEUE; queueIndex++)
	{
	  counter = onuEponPmDbaAddBytePerLlidAndQueueGet(llldIndex, queueIndex);
	  off += mvOsSPrintf(buf+off, " %8u", counter);
	}
	off += mvOsSPrintf(buf+off, "\n");
  }

  off += mvOsSPrintf(buf+off, "---------------------------------------------------------------------------\n");

  return(off);
}

/*******************************************************************************
**
**  onuEponUiAddLlidFifoBytes
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function add bytes for LLID FIFO for DBA report
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiAddLlidFifoBytes(MV_U32 llId, MV_U32 counter)
{
  onuEponPmDbaAddBytePerLlidFifo(llId, counter);
}

/*******************************************************************************
**
**  onuEponUiShowLlidFifoBytes
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function show bytes added for queueset FIFO for DBA report
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
int onuEponUiShowLlidFifoBytes(char* buf)
{
  MV_U32 llldIndex;
  MV_U32 counter;
  int off = 0;

  off += mvOsSPrintf(buf+off, "\n");
  off += mvOsSPrintf(buf+off, "Additional Bytes per LLID FIFO\n");
  off += mvOsSPrintf(buf+off, "------------------------------------------------------------------------------\n");
  off += mvOsSPrintf(buf+off, "LLID0     LLID1     LLID2     LLID3     LLID4     LLID5     LLID6     LLID7   \n");

  for (llldIndex = 0; llldIndex < EPON_MAX_MAC_NUM; llldIndex++)
  {
	counter = onuEponPmDbaAddBytePerLlidFifoGet(llldIndex);
	off += mvOsSPrintf(buf+off, "%8u  ", counter);
  }
  off += mvOsSPrintf(buf+off, "\n");
  off += mvOsSPrintf(buf+off, "------------------------------------------------------------------------------\n");

  return(off);
}

/*******************************************************************************
**
**  onuEponUiShowRandom
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function show random generation counters
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
int onuEponUiShowRandom(char *buf)
{
	int off = 0;
	int i;
	MV_U32 randomShift = 0;
	MV_U32 randomRange = 0;

	off += mvOsSPrintf(buf+off, "\n");

	off += mvOsSPrintf(buf+off, "------------------------------------------------------------------------------\n");
	off += mvOsSPrintf(buf+off, "onuEponPmRandomGet was called %d times ", g_OnuEponRandomCount);

	if (g_OnuEponRandomCount != 0) {
		off += mvOsSPrintf(buf+off, "and the following random values were generated:");
		for (i = 0; i < g_OnuEponRandomCount; i++) {
			if ((i % 4) == 0) /* 4 values in one row */
				off += mvOsSPrintf(buf+off, "\n");
			off += mvOsSPrintf(buf+off, "0x%08x\t", g_OnuEponRandomValues[i]);
		}
		off += mvOsSPrintf(buf+off, "\n");
		off += mvOsSPrintf(buf+off, "------------------------------------------------------------------------------\n");
		off += mvOsSPrintf(buf+off, "The same random values after processing:");
		for (i = 0; i < g_OnuEponRandomCount; i++) {
			if ((i % 4) == 0) /* 4 values in one row */
				off += mvOsSPrintf(buf+off, "\n");

			randomShift = ((g_OnuEponRandomValues[i] >> 16) & 0xF);
			randomRange = ((g_OnuEponRandomValues[i] >> randomShift) & regRandomMaskThreshold);
			off += mvOsSPrintf(buf+off, "%04d\t", randomRange);
		}
		off += mvOsSPrintf(buf+off, "\n");
	} else
		off += mvOsSPrintf(buf+off, ", no random values were generated\n");

	off += mvOsSPrintf(buf+off, "------------------------------------------------------------------------------\n");
	return(off);
}

/*******************************************************************************
**
**  onuEponUiShowPcsStat
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function show PCS statistics
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
int onuEponUiShowPcsStat(char *buf)
{
	S_PcsPm statCounters;
	int off = 0;

	off += mvOsSPrintf(buf+off, "------------------------------------------------------------------------------\n");

	onuEponPmPcsPmGet(&statCounters);

	off += mvOsSPrintf(buf+off, "Accumulated counter values in DB:\n");
	off += mvOsSPrintf(buf+off, "\n");
	off += mvOsSPrintf(buf+off, "legalFrameCnt                     0x%08x\n", statCounters.legalFrameCnt);
	off += mvOsSPrintf(buf+off, "maxFrameSizeErrCnt                0x%08x\n", statCounters.maxFrameSizeErrCnt);
	off += mvOsSPrintf(buf+off, "parityLenErrCnt                   0x%08x\n", statCounters.parityLenErrCnt);
	off += mvOsSPrintf(buf+off, "longGateErrCnt                    0x%08x\n", statCounters.longGateErrCnt);
	off += mvOsSPrintf(buf+off, "protocolErrCnt                    0x%08x\n", statCounters.protocolErrCnt);
	off += mvOsSPrintf(buf+off, "minFrameSizeErrCnt                0x%08x\n", statCounters.minFrameSizeErrCnt);
	off += mvOsSPrintf(buf+off, "legalFecFrameCnt                  0x%08x\n", statCounters.legalFecFrameCnt);
	off += mvOsSPrintf(buf+off, "legalNonFecFrameCnt               0x%08x\n", statCounters.legalNonFecFrameCnt);
	off += mvOsSPrintf(buf+off, "------------------------------------------------------------------------------\n");

	return(off);
}

/*******************************************************************************
**
**  onuEponUiCfgHelpShow
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print cfg help
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     message length
**
*******************************************************************************/
int onuEponUiCfgHelpShow(char* buf)
{
	int off = 0;
	MV_U32 devId = mvCtrlModelGet();

	off += mvOsSPrintf(buf+off, "============================================================================\n");
	off += mvOsSPrintf(buf+off, "Configuration Commands[HEX]\n");
	off += mvOsSPrintf(buf+off, "============================================================================\n");
	off += mvOsSPrintf(buf+off, " echo [enable]        > rxEn            - onu RX enbale, Disable[0], Enable[1]\n");
	off += mvOsSPrintf(buf+off, " echo [enable][macId] > txEn            - onu mac TX enbale, Disable[0], Enable[1]\n");
	off += mvOsSPrintf(buf+off, " echo [enable]        > ctrlTimerEn     - control timer for  reception of MPCP "
										"/ GATES, Disable[0], Enable[1]\n");
	off += mvOsSPrintf(buf+off, " echo [interval]      > ctrlTimerVal    - control timer interval\n");
	off += mvOsSPrintf(buf+off, " echo [limit][mask]   > ctrlTimerRandom - control timer random reset, expire limit"
										" and expire duration\n");
	off += mvOsSPrintf(buf+off, " echo [enable][T0 val][T0 time][T1 val][T1 time]\n");
	off += mvOsSPrintf(buf+off, "                      > rprtTimerCfg - report dba timer config\n");
	off += mvOsSPrintf(buf+off, " echo [queue][Threshold][llid]\n");
	off += mvOsSPrintf(buf+off, "                      > dbaHwParamsCfg   - configuration of HW DBA queues "
										"(Max Threshold 0xFFFF)\n");
	off += mvOsSPrintf(buf+off, " echo [interval]      > dbaSwIntervalCfg - configure SW report interval\n");
	off += mvOsSPrintf(buf+off, " echo [#qset][qBitMap][T0][T1][T2]\n");
	off += mvOsSPrintf(buf+off, "                      > dbaSwParamsCfg   - simulate configuration of SW DBA "
										"queues for LLID 0\n");
	off += mvOsSPrintf(buf+off, " echo [qset][queue]   > dbaSwOptionCfg   - configure SW report direction [0]normal"
										" [1]reverse\n");
	off += mvOsSPrintf(buf+off, " echo [time][state]   > holdoverDebug    - simulate configuration of Holdover\n");
	off += mvOsSPrintf(buf+off, " echo [state]         > silenceMode      - Set Silence Mode\n");
	off += mvOsSPrintf(buf+off, " echo [state]         > silenceSim       - simulate Silence Mode\n");
	off += mvOsSPrintf(buf+off, " echo [dummy]         > dgDebug          - simulate reception of Dying Gasp "
										"Interrupt\n");
	off += mvOsSPrintf(buf+off, " echo [#Queue][Thresh]> dgCfg            - configure Dying Gasp DBA report\n");
	off += mvOsSPrintf(buf+off, " echo [module][Level][Options] > printMask - change printing options\n");
	off += mvOsSPrintf(buf+off, " echo [enable][pattern][burst][duration]\n");
	off += mvOsSPrintf(buf+off, "      [period]        > pattern          - start enable[1] or stop[0] transmission"
										" of pattern: 0x1-1T, 0x2-2T, 0x80-RPBS"
										"-9, 0x82-RPBS-15, 0x83-RPBS-23 in "
										"burst: 0-static, 1- periodic for "
										"duration: peac time interval"
										"[micro seconds] in period - full cicle"
										" time interval[micro seconds]\n");
	if (devId == MV_6601_DEV_ID) {
		off += mvOsSPrintf(buf+off, " echo [latency][size] > pcsFrameLimits   - Configure PCS Frame Size "
											"limits\n");
		off += mvOsSPrintf(buf+off, " echo [threshold]     > rxDataFifoThresh - Configure RX Parser Data FIFO "
											"threshold\n");
		off += mvOsSPrintf(buf+off, " echo [txMask][txP][serdesP][xvrP][burstEn] > ddmTxPolarity - Configure "
											" DDM Polarity settings\n");
	}
	off += mvOsSPrintf(buf+off, "============================================================================\n");
	off += mvOsSPrintf(buf+off, "Display Commands: cat <file>\n");
	off += mvOsSPrintf(buf+off, "============================================================================\n");
	off += mvOsSPrintf(buf+off, " cat printMask                           - dump printing options\n");
	off += mvOsSPrintf(buf+off, " cat dbaSwCfgPrint                       - dump DBA cfg\n");
	if (devId == MV_6601_DEV_ID) {
		off += mvOsSPrintf(buf+off, " cat pcsFrameLimits                     - dump PCS Frame Size Limits\n");
		off += mvOsSPrintf(buf+off, " cat rxDataFifoThresh                   - dump RX Parser Data FIFO "
											"threshold\n");
	}
	off += mvOsSPrintf(buf+off, "============================================================================\n");

	return(off);
}

/*******************************************************************************
**
**  onuEponUiCountersReadClearFlag
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function config onu RX enable
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgMacRxEn(MV_U32 enable)
{
  mvOnuEponMacOnuRxEnableSet(enable);
}

/*******************************************************************************
**
**  onuEponUiCfgMacTxEn
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function config onu mac TX enable
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgMacTxEn(MV_U32 enable, MV_U32 macId)
{
	onuEponMiCfgMacTxEn(enable, macId);
}

/*******************************************************************************
**
**  onuEponUiCfgMpcpTimerEn
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function config onu mac MPCP timer enable
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgMpcpTimerEn(MV_U32 enable)
{
  onuEponIsrTimerMpcpStateSet((enable == 1) ? (MV_TRUE) : (MV_FALSE));
}

/*******************************************************************************
**
**  onuEponUiCfgMpcpTimerVal
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function config onu mac MPCP timer enable
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgMpcpTimerVal(MV_U32 value)
{
  onuEponIsrTimerMpcpIntervalSet(value);
}

/*******************************************************************************
**
**  onuEponUiCfgMpcpTimerRandom
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function config onu mac MPCP timer random reset
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgMpcpTimerRandom(MV_U32 limit, MV_U32 mask)
{
  onuEponIsrTimerMpcpRandomResetSet(limit, mask);
}

/*******************************************************************************
**
**  onuEponUiCfgRprtTimerEn
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function config onu periodic report message transmit
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgRprtTimerEn(MV_U32 enable, MV_U32 t0_val, MV_U32 t0_time, MV_U32 t1_val, MV_U32 t1_time)
{
  onuEponIsrTimerHwReportStateSet(((enable == 1) ? (MV_TRUE) : (MV_FALSE)),
							   	  t0_val, t0_time, t1_val, t1_time);
}

/*******************************************************************************
**
**  onuEponUiCfgDbaHwDebug
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure HW DBA queues thresholds for LLID 0
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgDbaHwDebug(MV_U32 queueId, MV_U32 threshold, MV_U32 llid)
{
  mvOnuEponMacTxmCppReportQueueX(queueId, (threshold / 64), llid);
}

/*******************************************************************************
**
**  onuEponUiCfgDbaSwRprtDir
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure DBA SW report direction [0]Normal [1]Reverse
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgDbaSwRprtDir(MV_U32 queueSetDir, MV_U32 queueDir)
{
  onuEponDbOnuDbaSwModeQueuesetDirSet(queueSetDir);
  onuEponDbOnuDbaSwModeQueueDirSet(queueDir);
}

/*******************************************************************************
**
**  onuEponUiCfgDbaSwDebug
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function simulate configuration of DBA queues for LLID 0
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgDbaSwDebug(MV_U32 queueSet, MV_U32 queueBitMap,
						    MV_U32 queueSet0, MV_U32 queueSet1, MV_U32 queueSet2, MV_U32 queueSet3)
{
  MV_U32         idx;
  MV_U32         qsetidx;
  MV_U32         numOfQueues = 0;
  S_EponIoctlDba ioctlDba;

  memset (&ioctlDba, 0, sizeof(S_EponIoctlDba));

  for (qsetidx = 0; qsetidx < queueSet; qsetidx++)
  {
	for (idx = 0; idx < EPON_MAX_QUEUE; idx++)
	{
	  if (queueBitMap & (1 << idx))
	  {
		if (qsetidx == 0)
		{
			ioctlDba.dbaLlid[0].threshold[qsetidx][idx].state     = 1;
			ioctlDba.dbaLlid[0].threshold[qsetidx][idx].threshold = queueSet0;
		}
		else if (qsetidx == 1)
		{
			ioctlDba.dbaLlid[0].threshold[qsetidx][idx].state     = 1;
			ioctlDba.dbaLlid[0].threshold[qsetidx][idx].threshold = queueSet1;
		}
		else if (qsetidx == 2)
		{
			ioctlDba.dbaLlid[0].threshold[qsetidx][idx].state     = 1;
			ioctlDba.dbaLlid[0].threshold[qsetidx][idx].threshold = queueSet2;
		}
		else if (qsetidx == 3)
		{
			ioctlDba.dbaLlid[0].threshold[qsetidx][idx].state     = 1;
			ioctlDba.dbaLlid[0].threshold[qsetidx][idx].threshold = queueSet3;
		}

		if (qsetidx == 0) numOfQueues++;
	  }
	}
  }

  ioctlDba.validLlid[0]              = 1;
  ioctlDba.dbaLlid[0].numOfQueueSets = queueSet;
  ioctlDba.dbaLlid[0].numOfQueues    = numOfQueues;

  onuEponMiDbaConfig(&ioctlDba);
}

/*******************************************************************************
**
**  onuEponUiCfgDgCfg
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure DG DBA report
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgDgCfg(MV_U32 numOfQueue, MV_U32 queueReport)
{
  onuEponPonMngDbaDgSwRprtCfg(numOfQueue, queueReport);
}

/*******************************************************************************
**
**  onuEponUiCfgDbaSwInterval
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure Sw DBA interval
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgDbaSwInterval(MV_U32 interval)
{
  onuEponDbOnuSwRprtMacTimerIntervalSet(interval);
}

/*******************************************************************************
**
**  onuEponUiCfgDbaCfgPrint
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print configuration of DBA queues for LLID 0
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
int onuEponUiCfgDbaCfgPrint(char* buf)
{
  MV_U32 llidIdx;
  MV_U32 queueSetIdx;
  MV_U32 queueIdx;
  MV_U32 threshold;
  MV_U32 state;
  MV_U32 numOfQueueSets;
  MV_U32 numOfQueues;
  int off = 0;

  for (llidIdx = 0; llidIdx < EPON_MAX_MAC_NUM; llidIdx++)
  {
    if (onuEponDbOnuDbaLlidActiveStatusGet(llidIdx) == ONU_DBA_ACTIVE_LLID)
    {
      numOfQueueSets = onuEponDbOnuDbaNumOfQueuesetGet(llidIdx);
      numOfQueues    = onuEponDbOnuDbaNumOfQueuesGet(llidIdx);

  	  off += mvOsSPrintf(buf+off, "LLID[%d]: #Queuesets = (%d), #Queues = (%d)\n",
  			             llidIdx, numOfQueueSets, numOfQueues);

      for (queueSetIdx = 0; queueSetIdx < numOfQueueSets; queueSetIdx++)
      {
        off += mvOsSPrintf(buf+off, "Queueset[%d]: Queues:", queueSetIdx);

        /* Queue Id */
        /* ======== */
        for (queueIdx = 0; queueIdx < EPON_MAX_QUEUE; queueIdx++)
        {
      	  onuEponDbOnuDbaQueueThresholdGet(&threshold, &state, queueIdx, queueSetIdx, llidIdx);

      	  off += mvOsSPrintf(buf+off, " [%d-%3s-0x%04x]",
                             queueIdx, ((state == 0) ? "off" : "on"), ((state == 0) ? 0 : threshold));
        }

        off += mvOsSPrintf(buf+off, "\n");
      }
    }
  }

  return(off);
}

/*******************************************************************************
**
**  onuEponUiCfgDbaStaticCfgPrint
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print static configuration of DBA info
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
int onuEponUiCfgDbaStaticCfgPrint(char* buf)
{
  MV_U32 llidIdx;
  MV_U32 queueSetIdx;
  MV_U32 queueIdx;
  MV_U32 threshold;
  MV_U32 state;
  MV_U32 numOfQueueSets;
  MV_U32 numOfQueues;
  int off = 0;

  for (llidIdx = 0; llidIdx < EPON_MAX_MAC_NUM; llidIdx++)
  {
    if (onuEponDbOnuDbaLlidActiveStatusGet(llidIdx) == ONU_DBA_ACTIVE_LLID)
    {
      numOfQueueSets = onuEponDbOnuDbaNumOfQueuesetGet(llidIdx);
      numOfQueues    = onuEponDbOnuDbaNumOfQueuesGet(llidIdx);

  	  off += mvOsSPrintf(buf+off, "LLID[%d]: #Queuesets = (%d), #Queues = (%d)\n",
  			             llidIdx, numOfQueueSets, numOfQueues);

      for (queueSetIdx = 0; queueSetIdx < numOfQueueSets; queueSetIdx++)
      {
        off += mvOsSPrintf(buf+off, "Queueset[%d]: Queues:", queueSetIdx);

        /* Queue Id */
        /* ======== */
        for (queueIdx = 0; queueIdx < EPON_MAX_QUEUE; queueIdx++)
        {
      	  onuEponDbOnuDbaQueueThresholdGet(&threshold, &state, queueIdx, queueSetIdx, llidIdx);

      	  off += mvOsSPrintf(buf+off, " [%d-%3s-0x%04x]",
                             queueIdx, ((state == 0) ? "off" : "on"), ((state == 0) ? 0 : threshold));
        }

        off += mvOsSPrintf(buf+off, "\n");
      }
    }
  }

  return(off);
}



/*******************************************************************************
**
**  onuEponUiCfgHoldoverDebug
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function simulate configuration of Holdover
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgHoldoverDebug(MV_U32 time, MV_U32 state)
{
  S_EponIoctlHoldOver ioctlHoldover;

  ioctlHoldover.holdoverState = state;
  ioctlHoldover.holdoverTime  = time;

  onuEponMiHoldoverConfig(&ioctlHoldover);
}

/*******************************************************************************
**
**  onuEponUiCfgSilenceModeDebug
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function simulate configuration of Silence
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgSilenceModeDebug(MV_U32 state)
{
  S_EponIoctlSilence ioctlSilence;

  ioctlSilence.silenceState = state;

  onuEponMiSilenceConfig(&ioctlSilence);
}

/*******************************************************************************
**
**  onuEponUiCfgSilenceSimDebug
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function simulate configuration of Silence
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgSilenceSimDebug(void)
{
  MV_STATUS status;
  S_EponIoctlSilence ioctlSilence;

  ioctlSilence.silenceState = 1;

  onuEponMiSilenceConfig(&ioctlSilence);

  status  = mvOnuEponMacGenOnuConfigAutoAckSet(0);
  status |= mvOnuEponMacGenOnuConfigAutoRequestSet(0);
  if (status != MV_OK)
  {
	mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
			   "ERROR: (%s:%d) Failed to disable auto request / ack\n\r", __FILE_DESC__, __LINE__);
	return;
  }

  onuPonTimerUpdate(&(onuPonResourceTbl_s.onuPonSilenceTimerId[0]), 0, ONU_PON_TIMER_SILENCE_INTERVAL, 1);
}

/*******************************************************************************
**
**  onuEponUiCfgSilenceDebug
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function simulate reception of DG interrupt
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgDgDebug(MV_U32 dummy)
{
  mvOnuPonMacDgInterruptDis();
  onuPonDyingGaspProcess();
  onuEponDgIsrRoutine();
}

/*******************************************************************************
**
**  onuEponUiCfgSetPatternBurst
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function config start/stop onu transmission of pattern burst
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgSetPatternBurst(MV_U32 enable, MV_U32 pattern, MV_U32 burst, MV_U32 duration, MV_U32 period)
{
  if (enable == 0)
  {
    onuPonPatternBurstOff();
  }
  else
  {
    onuPonPatternBurstOn(pattern, (MV_BOOL)burst, period, duration);
  }
}

/*******************************************************************************
**
**  onuEponUiCfgSetPcsFrameLimits
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configures PCS Frame Limits
**
**  PARAMETERS:  MV_U32 latency - FED decoder latency in unit of clock cycles
**               MV_U32 size    - Maximum frame size
**
**  OUTPUTS:     none
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgSetPcsFrameLimits(MV_U32 latency, MV_U32 size)
{
	mvOnuEponMacPcsFrameSizeLimitsSet(size, latency);
}

/*******************************************************************************
**
**  onuEponUiCfgShowPcsFrameLimits
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function shows PCS Frame Limits
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
int onuEponUiCfgShowPcsFrameLimits(char* buf)
{
	int off = 0;
	MV_STATUS status;
	MV_U32 size,latency;

	status = mvOnuEponMacPcsFrameSizeLimitsGet(&size, &latency);

	if (status != MV_OK)
		off += mvOsSPrintf(buf+off, "Failed to obtain PCS Frame Limits!\n");
	else {
		off += mvOsSPrintf(buf+off, "\n");
		off += mvOsSPrintf(buf+off, "FEC Decoder Latency = %#x\n", latency);
		off += mvOsSPrintf(buf+off, "Maximum Frame Size  = %#x\n", size);
	}

	return(off);
}

/*******************************************************************************
**
**  onuEponUiCfgSetRxDataFifoThresh
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configures RX Parser Data FIFO Threshold
**
**  PARAMETERS:  MV_U32 threshold - RX Data FIFO MAX allowed capacity
**
**  OUTPUTS:     none
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgSetRxDataFifoThresh(MV_U32 threshold)
{
	mvOnuEponMacRxpDataFifoThresholdSet(threshold);
}

/*******************************************************************************
**
**  onuEponUiCfgShowRxDataFifoThresh
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function shows RX Parser Data FIFO Threshold
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     void
**
*******************************************************************************/
int onuEponUiCfgShowRxDataFifoThresh(char* buf)
{
	int off = 0;
	MV_STATUS status;
	MV_U32 threshold;

	status = mvOnuEponMacRxpDataFifoThresholdGet(&threshold);

	if (status != MV_OK)
		off += mvOsSPrintf(buf+off, "Failed to obtain RX Parser Data FIFO Threshold!\n");
	else {
		off += mvOsSPrintf(buf+off, "\n");
		off += mvOsSPrintf(buf+off, "RX Parser Data FIFO Threshold = %#x\n", threshold);
	}

	return(off);
}

/*******************************************************************************
**
**  onuEponUiCfgSetDdmTxPolarity
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configures DDM TX polarity
**
**  PARAMETERS:  MV_U32 txEnable
**               MV_U32 txPol
**               MV_U32 serdesPol
**               MV_U32 xvrPol
**               MV_U32 burstEn
**
**  OUTPUTS:     none
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponUiCfgSetDdmTxPolarity(MV_U32 txEnable, MV_U32 txPol, MV_U32 serdesPol, MV_U32 xvrPol, MV_U32 burstEn)
{
#if (!defined PON_Z1) && (!defined PON_Z2)
	mvOnuEponMacDdmTxPolaritySet(txEnable, txPol, serdesPol, xvrPol, burstEn);
#endif
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/*                            ONU SYS FS Definition                           */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* ========================================================================== */
/*                            Info Group                                      */
/* ========================================================================== */
/******************************************************************************/

/* ========================================================================== */
/*                            Info Commands                                   */
/* ========================================================================== */
static ssize_t epon_info_show(struct device *dev,
                              struct device_attribute *attr,
                              char *buf)
{
  const char* name = attr->attr.name;

  if (!capable(CAP_NET_ADMIN))
  	return -EPERM;

  if      (!strcmp(name, "infoEpon"))
  	return(onuEponUiInfoShow(buf));
  else if (!strcmp(name, "alarmEpon"))
    return(onuEponUiAlarmShow(buf));
  else if (!strcmp(name, "helpEInfo"))
    return(onuEponUiInfoHelpShow(buf));

  return 0;
}

static ssize_t epon_info_store(struct device *dev,
                               struct device_attribute *attr,
                               const char *buf, size_t len)
{
  const char* name = attr->attr.name;
  unsigned long flags;
  unsigned int  param1 = 0;
  unsigned int  param2 = 0;
  unsigned int  param3 = 0;

  if (!capable(CAP_NET_ADMIN))
    return -EPERM;

  sscanf(buf, "%x %x %x", &param1, &param2, &param3);

  raw_local_irq_save(flags);

  if      (!strcmp(name, "readRegEpon"))
    onuEponUiDebugReadReg((MV_U32)param1, (MV_U32)param2);                     /* readOffset, readEntry */
  else if (!strcmp(name, "writeRegEpon"))
    onuEponUiDebugWriteReg((MV_U32)param1, (MV_U32)param2, (MV_U32)param3);    /*  writeOffset, writeEntry, regValue */
  else
    printk("%s: illegal operation <%s>\n", __FUNCTION__, attr->attr.name);

  raw_local_irq_restore(flags);

  return(len);
}

/* ========================================================================== */
/*                            Info Attributes                                 */
/* ========================================================================== */
static DEVICE_ATTR(infoEpon,     S_IRUSR, epon_info_show, epon_info_store);
static DEVICE_ATTR(alarmEpon,    S_IRUSR, epon_info_show, epon_info_store);
static DEVICE_ATTR(helpEInfo,    S_IRUSR, epon_info_show, epon_info_store);
static DEVICE_ATTR(readRegEpon,  S_IWUSR, epon_info_show, epon_info_store);
static DEVICE_ATTR(writeRegEpon, S_IWUSR, epon_info_show, epon_info_store);

static struct attribute *epon_info_attrs[] = {
	&dev_attr_infoEpon.attr,
	&dev_attr_alarmEpon.attr,
    &dev_attr_readRegEpon.attr,
    &dev_attr_writeRegEpon.attr,
    &dev_attr_helpEInfo.attr,
	NULL
};

static struct attribute_group epon_info_group =
{
  .name = "info",
  .attrs = epon_info_attrs,
};

/******************************************************************************/
/* ========================================================================== */
/*                            Pm Group                                        */
/* ========================================================================== */
/******************************************************************************/

/* ========================================================================== */
/*                            Pm Commands                                     */
/* ========================================================================== */
static ssize_t epon_pm_show(struct device *dev,
                            struct device_attribute *attr,
                            char *buf)
{
	const char* name = attr->attr.name;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!strcmp(name, "rxCnt"))
		return(onuEponUiRxCountersShow(buf));
	else if (!strcmp(name, "txCnt"))
		return(onuEponUiTxCountersShow(buf));
	else if (!strcmp(name, "swCnt"))
		return(onuEponUiSwCountersShow(buf));
	else if (!strcmp(name, "gpmCnt"))
		return(onuEponUiGpmCountersShow(buf));
	else if (!strcmp(name, "allCnt"))
		return(onuEponUiAllCountersShow(buf));
	else if (!strcmp(name, "showDbaBytesPerLlidQueue"))
		return(onuEponUiShowLlidQueueBytes(buf));
	else if (!strcmp(name, "showDbaBytesPerLlidFifo"))
		return(onuEponUiShowLlidFifoBytes(buf));
	else if (!strcmp(name, "showRandom"))
		return(onuEponUiShowRandom(buf));
	else if (!strcmp(name, "showPcsStat"))
		return(onuEponUiShowPcsStat(buf));
	else if (!strcmp(name, "helpPm"))
		return(onuEponUiPmHelpShow(buf));

	return 0;
}

static ssize_t epon_pm_store(struct device *dev,
                             struct device_attribute *attr,
                             const char *buf, size_t len)
{
	const char* name = attr->attr.name;
	unsigned long flags;
	unsigned int  param1 = 0;
	unsigned int  param2 = 0;
	unsigned int  param3 = 0;


	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	sscanf(buf, "%d %d %d", &param1, &param2, &param3);

	raw_local_irq_save(flags);

	if (!strcmp(name, "cntRdClrState"))
		onuEponUiCountersReadClearFlag((MV_BOOL)param1);
	else if (!strcmp(name, "addDbaBytesPerLlidQueue"))
		onuEponUiAddLlidQueueBytes(param1, param2, param3);
	else if (!strcmp(name, "addDbaBytesPerLlidFifo"))
		onuEponUiAddLlidFifoBytes(param1, param2);
	else
		printk(KERN_ERR "%s: illegal operation <%s>\n", __func__, attr->attr.name);

	raw_local_irq_restore(flags);

  return(len);
}

/* ========================================================================== */
/*                            PM Attributes                                   */
/* ========================================================================== */
static DEVICE_ATTR(rxCnt,                    S_IRUSR, epon_pm_show, epon_pm_store);
static DEVICE_ATTR(txCnt,                    S_IRUSR, epon_pm_show, epon_pm_store);
static DEVICE_ATTR(swCnt,                    S_IRUSR, epon_pm_show, epon_pm_store);
static DEVICE_ATTR(gpmCnt,                   S_IRUSR, epon_pm_show, epon_pm_store);
static DEVICE_ATTR(allCnt,                   S_IRUSR, epon_pm_show, epon_pm_store);
static DEVICE_ATTR(showDbaBytesPerLlidQueue, S_IRUSR, epon_pm_show, epon_pm_store);
static DEVICE_ATTR(addDbaBytesPerLlidQueue,  S_IWUSR, epon_pm_show, epon_pm_store);
static DEVICE_ATTR(showDbaBytesPerLlidFifo,  S_IRUSR, epon_pm_show, epon_pm_store);
static DEVICE_ATTR(addDbaBytesPerLlidFifo,   S_IWUSR, epon_pm_show, epon_pm_store);
static DEVICE_ATTR(cntRdClrState,            S_IWUSR, epon_pm_show, epon_pm_store);
static DEVICE_ATTR(showRandom,               S_IRUSR, epon_pm_show, epon_pm_store);
static DEVICE_ATTR(showPcsStat,              S_IRUSR, epon_pm_show, epon_pm_store);
static DEVICE_ATTR(helpPm,                   S_IRUSR, epon_pm_show, epon_pm_store);


static struct attribute *epon_pm_attrs[] = {
	&dev_attr_rxCnt.attr,
	&dev_attr_txCnt.attr,
	&dev_attr_swCnt.attr,
	&dev_attr_gpmCnt.attr,
	&dev_attr_allCnt.attr,
	&dev_attr_showDbaBytesPerLlidQueue.attr,
	&dev_attr_addDbaBytesPerLlidQueue.attr,
	&dev_attr_showDbaBytesPerLlidFifo.attr,
	&dev_attr_addDbaBytesPerLlidFifo.attr,
	&dev_attr_cntRdClrState.attr,
	&dev_attr_showRandom.attr,
	&dev_attr_showPcsStat.attr,
	&dev_attr_helpPm.attr,
	NULL
};

static struct attribute_group epon_pm_group =
{
  .name = "pm",
  .attrs = epon_pm_attrs,
};


/******************************************************************************/
/* ========================================================================== */
/*                            Cfg Group                                       */
/* ========================================================================== */
/******************************************************************************/

/* ========================================================================== */
/*                            Cfg Commands                                    */
/* ========================================================================== */
static ssize_t cfg_show(struct device *dev,
                        struct device_attribute *attr,
                        char *buf)
{
	const char* name = attr->attr.name;
	MV_U32 devId = mvCtrlModelGet();

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!strcmp(name, "printMask"))
		return ponOnuPrintStatus(buf);
	else if (!strcmp(name, "dbaSwCfgPrint"))
		return(onuEponUiCfgDbaCfgPrint(buf));
	else if (!strcmp(name, "dbaStaticCfgPrint"))
		return(onuEponUiCfgDbaStaticCfgPrint(buf));
	else if (!strcmp(name, "helpCfg"))
		return(onuEponUiCfgHelpShow(buf));
	else if (devId == MV_6601_DEV_ID) {
		if (!strcmp(name, "pcsFrameLimits"))
			return(onuEponUiCfgShowPcsFrameLimits(buf));
		else if (!strcmp(name, "rxDataFifoThresh"))
			return(onuEponUiCfgShowRxDataFifoThresh(buf));
	}

	return 0;
}


static ssize_t cfg_store(struct device *dev,
                         struct device_attribute *attr,
                         const char *buf, size_t len)
{
	const char* name = attr->attr.name;
	unsigned long flags;
	unsigned int  param1 = 0;
	unsigned int  param2 = 0;
	unsigned int  param3 = 0;
	unsigned int  param4 = 0;
	unsigned int  param5 = 0;
	unsigned int  param6 = 0;
	MV_U32 devId = mvCtrlModelGet();

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	sscanf(buf, "%x %x %x %x %x %x", &param1, &param2, &param3, &param4, &param5, &param6);

	raw_local_irq_save(flags);

	if (!strcmp(name, "rxEn"))
		onuEponUiCfgMacRxEn((MV_U32)param1);
	else if (!strcmp(name, "txEn"))
		onuEponUiCfgMacTxEn((MV_U32)param1, (MV_U32)param2);
	else if (!strcmp(name, "ctrlTimerEn"))
		onuEponUiCfgMpcpTimerEn((MV_U32)param1);
	else if (!strcmp(name, "ctrlTimerVal"))
		onuEponUiCfgMpcpTimerVal((MV_U32)param1);
	else if (!strcmp(name, "ctrlTimerRandom"))
		onuEponUiCfgMpcpTimerRandom((MV_U32)param1, (MV_U32)param2);
	else if (!strcmp(name, "rprtTimerCfg"))
		onuEponUiCfgRprtTimerEn((MV_U32)param1, (MV_U32)param2, (MV_U32)param3, (MV_U32)param4, (MV_U32)param5);
	else if (!strcmp(name, "dbaHwParamsCfg"))
		onuEponUiCfgDbaHwDebug((MV_U32)param1, (MV_U32)param2, (MV_U32)param3);
	else if (!strcmp(name, "dbaSwIntervalCfg"))
		onuEponUiCfgDbaSwInterval((MV_U32)param1);
	else if (!strcmp(name, "dbaSwParamsCfg"))
		onuEponUiCfgDbaSwDebug((MV_U32)param1, (MV_U32)param2, (MV_U32)param3, (MV_U32)param4,
				       (MV_U32)param5, (MV_U32)param6);
	else if (!strcmp(name, "dbaSwOptionCfg"))
		onuEponUiCfgDbaSwRprtDir((MV_U32)param1, (MV_U32)param2);
	else if (!strcmp(name, "holdoverDebug"))
		onuEponUiCfgHoldoverDebug((MV_U32)param1, (MV_U32)param2);
	else if (!strcmp(name, "silenceMode"))
		onuEponUiCfgSilenceModeDebug((MV_U32)param1);
	else if (!strcmp(name, "silenceSim"))
		onuEponUiCfgSilenceSimDebug();
	else if (!strcmp(name, "dgDebug"))
		onuEponUiCfgDgDebug((MV_U32)param1);
	else if (!strcmp(name, "dgCfg"))
		onuEponUiCfgDgCfg((MV_U32)param1, (MV_U32)param2);
	else if (!strcmp(name, "printMask"))					/* module, print level, options */
		ponOnuChangePrintStatus((MV_U32)param1, (MV_U32)param2, (MV_U32)param3);
	else if (!strcmp(name, "pattern"))
		onuEponUiCfgSetPatternBurst((MV_U32)param1,(MV_U32)param2,(MV_U32)param3,(MV_U32)param4,(MV_U32)param5);
	else if (devId == MV_6601_DEV_ID) {
		if (!strcmp(name, "pcsFrameLimits"))
			onuEponUiCfgSetPcsFrameLimits((MV_U32)param1, (MV_U32)param2);
		else if (!strcmp(name, "rxDataFifoThresh"))
			onuEponUiCfgSetRxDataFifoThresh((MV_U32)param1);
		else if (!strcmp(name, "ddmTxPolarity")) /* txEnable, txPol, serdesPol, xvrPol, burstEn */
			onuEponUiCfgSetDdmTxPolarity((MV_U32)param1,(MV_U32)param2,(MV_U32)param3,
						     (MV_U32)param4,(MV_U32)param5);
		else
			printk(KERN_ERR "%s: illegal operation <%s>\n", __func__, attr->attr.name);
	} else
		printk("%s: illegal operation <%s>\n", __func__, attr->attr.name);

	raw_local_irq_restore(flags);

	return(len);
}

/* ========================================================================== */
/*                            Cfg Attributes                                  */
/* ========================================================================== */
static DEVICE_ATTR(rxEn,                    S_IWUSR, cfg_show, cfg_store);
static DEVICE_ATTR(txEn,                    S_IWUSR, cfg_show, cfg_store);
static DEVICE_ATTR(ctrlTimerEn,             S_IWUSR, cfg_show, cfg_store);
static DEVICE_ATTR(ctrlTimerVal,            S_IWUSR, cfg_show, cfg_store);
static DEVICE_ATTR(ctrlTimerRandom,         S_IWUSR, cfg_show, cfg_store);
static DEVICE_ATTR(rprtTimerCfg,            S_IWUSR, cfg_show, cfg_store);
static DEVICE_ATTR(dbaHwParamsCfg,          S_IWUSR, cfg_show, cfg_store);
static DEVICE_ATTR(dbaSwIntervalCfg,        S_IWUSR, cfg_show, cfg_store);
static DEVICE_ATTR(dbaSwParamsCfg,          S_IWUSR, cfg_show, cfg_store);
static DEVICE_ATTR(dbaSwOptionCfg,          S_IWUSR, cfg_show, cfg_store);
static DEVICE_ATTR(dbaSwCfgPrint,           S_IRUSR, cfg_show, cfg_store);
static DEVICE_ATTR(dbaStaticCfgPrint,       S_IRUSR, cfg_show, cfg_store);
static DEVICE_ATTR(holdoverDebug,           S_IWUSR, cfg_show, cfg_store);
static DEVICE_ATTR(silenceMode,             S_IWUSR, cfg_show, cfg_store);
static DEVICE_ATTR(silenceSim,              S_IWUSR, cfg_show, cfg_store);
static DEVICE_ATTR(dgDebug,                 S_IWUSR, cfg_show, cfg_store);
static DEVICE_ATTR(dgCfg,                   S_IWUSR, cfg_show, cfg_store);
static DEVICE_ATTR(pattern,                 S_IWUSR, cfg_show, cfg_store);
static DEVICE_ATTR(printMask,               S_IRUSR | S_IWUSR, cfg_show, cfg_store);
static DEVICE_ATTR(pcsFrameLimits,          S_IWUSR | S_IRUSR, cfg_show, cfg_store);
static DEVICE_ATTR(rxDataFifoThresh,        S_IWUSR | S_IRUSR, cfg_show, cfg_store);
static DEVICE_ATTR(ddmTxPolarity,           S_IWUSR, cfg_show, cfg_store);
static DEVICE_ATTR(helpCfg,                 S_IRUSR, cfg_show, cfg_store);

static struct attribute *cfg_attrs[] = {
	&dev_attr_rxEn.attr,
	&dev_attr_txEn.attr,
	&dev_attr_ctrlTimerEn.attr,
	&dev_attr_ctrlTimerVal.attr,
	&dev_attr_ctrlTimerRandom.attr,
	&dev_attr_rprtTimerCfg.attr,
	&dev_attr_dbaHwParamsCfg.attr,
	&dev_attr_dbaSwIntervalCfg.attr,
	&dev_attr_dbaSwParamsCfg.attr,
	&dev_attr_dbaSwOptionCfg.attr,
	&dev_attr_dbaSwCfgPrint.attr,
	&dev_attr_dbaStaticCfgPrint.attr,
	&dev_attr_holdoverDebug.attr,
	&dev_attr_silenceMode.attr,
	&dev_attr_silenceSim.attr,
	&dev_attr_dgDebug.attr,
	&dev_attr_dgCfg.attr,
	&dev_attr_pattern.attr,
	&dev_attr_printMask.attr,
	&dev_attr_pcsFrameLimits.attr,
	&dev_attr_rxDataFifoThresh.attr,
	&dev_attr_ddmTxPolarity.attr,
	&dev_attr_helpCfg.attr,
	NULL
};

static struct attribute_group cfg_group =
{
  .name = "cfg",
  .attrs = cfg_attrs,
};

/******************************************************************************/
/* ========================================================================== */
/*                            ONU SYS FS                                      */
/* ========================================================================== */
/******************************************************************************/
int epon_sysfs_init(void)
{
  int err;
  struct device *pd;

  pd = bus_find_device_by_name(&platform_bus_type, NULL, "epon");
  if (!pd) {
  	platform_device_register_simple("epon", -1, NULL, 0);
  	pd = bus_find_device_by_name(&platform_bus_type, NULL, "epon");
  }

  if (!pd) {
  	printk(KERN_ERR"%s: cannot find epon device\n", __FUNCTION__);
  	pd = &platform_bus;
  }

  err = sysfs_create_group(&pd->kobj, &epon_info_group);
  if (err) {
  	printk(KERN_INFO "sysfs group failed %d\n", err);
  	goto out;
  }

  err = sysfs_create_group(&pd->kobj, &epon_pm_group);
  if (err) {
  	printk(KERN_INFO "sysfs group failed %d\n", err);
  	goto out;
  }

  err = sysfs_create_group(&pd->kobj, &cfg_group);
  if (err) {
  	printk(KERN_INFO "sysfs group failed %d\n", err);
  	goto out;
  }

  printk(KERN_INFO "= EPON Module SYS FS Init ended successfully =\n");
out:
  return err;
}

void epon_sysfs_delete(void)
{
  struct device *pd;

  pd = bus_find_device_by_name(&platform_bus_type, NULL, "epon");
  if (!pd)
  {
  	printk(KERN_ERR"%s: cannot find epon device\n", __FUNCTION__);
	return;
  }

  sysfs_remove_group(&pd->kobj, &epon_info_group);
  sysfs_remove_group(&pd->kobj, &epon_pm_group);
  sysfs_remove_group(&pd->kobj, &cfg_group);

  printk(KERN_INFO "= EPON Module SYS FS Remove ended successfully =\n");
}


/*******************************************************************************
**
**  onuEponUsrInterfaceCreate
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function create user interface - sys fs
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR
**
*******************************************************************************/
MV_STATUS onuEponUsrInterfaceCreate(void)
{
  if (epon_sysfs_init() != 0)
    return(MV_ERROR);

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponUsrInterfaceRelease
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function release user interface - sys fs
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR
**
*******************************************************************************/
MV_STATUS onuEponUsrInterfaceRelease(void)
{
  epon_sysfs_delete();

  return(MV_OK);
}





