/** \file sys_service.c
 * sys_service.c
 *
 *  This file implements the required system services for the API-II using a
 * Linux OS running on the UVB.  The user should replace the functions provided
 * here with the equivalent based on their OS and hardware.
 *
 * Copyright (c) 2008, Zarlink Semiconductor, Inc.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>     /* exit() */
#include <stdio.h>
#include "vp_api_cfg.h"
#include "vp_api_types.h"
#include "sys_service.h"
#include "vp_api_common.h"
#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN
#define GET_DEV_STATUS(deviceId)	vpapi_dev_status[deviceId]
#define GET_LINE_STATUS(lineId)		vpapi_line_status[lineId]
#define MAX_EVENT_QUEUE_SIZE		256

/* VE880 */
#if defined(ZARLINK_SLIC_VE880)
#define MAX_DEVICES			2
#define MAX_LINES			4

/* VE792 */
#elif defined(ZARLINK_SLIC_VE792)
#define MAX_DEVICES		4
#define MAX_LINES		32
#endif

static const key_t key = 0xADEF;
static const int flagMakeSem = S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | IPC_CREAT | IPC_EXCL;
static const int flagGetSem =  S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH;
static const int NUM_SEMS = 1; /*VP_NUM_DEVICES * VP_NUM_CRITICAL_SEC_TYPES;*/
static int critDepth = 0;

/*
 * VPD socket file descriptor: This is opened only once, the first time it is
 * needed.  It is closed automatically by the OS when the process exits.  This
 * policy is to avoid the overhead of reopening the socket each time a VpSys...()
 * function is called.
 */
static int vpd_fd = -1;

/* Enumurators */
typedef struct {
	unsigned char valid;		/* valid event */
	VpEventType vp_event;
} vpapi_event;

timer_t 			timerid;
pthread_mutex_t		vp_lock;
unsigned int 		event_count;

vpapi_event event_queue[MAX_EVENT_QUEUE_SIZE];
volatile unsigned int next_event = 0, curr_event = 0;

extern VpDevCtxType pDevCtx[MAX_DEVICES];
extern unsigned char vpapi_dev_status[MAX_DEVICES];
extern unsigned char vpapi_line_status[MAX_LINES];


/*
 * VpSysWait() function implementation is needed only for CSLAC devices
 * (880, 790). For other devices this function could be commented.
 */
void
VpSysWait(
    uint8 time)  /* Time specified in increments of 125uS (e.g. 4 = 500uS) */
{
	usleep(125 * time);
}
void VpSysSemaphoreInit(int semaphoreId)
{
    int i;
    int rc;
    unsigned short initialValues[NUM_SEMS];
    union semun semArg;
    pid_t pid = 0;
#ifdef SYS_SERVICE_DEBUG
    pid = getpid();
#endif
    for (i=0; i < NUM_SEMS; i++) {
        initialValues[i] = 1;
    }
    semArg.array = initialValues;

    dprintf("%u:initing semaphore key=0x%X, id=0x%X...",pid, key, semaphoreId);
    if ((rc = semctl(semaphoreId, 0, SETALL, semArg)) == -1) {
#ifdef SYS_SERVICE_DEBUG
        perror("semctl SETALL");
#endif
        exit(1);
    }
}

/*
 * VpSysEnterCritical(), VpSysExitCritical():
 *
 *  These functions allow for disabling interrupts while executing nonreentrant
 * portions of VoicePath API code. Note that the following implementations of
 * enter/exit critical section functions are simple implementations. These
 * functions could be expanded (if required) to handle different critical
 * section types differently.
 *
 * Params:
 *  VpDeviceIdType deviceId: Device Id (chip select ID)
 *  VpCriticalSecType: Critical section type
 *
 * Return:
 *  Number of critical sections currently entered for the device.
 */
uint8
VpSysEnterCritical(
    VpDeviceIdType deviceId,
    VpCriticalSecType criticalSecType)
{
   int rc;
    int semaphoreId;
    struct sembuf semOp[1];
    pid_t pid = 0;
#ifdef SYS_SERVICE_DEBUG
    pid = getpid();
#endif
    /* semOp[0].sem_num = deviceId*VP_NUM_CRITICAL_SEC_TYPES + criticalSecType;    */
    semOp[0].sem_num = 0;
    semOp[0].sem_op = -1;
    semOp[0].sem_flg = SEM_UNDO;

    critDepth++;
    if (critDepth == 1) {
        if ((semaphoreId = semget(key, NUM_SEMS, flagMakeSem)) >= 0) {
             /* we just created the semaphore and it needs to be inited */
            VpSysSemaphoreInit(semaphoreId);
            dprintf("%u:Made new semaphore %d (hopefully sem_val=1)\n", pid, semaphoreId);
         } else {
             /* semaphore already exists, we just need to get an id */
             if ((semaphoreId = semget(key, NUM_SEMS, flagGetSem)) < 0) {
                /* something has gone sour */
#ifdef SYS_SERVICE_DEBUG
                perror("semget flagGetSem");
#endif
                exit(1);
            }
            /* dprintf("%u:Got existing semaphore %d\n",pid, semaphoreId); */
        }

        /* dprintf("%u:?v,d=%d\n",pid);    */
        if ((rc = semop(semaphoreId, semOp, 1)) < 0) {
#ifdef SYS_SERVICE_DEBUG
            perror("semop -1");
#endif
            exit(1);
        }
        if (rc)    {
            dprintf("semaphore error with deviceId=0x%X, criticalSecType=0x%X\n",
                deviceId, criticalSecType);
                return -1;
        }
    }
    dprintf("%u:v,d=%d\n",pid, critDepth);

    return critDepth;
} /* VpSysEnterCritical() */

uint8
VpSysExitCritical(
    VpDeviceIdType deviceId,
    VpCriticalSecType criticalSecType)
{
 int rc;
    int semaphoreId;
    struct sembuf semOp[1];
    pid_t pid = 0;
#ifdef SYS_SERVICE_DEBUG
    pid = getpid();
#endif
    /* semOp[0].sem_num = deviceId*VP_NUM_CRITICAL_SEC_TYPES + criticalSecType;    */
    semOp[0].sem_num = 0;
    semOp[0].sem_op = 1;
    semOp[0].sem_flg = SEM_UNDO;

    critDepth--;
    if (critDepth == 0) {
        /* we can free the semaphore now */
        if ((semaphoreId = semget(key, NUM_SEMS, flagGetSem)) < 0) {
#ifdef SYS_SERVICE_DEBUG
            perror("semget");
#endif
            exit(1);
        }
        /* dprintf("%u:Got existing semaphore %d\n",pid, semaphoreId); */
        dprintf("%u:^,d=%d\n",pid,critDepth);
        if ((rc = semop(semaphoreId, semOp, 1)) == -1) {
#ifdef SYS_SERVICE_DEBUG
            perror("semop +1");
#endif
            exit(1);
        }

        if (rc)    {
            dprintf("semaphore error with deviceId=0x%X, criticalSecType=0x%X",
                deviceId, criticalSecType);
        }
    } else {
        dprintf("%u:^,d=%d\n",pid,critDepth);
    }
    /*dprintf("%u:VpSysExitCritical finished with critDepth=%d.\n",pid, critDepth);*/

    return critDepth;
} /* VpSysExitCritical() */

/**
 * VpSysDisableInt(), VpSysEnableInt(), and VpSysTestInt()
 *
 *  These functions are used by the CSLAC device family for interrupt driven
 * polling modes. These are called by the API to detect when a non-masked
 * device status has changed.  If using SIMPLE_POLL mode, these functions do not
 * require implementation.
 *
 * Preconditions:
 *  None. The implementation of these functions is architecture dependent.
 *
 * Postconditions:
 *  VpSysDisableInt() - The interrupt associated with the deviceId passed is
 * disabled.
 *
 * VpSysEnableInt() - The interrupt associated with the deviceId passed is
 * enabled.
 *
 * VpSysTestInt() - The return value is TRUE if an interrupt occurred, otherwise
 * return FALSE.
 *
 * These functions are needed only for CSLAC devices
 * (880, 790). For other devices these functions could be commented.
 *
 */
void
VpSysDisableInt(
    VpDeviceIdType deviceId)
{
}
void
VpSysEnableInt(
    VpDeviceIdType deviceId)
{
}
bool
VpSysTestInt(
    VpDeviceIdType deviceId)
{
    return FALSE;
}
/**
 * VpSysDtmfDetEnable(), VpSysDtmfDetDisable()
 *
 *  These functions are used by the CSLAC device family for devices that do not
 * internally detect DTMF. It is used for Caller ID type-II and is provided to
 * enable external DTMF detection.
 *
 * Preconditions:
 *  None. The implementation of these functions is application dependent.
 *
 * Postconditions:
 *  VpSysDtmfDetEnable() - The device/channel resource for DTMF detection is
 * enabled.
 *
 *  VpSysDtmfDetDisable() - The device/channel resource for DTMF detection is
 * disabled.
 *
 * These functions are needed only for CSLAC devices
 * (880, 790). For other devices these functions could be commented.
 *
 */
void
VpSysDtmfDetEnable(
    VpDeviceIdType deviceId,
    uint8 channelId)
{
}
void
VpSysDtmfDetDisable(
    VpDeviceIdType deviceId,
    uint8 channelId)
{
}
/*
 * The following functions VpSysTestHeapAcquire(),  VpSysTestHeapRelease()
 * VpSysPcmCollectAndProcess() and are needed only for CSLAC devices
 * (880). For other devices these functions could be commented. Please see
 * the LineTest API documentation for function requirements.
 */
void *
VpSysTestHeapAcquire(
    uint8 *pHeapId)
{
    return VP_NULL;
} /* VpSysTestHeapAcquire() */
bool
VpSysTestHeapRelease(
    uint8 heapId)
{
    return TRUE;
} /* VpSysTestHeapRelease() */
void
VpSysPcmCollectAndProcess(
    void *pLineCtx,
    VpDeviceIdType deviceId,
    uint8 channelId,
    uint8 startTimeslot,
    uint16 operationTime,
    uint16 settlingTime,
    uint16 operationMask)
{
} /* VpSysPcmCollectAndProcess() */

#ifdef ENABLE_DBG_TAG
int
VpSysDebugPrintf(
    const char *format, ...)
{
    va_list ap;
    static char buf[1024];
    static uint16 indexBuf;
    static bool beginLine = TRUE;
    uint16 bufLen;
    int retval;

    if (beginLine == TRUE) {
		printf("<DBG> ");
		beginLine = FALSE;
    }
    /* print in a string to check if there's a \n */
    va_start(ap, format);
    retval = vsprintf(&buf[indexBuf], format, ap);
    va_end(ap);

    bufLen = strlen(&buf[indexBuf]);
    if (buf[indexBuf + bufLen - 1] == '\n') {
		buf[indexBuf + bufLen - 1] = ' ';
		printf("%s</DBG>\n", buf);
		indexBuf = 0;
		beginLine = TRUE;
    } else {
		indexBuf += bufLen;

		/* Just in case of a buffer overflow, not suppose to append */
		if (indexBuf > 800) {
			retval = printf("%s</DBG>\n", buf);
			indexBuf = 0;
			beginLine = TRUE;
		}
      }
    return retval;
}
#endif /* ENABLE_DBG_TAG */

void sys_service_init()
{
	pthread_mutex_init(&vp_lock, NULL);
	/* Clear event queue */
	memset(event_queue, 0, (MAX_EVENT_QUEUE_SIZE * sizeof(vpapi_event)));
}

static void sys_service_tick_handler()
{
	unsigned char deviceId;
	unsigned long flags;
	vpapi_event *pEvent;
#if !defined(ZARLINK_SLIC_VE792)
	bool eventStatus;
#endif
	sigset_t mask;


	pthread_mutex_lock(&vp_lock);

	for(deviceId = 0; deviceId < MAX_DEVICES; deviceId++) {

		

		if(GET_DEV_STATUS(deviceId) == 0)
			continue;

		/* Check for free resources */
		if(event_count >= MAX_EVENT_QUEUE_SIZE)
			goto timer_exit;



#if !defined(ZARLINK_SLIC_VE792)
		if(VP_STATUS_SUCCESS == VpApiTick(&pDevCtx[deviceId], &eventStatus)) {
			if(eventStatus == TRUE) {
#endif
				pEvent = &event_queue[next_event];

				while(VpGetEvent(&pDevCtx[deviceId], &pEvent->vp_event) == TRUE) {

					if(pEvent->vp_event.status != VP_STATUS_SUCCESS) {
						printf("%s: bad status(%d)\n", __func__, pEvent->vp_event.status);
						break;
					}

					if(pEvent->vp_event.eventId == 0)  {
						printf("%s: warning, empty event\n", __func__);
						break;
					}
					next_event++;
					if(next_event == MAX_EVENT_QUEUE_SIZE) {
						next_event = 0;
					}

					event_count++;
					if(pEvent->valid == 0) {
						pEvent->valid = 1;
					}
					else {
						printf("%s: error, event(%u) was overrided\n", __func__, next_event);
						break;
					}

					pEvent = &event_queue[next_event];
				}
#if !defined(ZARLINK_SLIC_VE792)
			}
		}
#endif
	}

	pthread_mutex_unlock(&vp_lock);

	return;

timer_exit:
	return;
}

bool sys_service_get_event(VpDeviceIdType dev_id, VpEventType *event_p)
{
	bool		newEvent;

	pthread_mutex_lock(&vp_lock);

	if(event_count == 0) {
		newEvent = FALSE;
	}
	else {
		memcpy(event_p, &event_queue[curr_event].vp_event, sizeof(VpEventType));
		event_queue[curr_event].valid = 0;
		newEvent = TRUE;
		event_count--;
		curr_event++;
		if(curr_event == MAX_EVENT_QUEUE_SIZE)
			curr_event = 0;
	}

	pthread_mutex_unlock(&vp_lock);

	return newEvent;
}

void *sys_service_timer_function (void *arg)
{
	while(1)
	{
		/* Do useful work */
		sys_service_tick_handler();
		/* Wait 10ms */
		usleep(10000);
	}
	return NULL;
}

