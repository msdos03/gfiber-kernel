/** \file vp_api_timer.h
 * vp_api_timer.h
 *
 * Header file for all timer typedefs used in API-II (internal).
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 8481 $
 * $LastChangedDate: 2011-04-07 09:43:23 -0500 (Thu, 07 Apr 2011) $
 */

#ifndef VP_API_TIMER_H
#define VP_API_TIMER_H

/* IMT average for Voltage to Frequency (FXO) smoothing */
#define VP_IMT_AVG_LENGTH 2  /* FXO variable, but need this to compile */

#define VP_ACTIVATE_TIMER       0x8000
#define VP_TIMER_TIME_MASK      0x7FFF

/*
 * The timer types are generally evaluated in an incremental loop, so must use
 * only sequential values.
 */
/**< Line Timer Definitions */

/**< Debounce for known CID state changes */
#define VP_LINE_CID_DEBOUNCE            0

/**< Timer for DTMF generation in CID */
#define VP_LINE_TIMER_CID_DTMF          (VP_LINE_CID_DEBOUNCE + 1)

/**< The fault timer */
#define VP_LINE_TIMER_FAULT             (VP_LINE_TIMER_CID_DTMF + 1)

/**< Line Debounce specifically for Ring Exit */
#define VP_LINE_RING_EXIT_DEBOUNCE      (VP_LINE_TIMER_FAULT + 1)

/**< Hook Mask on Polarity Reversals */
#define VP_LINE_POLREV_DEBOUNCE         (VP_LINE_RING_EXIT_DEBOUNCE + 1)

/**< Line Debounce for Disconnect Recovery */
#define VP_LINE_DISCONNECT_EXIT         (VP_LINE_POLREV_DEBOUNCE + 1)

/**< Timer for Recovery from Tip-Open */
#define VP_LINE_GND_START_TIMER         (VP_LINE_DISCONNECT_EXIT + 1)

/**< Timer for CalLine operations */
#define VP_LINE_CAL_LINE_TIMER          (VP_LINE_GND_START_TIMER + 1)

/**< Timer to prevent phone "ping" */
#define VP_LINE_PING_TIMER              (VP_LINE_CAL_LINE_TIMER + 1)

/**< Offhook report delay when not pulse detecting */
#define VP_LINE_OFFHOOK_DELAY           (VP_LINE_PING_TIMER + 1)

/**< Timer used to disable switcher with low power termination type. */
#define VP_LINE_TRACKER_DISABLE         (VP_LINE_OFFHOOK_DELAY + 1)

/**< Timer for Cal operations */
#define VP_LINE_CAL_TIMER               (VP_LINE_TRACKER_DISABLE + 1)

/**< Timer to use a GPIO as a slow clock output */
#define VP_LINE_GPIO_CLKOUT_TIMER       (VP_LINE_CAL_TIMER + 1)

/**< Timer for the internal test termination */
#define VP_LINE_INTERNAL_TESTTERM_TIMER (VP_LINE_GPIO_CLKOUT_TIMER + 1)

/**< Timer to restore speedup recovery */
#define VP_LINE_SPEEDUP_RECOVERY_TIMER  (VP_LINE_INTERNAL_TESTTERM_TIMER + 1)

/**< Place holder to indicate number of line timers */
#define VP_LINE_TIMER_LAST              (VP_LINE_SPEEDUP_RECOVERY_TIMER + 1)


/**< Time after disconnect recovery for Hook mask */
#define VP_DISCONNECT_RECOVERY_TIME     100

/**< Device Timer Definitions */
/* Test line timer to ensure a more consistant MPI PCM collect routine, leave
 * this timer as the first timer. */
#define VP_DEV_TIMER_TESTLINE   0

/* Clock fail interrupt timer - used only in 790 API */
#define VP_DEV_TIMER_CLKFAIL            (VP_DEV_TIMER_TESTLINE + 1)

/* ABS Calibration timer */
#define VP_DEV_TIMER_ABSCAL             (VP_DEV_TIMER_CLKFAIL + 1)

/* Lower Power Mode Switcher Changes */
#define VP_DEV_TIMER_LP_CHANGE          (VP_DEV_TIMER_ABSCAL + 1)

/* ABV Caibration  device timers */
#define VP_DEV_TIMER_ABV_CAL            (VP_DEV_TIMER_LP_CHANGE + 1)

/* In-rush limiting enter ringing device timers */
#define VP_DEV_TIMER_ENTER_RINGING      (VP_DEV_TIMER_ABV_CAL + 1)

/* Used to enable Low Power Switching */
#define VP_DEV_TIMER_EXIT_RINGING       (VP_DEV_TIMER_ENTER_RINGING + 1)

/* Used to mask clock faults */
#define VP_DEV_TIMER_WB_MODE_CHANGE     (VP_DEV_TIMER_EXIT_RINGING + 1)

/* Place holder to indicate number of device timers */
#define VP_DEV_TIMER_LAST               (VP_DEV_TIMER_WB_MODE_CHANGE + 1)

#define VP_DEV_TIMER_EXIT_RINGING_SAMPLE    (5000)
#define VP_WB_CHANGE_MASK_TIME  (20)

/** FXO specific timer variables to be used for FXO type lines only */
typedef enum {
    VP_CSLAC_FXS_TIMER,
    VP_CSLAC_FXO_TIMER,
    VP_CSLAC_TIMER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} VpCslacTimerType;


#if (defined (VP_CC_880_SERIES) && defined (VP880_FXO_SUPPORT)) || \
    (defined (VP_CC_890_SERIES) && defined (VP890_FXO_SUPPORT)) || \
     defined (VP_CC_580_SERIES) || defined (VP_CC_KWRAP)
typedef struct {
    uint16  highToLowTime;      /**< Device timestamp of last high to low change */
    uint16  prevHighToLowTime;
    bool    noCount;            /**< TRUE when not counting - lack of activity */
    bool    lastState;          /**< TRUE if last known high, FALSE if low */
    uint16  timeLastPolRev;     /**< Time in 0.25ms since polrev detected */
    uint16  timePrevPolRev;     /**< Time in 0.25ms since prev polrev detected */
    uint8   maxPeriod;          /**< Time in 0.25ms that ringing is detected */
    uint16  lastStateChange;    /**< Time in 1mS since last state change */
    uint16  lastNotLiu;         /**< Time in 1mS since last Not LIU detected */
    uint16  disconnectDebounce; /**< Time in 1mS to debounce disconnect events */
    uint16  disconnectDuration; /**< Time in ticks that disconnect is detected */
    uint8   liuDebounce;        /**< Time in 1mS to debounce LIU detection that
                                     accompanies some kinds of ringing */
    uint8   ringOffDebounce;    /**< Time in 1mS to ignore disconnect after ring_off */
    uint8   ringTimer;          /**< When this timer expires, ringing is off */
    uint8   cidCorrectionTimer; /**< 1mS increments for CID correction timing */
    uint8   bCalTimer;          /**< 1mS increments for BFilter sampling time */
    uint16  fxoDiscIO2Change;   /**< Countdown in 1ms since IO2 changed for FXO_DISC termType */
    uint16  pllRecovery;        /**< Timer in ticks to recover PLL when FXO Disconnect to OHT */
    uint16  currentMonitorTimer;/**< Used to provide FXO line current buffer */
    uint16  measureBFilterTimer;/**< 1mS increments for BFilter sampling time */
    uint8   lowVoltageTimer;    /**< Timer in ticks for the low voltage
                                     disconnect/LIU distinction workaround */
} VpFXOTimerType;
#endif

/** Union of FXO and FXS timers since only one type can exist for each line */
typedef union {
#if (defined (VP_CC_880_SERIES) && defined (VP880_FXO_SUPPORT)) || \
    (defined (VP_CC_890_SERIES) && defined (VP890_FXO_SUPPORT)) || \
     defined (VP_CC_580_SERIES) || defined (VP_CC_KWRAP)
    VpFXOTimerType fxoTimer;
#endif
    uint16 timer[VP_LINE_TIMER_LAST]; /**< FXS Array of timers */
} VpCslacTimers;

typedef struct {
    VpCslacTimerType type;
    VpCslacTimers timers;
} VpCslacTimerStruct;
#endif
