/*
 * sys_service.h
 *
 *  This file is the header for all standard types used in the API code.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 */

#ifndef SYS_SERVICE_H
#define SYS_SERVICE_H

#include "vp_api_types.h"
#include <stdio.h>

#define MAX_SLIC_RDWR_BUFF_SIZE		128
/*-----------------8/10/2005 10:31AM----------------
 * DEFINE THE FOLLOWING
 * --------------------------------------------------*/
#define VP_NUM_DEVICES 1
/*-----------------8/10/2005 10:31AM----------------
 * END DEFINE THE FOLLOWING
 * --------------------------------------------------*/

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
	/* union semun is defined by including <sys/sem.h> */
#else
	/* according to X/OPEN we have to define it ourselves */
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
    /* these fields unique to linux */
    struct seminfo *__buf;  /* buffer for IPC_INFO */
    void *__pad;
	/* end unique to linux */
};
#endif

typedef struct vpapi_init_device_params {
	unsigned short dev_size;
	unsigned short ac_size;
	unsigned short dc_size;
	unsigned short ring_size;
	unsigned short fxo_ac_size;
	unsigned short fxo_cfg_size;
} vpapi_init_device_params_t;

/* Critical section types */
typedef enum {
	VP_MPI_CRITICAL_SEC, 	/* MPI access critical code section */
	VP_HBI_CRITICAL_SEC, 	/* HBI access critical code section */
	VP_CODE_CRITICAL_SEC, 	/* Critical code section */
	VP_NUM_CRITICAL_SEC_TYPES, /* The number of critical section types */
    VP_CRITICAL_SEC_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req. */
} VpCriticalSecType;
EXTERN uint8
VpSysEnterCritical(
    VpDeviceIdType deviceId,
    VpCriticalSecType criticalSecType);
EXTERN uint8
VpSysExitCritical(
    VpDeviceIdType deviceId,
    VpCriticalSecType criticalSecType);
EXTERN void
VpSysWait(
    uint8 time);
EXTERN void
VpSysDisableInt(
    VpDeviceIdType deviceId);
EXTERN void
VpSysEnableInt(
    VpDeviceIdType deviceId);
EXTERN bool
VpSysTestInt(
    VpDeviceIdType deviceId);
EXTERN void
VpSysDtmfDetEnable(
    VpDeviceIdType deviceId,
    uint8 channelId);
EXTERN void
VpSysDtmfDetDisable(
    VpDeviceIdType deviceId,
    uint8 channelId);

EXTERN void *
VpSysTestHeapAcquire(
    uint8 *pHeapId);
EXTERN bool
VpSysTestHeapRelease(
    uint8 heapId);
EXTERN void
VpSysPcmCollectAndProcess(
    void *pLineCtx,
    VpDeviceIdType deviceId,
    uint8 channelId,
    uint8 startTimeslot,
    uint16 operationTime,
    uint16 settlingTime,
    uint16 operationMask);

/* Enable the XML tag <DBG> debug output </DBG>  */
/* #define ENABLE_DBG_TAG */

#ifdef ENABLE_DBG_TAG
    EXTERN int
    VpSysDebugPrintf(
		const char *format, ...);
#else
    #define VpSysDebugPrintf printf
#endif /* ENABLE_DBG_TAG */

#endif /* SYS_SERVICE_H */

