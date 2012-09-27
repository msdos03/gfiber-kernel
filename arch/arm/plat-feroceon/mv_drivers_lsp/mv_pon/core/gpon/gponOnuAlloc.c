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
**  FILE        : gponOnuAlloc.c                                             **
**                                                                           **
**  DESCRIPTION : This file implements ONU GPON BW Allocation Table          **
**                                                                           **
*******************************************************************************
*                                                                             *
*  MODIFICATION HISTORY:                                                      *
*                                                                             *
*   29Oct06  Oren Ben Hayun   created                                         *
* =========================================================================== *
 ******************************************************************************/

/* Include Files
------------------------------------------------------------------------------*/
#include "gponOnuHeader.h"

/* Local Constant
------------------------------------------------------------------------------*/
#define __FILE_DESC__ "mv_pon/core/gpon/gponOnuAlloc.c"
/* Global Variables
------------------------------------------------------------------------------*/

/* Local Variables
------------------------------------------------------------------------------*/
MV_U32 onuIdleAllocTcont = PON_ONU_IDLE_ALLOC_TCONT;

/* Export Functions
------------------------------------------------------------------------------*/

/* Local Functions
------------------------------------------------------------------------------*/
MV_STATUS onuGponAllocIdTcontClearAll(void);
MV_STATUS onuGponAllocIdTcontSet(MV_U32 allocId, MV_U32 tcontNum);

/*
** ALLOC ID & T-CONT Handling
** ==========================
** Three tables to used to handle Alloc-Id and T-Cont management
**
** Software tables:
** ==> Alloc Id table           - contains all Alloc Id configured by the OLT
** ==> Idle Alloc Id table      - contains all configured Alloc not connected to HW
** ==> T-Cont table             - contains the XC between configured Alloc Id and T-Cont
**
** Hanrdware table
** ==> BW Map table             - contains the hardware configuration of the GMAC
**
** Rules
** =====
** 1. Default Alloc Id == ONU ID, is set to default T-Cont and can not be changed
** 2. Configure T-cont is only valid for a pre-defined Alloc Id which is not used
*/

MV_BOOL sendNotifyToMng = MV_FALSE;

 /******************************************************************************/
/* ========================================================================== */
/*                     SoC BW MAP Table Access Operations                     */
/* ========================================================================== */
/******************************************************************************/

/*******************************************************************************
**
**  onuGponAllocIdMacInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function init Rx Bw Map
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponAllocIdMacInit(void)
{
	MV_U32 iEntry;
	MV_STATUS status = MV_OK;

	for (iEntry = 0 ; iEntry < ONU_GPON_MAX_NUM_OF_T_CONTS ; iEntry++)
		status |= mvOnuGponMacRxBwMapSet(iEntry, 0, GPON_BW_MAP_IDLE_TCONT, MV_FALSE);

	return (status);
}

/*******************************************************************************
**
**  onuGponAllocIdMacAllocExistCheck
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function checks if allocId is configured to T-Cont and
**               returns T-Cont entry index in the BW MAP table
**
**  PARAMETERS:  MV_U32 allocId
**               MV_U32 *entry
**
**  OUTPUTS:     MV_U32 *entry
**
**  RETURNS:     MV_TRUE or MV_FALSE
**
*******************************************************************************/
MV_BOOL onuGponAllocIdMacAllocExistCheck(MV_U32 allocId, MV_U32 *entry)
{
	MV_U32  iEntry;
	MV_U32  macAllocId;
	MV_U32  tcontNum;
	MV_BOOL valid;
	MV_STATUS status;

	for (iEntry = 0 ; iEntry < ONU_GPON_MAX_NUM_OF_T_CONTS ; iEntry++) {
		status = mvOnuGponMacRxBwMapGet(iEntry,&macAllocId,&valid,&tcontNum);
		if ((status == MV_OK) && (macAllocId == allocId) && (valid == MV_TRUE)) {
			*entry = iEntry;
			return (MV_TRUE);
		}
	}

	return (MV_FALSE);
}

/*******************************************************************************
**
**  onuGponAllocIdMacAllocFreeEntryGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function returns free T-Cont entry in the BW MAP table
**
**  PARAMETERS:  MV_U32 *entry
**
**  OUTPUTS:     MV_U32 *entry
**
**  RETURNS:     MV_TRUE or MV_FALSE
**
*******************************************************************************/
MV_BOOL onuGponAllocIdMacAllocFreeEntryGet(MV_U32 *entry)
{
	MV_U32  iEntry;
	MV_U32  macAllocId;
	MV_U32  tcontNum;
	MV_BOOL valid;
	MV_STATUS status;

	for (iEntry = 0 ; iEntry < ONU_GPON_MAX_NUM_OF_T_CONTS ; iEntry++) {
		status = mvOnuGponMacRxBwMapGet(iEntry, &macAllocId, &valid, &tcontNum);
		if ((status == MV_OK) && (valid == MV_FALSE)) {
			*entry = iEntry;
			return (MV_TRUE);
		}
	}

	return (MV_FALSE);
}

/*******************************************************************************
**
**  onuGponAllocIdMacAdd
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function adds an entry to the BW MAP table
**
**  PARAMETERS:  MV_U32 allocId
**               MV_U32 tcontId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR
**
*******************************************************************************/
MV_STATUS onuGponAllocIdMacAdd(MV_U32 allocId, MV_U32 tcontId)
{
	MV_U32  entry;
	MV_BOOL exist;

	exist = onuGponAllocIdMacAllocFreeEntryGet(&entry);
	if (exist == MV_FALSE)
		return (MV_ERROR);

	return (mvOnuGponMacRxBwMapSet(entry, allocId, tcontId, MV_TRUE));
}

/*******************************************************************************
**
**  onuGponAllocIdMacConnect
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function connect Alloc-Id to  T-Cont
**
**  PARAMETERS:  MV_U32 allocId
**               MV_U32 tcontnum
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuGponAllocIdMacConnect(MV_U32 allocId, MV_U32 tcontNum)
{
	MV_STATUS status;
	MV_BOOL   exist;
	MV_U32    entry;
	MV_U32    newIdleTcontEntry;
	MV_U32    freeTcontEntry;
	MV_U32    entryAllocId;
	MV_U32    bwMapEntry;

	/* validate that requested alloc-Id is configured in HW and is valid */
	exist = onuGponAllocIdMacAllocExistCheck(allocId, &entry);
	if (exist != MV_TRUE) {
		mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
					"ERROR: (%s:%d) onuGponAllocIdMacConnect, alloc Id(%d) entry(%d)\n",
					__FILE_DESC__, __LINE__, allocId, entry);
		return (MV_ERROR);
	}

	if (tcontNum == onuIdleAllocTcont) {
		/* If the input T-Cont is the T-Cont used as an Idle T-Cont, all alloc-Id connected to the T-Cont
		** should be changed to another Idle T-Cont */

		/* Check for a free (idle) entry in the Tcont DB table */
		exist = onuGponDbBwTcontFreeGet(allocId, &freeTcontEntry);
		if (exist != MV_TRUE) {
#ifdef MV_GPON_DEBUG_PRINT
			mvPonPrint(PON_PRINT_DEBUG, PON_ALLOC_MODULE,
						"DEBUG: (%s:%d) onuGponAllocIdMacConnect, alloc Id(%d) freeTcontEntry(%d)\n",
						__FILE_DESC__, __LINE__, allocId, freeTcontEntry);
#endif /* MV_GPON_DEBUG_PRINT */
		} else {
			/* new idle Tcont entry allocated, update it globally */
			onuIdleAllocTcont = freeTcontEntry;

			/* scan all T-Cont / Alloc-Id in DB and update each HW Idle Alloc-Id to the new Idle T-Cont */
			for (newIdleTcontEntry = 0 ; newIdleTcontEntry < ONU_GPON_MAX_NUM_OF_T_CONTS ; newIdleTcontEntry++) {
				/* find the idle Alloc ID for this Tcont entry in DB */
				onuGponDbBwIdleAllocGet(newIdleTcontEntry, &entryAllocId);
				if (entryAllocId != PON_ONU_ALLOC_NOT_EXIST) {
					/* Check if this idle Alloc ID exists in the HW table */
					exist = onuGponAllocIdMacAllocExistCheck(entryAllocId, &bwMapEntry);
					if (exist == MV_TRUE) {
						/* re-assign this Alloc ID to the new idle Tcont in HW */
						status = mvOnuGponMacRxBwMapSet(bwMapEntry, entryAllocId, onuIdleAllocTcont, MV_TRUE);
						if (status != MV_OK) {
							mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
										"ERROR: (%s:%d) onuGponAllocIdMacConnect, Faild to re-ssign alloc Id(%d) "
										"to idle Tcont(%d)%d\n",
										__FILE_DESC__, __LINE__, entryAllocId, onuIdleAllocTcont);
						}
					} else {
						/* Gewalt! DB and HW are not in sync! */
						mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
									"ERROR: (%s:%d) onuGponAllocIdMacConnect, alloc Id(%d) is not in sync!\n",
									__FILE_DESC__, __LINE__, allocId);
					}
				} /* entryAllocId != PON_ONU_ALLOC_NOT_EXIST */
			}
		}
		 /* There is a free entry in Tcont table */
	} /* tcontNum == onuIdleAllocTcont */

	/* update input Alloc-Id and T-Cont in the HW */
	return (mvOnuGponMacRxBwMapSet(entry, allocId, tcontNum, MV_TRUE));
}

/*******************************************************************************
**
**  onuGponAllocIdMacReconnect
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function reconnect alloc-Id to idle tcont
**
**  PARAMETERS:  MV_U32 allocId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuGponAllocIdMacReconnect(MV_U32 allocId)
{
	MV_BOOL   exist;
	MV_STATUS status = MV_OK;
	MV_U32    entry;
	MV_U32    onuId;
	MV_U32    tcontNum;

	/* Get free entry */
	exist = onuGponAllocIdMacAllocExistCheck(allocId, &entry);
	if (exist != MV_TRUE)
		return (MV_ERROR);

	exist = onuGponDbBwTcontFreeGet(allocId, &tcontNum);
	if (exist != MV_TRUE)
		onuIdleAllocTcont = tcontNum;

	/* Check if default Alloc Id */
	onuId = onuGponDbOnuIdGet();
	if (allocId != onuId) {
		/* Set Alloc Id to T-Cont in the GMAC hardware */
		status = mvOnuGponMacRxBwMapSet(entry, allocId, onuIdleAllocTcont, MV_TRUE);
	}

	return (status);
}

/*******************************************************************************
**
**  onuGponAllocIdMacDisconnect
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function disconnect alloc-Id from tcont
**
**  PARAMETERS:  MV_U32 allocId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuGponAllocIdMacDisconnect(MV_U32 allocId)
{
	MV_U32  onuId;
	MV_U32  entry;
	MV_BOOL exist;
	MV_STATUS status = MV_OK;

	/* Check if already exist */
	exist = onuGponAllocIdMacAllocExistCheck(allocId, &entry);
	if (exist == MV_FALSE)
		return (MV_ERROR);

	/* Check if default Alloc Id */
	onuId = onuGponDbOnuIdGet();
	if (allocId != onuId)
		status = mvOnuGponMacRxBwMapSet(entry, 0, 0, MV_FALSE);

	return (status);
}

/******************************************************************************/
/* ========================================================================== */
/*                                    APIs                                    */
/* ========================================================================== */
/******************************************************************************/

/*******************************************************************************
**
**  onuGponAllocIdInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function init Tconts Bitmap
**
**  PARAMETERS:  MV_U32 existTcontsBitmap
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponAllocIdInit(MV_U32 existTcontsBitmap)
{
	MV_U32  iEntry;

#ifdef MV_GPON_DEBUG_PRINT
	mvPonPrint(PON_PRINT_DEBUG, PON_ALLOC_MODULE,
				"DEBUG: (%s:%d) Tconts Bitmap(0x%x)\n", __FILE_DESC__, __LINE__, existTcontsBitmap);
#endif /* MV_GPON_DEBUG_PRINT */

	for (iEntry = 0 ; iEntry < ONU_GPON_MAX_NUM_OF_T_CONTS ; iEntry++) {
		/* Check if T-CONT Exist */
		if ((existTcontsBitmap >> iEntry) & 0x01)
			onuGponDbBwTcontSet(iEntry, MV_TRUE, PON_ONU_ALLOC_NOT_EXIST, MV_FALSE);
	}

	return (MV_OK);
}

/*******************************************************************************
**
**  onuGponAllocIdAssign
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function Assign Alloc ID
**
**  PARAMETERS:  MV_U32 allocId
**               MV_U32 notifyFlag
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuGponAllocIdAssign(MV_U32 allocId, MV_U32 notifyFlag)
{
	MV_STATUS status;
	MV_U32    entry;
	MV_U32    idleEntry;
	MV_U32    onuId;
	MV_BOOL   allocIdExist;

#ifdef MV_GPON_DEBUG_PRINT
	mvPonPrint(PON_PRINT_DEBUG, PON_ALLOC_MODULE,
				"DEBUG: (%s:%d) Alloc ID Assign, alloc Id(%d)\n", __FILE_DESC__, __LINE__, allocId);
#endif /* MV_GPON_DEBUG_PRINT */

	/* check if Alloc-Id exist in the alloc Id DB table */
	allocIdExist = onuGponDbBwAllocExist(allocId);
	if (allocIdExist == MV_TRUE)
		return (MV_OK);

	/* set the Alloc-Id exist to the database */
	status = onuGponDbBwAllocInsert(allocId, &entry);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
				"ERROR: (%s:%d) onuGponDbBwAllocInsert, alloc Id(%d)\n",
					__FILE_DESC__, __LINE__, allocId);
		return (status);
	}

	onuId = onuGponDbOnuIdGet();

	/* Default AllocId */
	/* =============== */
	/* configured in the T-Cont table and BW MAP */
	if (allocId == onuId) {
		/* Set default Alloc Id to T-Cont table */
		status = onuGponDbBwTcontAlloc(PON_ONU_DEFAULT_ALLOC_TCONT, allocId);
		if (status != MV_OK) {
			mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
						"ERROR: (%s:%d) tcont to alloc set failed, tcont (%d) alloc Id(%d)\n",
						__FILE_DESC__, __LINE__, PON_ONU_DEFAULT_ALLOC_TCONT, allocId);
			return (MV_ERROR);
		}

		sendNotifyToMng = MV_TRUE;

	} else {

		/* Data AllocId  */
		/* ============= */
		/* Configured in the Idle Alloc table and BW MAP */
		/* set the Alloc-Id exist to the database */
		status = onuGponDbBwIdleAllocInsert(allocId, &idleEntry);
		if (status != MV_OK) {
			mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
						"ERROR: (%s:%d) onuGponDbBwIdleAllocInsert, alloc Id(%d))\n",
						__FILE_DESC__, __LINE__, allocId);
			return (status);
		}

		/* set the Alloc-Id exist to the BW table in the MAC (hardware) */
		status = onuGponAllocIdMacAdd(allocId, onuIdleAllocTcont);
		if (status != MV_OK) {
			mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
						"ERROR: (%s:%d) onuGponAllocIdMacAdd, alloc Id(%d)\n",
						__FILE_DESC__, __LINE__, allocId);
			return (status);
		}

		sendNotifyToMng = MV_TRUE;
	}

	if ((sendNotifyToMng == MV_TRUE) && (notifyFlag != 0)) {
		/* Send Status Notification to upper layer */
		onuGponSrvcStatusNotify(GPON_ONU_STATUS_RANGED);
	}

	return (MV_OK);
}

/*******************************************************************************
**
**  onuGponAllocIdDeAssign
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function De-Assign Alloc ID
**
**  PARAMETERS:  MV_U32 allocId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuGponAllocIdDeAssign(MV_U32 allocId)
{
	MV_STATUS status;
	MV_U32    entry;
	MV_U32    tcontNum;
	MV_BOOL   allocIdExist;
	MV_BOOL   valid;
#ifdef MV_GPON_DEBUG_PRINT
	mvPonPrint(PON_PRINT_DEBUG, PON_ALLOC_MODULE,
				"DEBUG: (%s:%d) De-Assign Alloc ID, alloc Id(%d)\n", __FILE_DESC__, __LINE__, allocId);
#endif /* MV_GPON_DEBUG_PRINT */

	/* Check if Alloc-Id exist in the database alloc Id, if not exist */
	allocIdExist = onuGponDbBwAllocExist(allocId);
	if (allocIdExist == MV_FALSE)
		return (MV_OK);

	/* Delete the Alloc Id from the BW MAP table (if it is not the default T-Cont) */
	status = onuGponAllocIdMacDisconnect(allocId);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
				"ERROR: (%s:%d) onuGponAllocIdMacDisconnect, alloc Id(%d)\n",
					__FILE_DESC__, __LINE__, allocId);
		return (status);
	}

	/* check if Alloc-Id is connected Data T-Cont */
	status = onuGponDbBwTcontConnectCheck(allocId, &tcontNum, &valid);
	if ((status == MV_OK) && (valid == MV_TRUE)) {
		/* Delete Alloc ID from the Alloc Id / T-Cont table in the database */
		onuGponDbBwTcontClear(tcontNum);
	}

	/* check if Alloc-Id is Idle Alloc-Id */
	if (onuGponDbBwIdleAllocExist(allocId, &entry) == MV_TRUE) {
		/* Delete Alloc ID from the Idle Alloc Id table */
		status = onuGponDbBwIdleAllocRemove(allocId, &entry);
		if (status != MV_OK) {
			mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
						"ERROR: (%s:%d) onuGponDbBwIdleAllocRemove, alloc Id(%d) entry(%d)\n",
						__FILE_DESC__, __LINE__, allocId, entry);
			return (status);
		}
	}

	/* Delete Alloc Id from the Alloc Id table in the database */
	status = onuGponDbBwAllocRemove(allocId, &entry);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
				"ERROR: (%s:%d) onuGponDbBwAllocRemove, alloc Id(%d) entry(%d)\n",
					__FILE_DESC__, __LINE__, allocId, entry);
		return (status);
	}

	return (MV_OK);
}

/*******************************************************************************
**
**  onuGponAllocIdDeAssignAll
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function De-Assign All Alloc IDs
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponAllocIdDeAssignAll(void)
{
#ifdef MV_GPON_DEBUG_PRINT
  mvPonPrint(PON_PRINT_DEBUG, PON_ALLOC_MODULE,
             "DEBUG: (%s:%d) De-Assign All Alloc IDs and T-Conts\n", __FILE_DESC__, __LINE__);
#endif /* MV_GPON_DEBUG_PRINT */

  onuGponDbBwAllocInit();
  onuGponAllocIdMacInit();
  onuGponAllocIdInit(0x00FF);

  sendNotifyToMng = MV_FALSE;

  return (MV_OK);
}

/*******************************************************************************
**
**  onuGponAllocIdTcontSet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function Set Alloc ID to T-CONT
**
**  PARAMETERS:  MV_U32 allocId
**               MV_U32 tcontnum
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuGponAllocIdTcontSet(MV_U32 allocId, MV_U32 tcontNum)
{
	MV_STATUS status;
	MV_U32    otherTcontNum;
	MV_U32    dbAllocId;
	MV_U32    entry;
	MV_BOOL   valid;
	MV_BOOL   exist;

#ifdef MV_GPON_DEBUG_PRINT
	mvPonPrint(PON_PRINT_DEBUG, PON_ALLOC_MODULE,
				"DEBUG: (%s:%d) Set Alloc ID to T-CONT, alloc Id(%d) tcont(%d)\n",
				__FILE_DESC__, __LINE__, allocId, tcontNum);
#endif /* MV_GPON_DEBUG_PRINT */

	/* Check if Alloc Id was assigned by the OLT */
	exist = onuGponDbBwAllocExist(allocId);
	if (exist == MV_FALSE) {
		mvPonPrint(PON_PRINT_DEBUG, PON_ALLOC_MODULE,
				"ERROR: (%s:%d) Alloc ID Not Exist, alloc Id(%d)\n",
				__FILE_DESC__, __LINE__, allocId, tcontNum);
		return (MV_OK);
	}

	/* Check if T-CONT Exist - all 8 tconts are set to exist during onuGponAllocIdInit function */
	status = onuGponDbBwTcontExist(tcontNum,&exist);
	if ((status != MV_OK) || (exist == MV_FALSE)) {
		mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
				"ERROR: (%s:%d) Tcont Not Exist, alloc Id(%d) tcont(%d)\n",
				__FILE_DESC__, __LINE__, allocId, tcontNum);
		return (MV_ERROR);
	}

	/* Check that Alloc Id is not exist on other T-CONT */
	status = onuGponDbBwTcontConnectCheck(allocId, &otherTcontNum, &valid);
	if ((status == MV_OK) && (valid == MV_TRUE)) {
		if (otherTcontNum == tcontNum) {

			mvPonPrint(PON_PRINT_DEBUG, PON_ALLOC_MODULE,
						"ERROR: (%s:%d) Alloc Id/T-CONT Already Configured, alloc Id(%d) tcont(%d) otherTcontNum(%d)\n",
						__FILE_DESC__, __LINE__, allocId, tcontNum, otherTcontNum);
			return (MV_OK);

		} else {

			mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
						"ERROR: (%s:%d) Alloc Id Already Exist on T-CONT, alloc Id(%d) tcont(%d) otherTcontNum(%d)\n",
						__FILE_DESC__, __LINE__, allocId, tcontNum, otherTcontNum);
			return (MV_ERROR);
		}
	}

	/* Check that the T-CONT is valid */
	onuGponDbBwTcontGet(tcontNum, &exist, &dbAllocId, &valid);
	if (valid == MV_TRUE) {
		if (dbAllocId != PON_ONU_ALLOC_NOT_EXIST) {
			mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
						"ERROR: (%s:%d) Alloc Id/T-CONT Already Configured, alloc Id(%d) tcont(%d)\n",
						__FILE_DESC__, __LINE__, allocId, tcontNum);
			return (MV_OK);
		}
	}

	/* configure Alloc Id / T-Cont table */
	status = onuGponDbBwTcontAlloc(tcontNum, allocId);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
				"ERROR: (%s:%d) onuGponDbBwTcontAlloc, alloc Id(%d) tcont(%d)\n",
				__FILE_DESC__, __LINE__, allocId, tcontNum);
		return (MV_ERROR);
	}

	/* remove Alloc Id from Idle Alloc table */
	status = onuGponDbBwIdleAllocRemove(allocId, &entry);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
				"ERROR: (%s:%d) onuGponDbBwIdleAllocRemove, alloc Id(%d) entry(%d) removed from Idle table\n",
				__FILE_DESC__, __LINE__, allocId, entry);
		return (MV_ERROR);
	}

	/* set Alloc Id to T-Cont in the GMAC hardware */
	status = onuGponAllocIdMacConnect(allocId, tcontNum);
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
				"ERROR: (%s:%d) onuGponAllocIdMacConnect, alloc Id(%d) tcont(%d)\n",
				__FILE_DESC__, __LINE__, allocId, tcontNum);
		return (MV_ERROR);
	}

	return (MV_OK);
}

/*******************************************************************************
**
**  onuGponAllocIdTcontClear
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function Clear Alloc ID from T-CONT
**
**  PARAMETERS:  MV_U32 tcontnum
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuGponAllocIdTcontClear(MV_U32 tcontNum)
{
	MV_STATUS status;
	MV_U32    allocId;
	MV_U32    onuId;
	MV_U32    idleEntry;
	MV_BOOL   valid;
	MV_BOOL   exist;

#ifdef MV_GPON_DEBUG_PRINT
	mvPonPrint(PON_PRINT_DEBUG, PON_ALLOC_MODULE,
				"DEBUG: (%s:%d) Clear Alloc ID from T-CONT: T-CONT(%d)\n",
				__FILE_DESC__, __LINE__, tcontNum);
#endif /* MV_GPON_DEBUG_PRINT */

	/* Check if T-CONT Exist */
	status = onuGponDbBwTcontExist(tcontNum,&exist);
	if ((status != MV_OK) || (exist == MV_FALSE)) {
		mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
				"ERROR: (%s:%d) T-CONT Not Exist, tcont(%d)\n",
				__FILE_DESC__, __LINE__, tcontNum);
		return (MV_ERROR);
	}

	/* Get tcont params from the database */
	onuGponDbBwTcontGet(tcontNum, &exist, &allocId, &valid);
	if (valid == MV_FALSE)
		return (MV_OK);

	/* Check if Alloc Id Exist */
	exist = onuGponDbBwAllocExist(allocId);
	if (exist == MV_TRUE) {
		/* Clear T-Cont in hardware in case T-Cont is not connected to default Alloc-Id (== ONU ID) */
		status = onuGponAllocIdMacReconnect(allocId);
		if (status != MV_OK) {
			mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
						"ERROR: (%s:%d) onuGponAllocIdMacReconnect, alloc Id(%d)\n",
						__FILE_DESC__, __LINE__, allocId);
			return (MV_ERROR);
		}
	}

	/* Clear T-Cont in the database */
	onuId = onuGponDbOnuIdGet();
	if (allocId != onuId) {
		onuGponDbBwTcontClear(tcontNum);

		/* set the Alloc-Id exist to the idle alloc-Id */
		status = onuGponDbBwIdleAllocInsert(allocId, &idleEntry);
		if (status != MV_OK) {
			mvPonPrint(PON_PRINT_ERROR, PON_ALLOC_MODULE,
						"ERROR: (%s:%d) onuGponAllocIdTcontClear, idle alloc Id(%d) entry(%d)\n",
						__FILE_DESC__, __LINE__, allocId, idleEntry);
			return (status);
		}
	}

	return (MV_OK);
}

/*******************************************************************************
**
**  onuGponAllocIdTcontClearAll
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function Clear All Alloc-Ids from T-TCONTs
**
**  PARAMETERS:  MV_U32 allocId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponAllocIdTcontClearAll(void)
{
	MV_U32  iEntry;
	MV_BOOL exist;
	MV_STATUS status;

#ifdef MV_GPON_DEBUG_PRINT
	mvPonPrint(PON_PRINT_DEBUG, PON_ALLOC_MODULE,
				"DEBUG: (%s:%d) Clear All Alloc-Ids from T-TCONTs\n", __FILE_DESC__, __LINE__);
#endif /* MV_GPON_DEBUG_PRINT */

	for (iEntry = 0 ; iEntry < ONU_GPON_MAX_NUM_OF_T_CONTS ; iEntry++) {
		/* Check if T-CONT Exist */
		status = onuGponDbBwTcontExist(iEntry,&exist);
		if ((status == MV_OK) && (exist == MV_TRUE))
			onuGponAllocIdTcontClear(iEntry);
	}

	return (MV_OK);
}

/*******************************************************************************
**
**  onuGponIdleAllocIdTcontGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function return Idle Alloc-Id
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_U32 allocId
**
*******************************************************************************/
MV_U32 onuGponIdleAllocIdTcontGet(void)
{
  return (onuIdleAllocTcont);
}

