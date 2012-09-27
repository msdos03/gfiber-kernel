/** \file vp_api_cfg.h
 * vp_api_cfg.h
 *
 * This file contains the configuration and compile time settings for
 * building appropriate VP-API library modules needed for any application.
 *
 * Copyright (c) 2011, Microsemi
 *
 * $Revision: 9058 $
 * $LastChangedDate: 2011-11-07 14:21:29 -0600 (Mon, 07 Nov 2011) $
 */

#ifndef VP_API_CFG_H
#define VP_API_CFG_H

#include "vp_debug_masks.h"

/**< NOTE Regarding compile-time settings to reduce code size. The values
 * provided in these comments is for approximation and comparison purposes
 * only. Specific savings will vary depending on the compiler and target
 * processor.
 */

/**< VP_REDUCED_API_IF
 * Define this to remove the I/F for unsupported functions. This reduces
 * code size with the only limitation that the application cannot call
 * functions that would return "Unsupported" for all devices used in the
 * application. The specific functions removed will depend on the device and
 * test packages compiled in.
 */
#define VP_REDUCED_API_IF

/******************************************************************************
 * COMPILE-TIME OPTIONS:: API DEBUG OUTPUT SELECTIONS                         *
 *****************************************************************************/

/**< VP_DEBUG
 * If VP_DEBUG is undefined, then ALL debug messages will be compiled out. In
 * this case, the code size will be smaller, but debug output CANNOT be enabled
 * at runtime.
 */
#undef  VP_DEBUG

/**< VP_CC_DEBUG_SELECT
 * Choose which types of debug messages to be compiled in.  Only these message
 * types can be enabled at runtime. See vp_debug_masks.h for a list of debug
 * message types.
 */
#define VP_CC_DEBUG_SELECT (VP_DBG_ALL)

/**< VP_OPTION_DEFAULT_DEBUG_SELECT
 * The VP_OPTION_ID_DEBUG_SELECT option enables/disables debug output at
 * runtime.  Define the default value of this option here.  The option will be
 * automatically set to the default value at device initialization.  See
 * vp_debug_masks.h for a list of debug message types.
 *
 * Note: The VP_OPTION_ID_DEBUG_SELECT option can be changed with VpSetOption()
 *       only AFTER device initialization (VpInitDevice()).  If you wish to
 *       see debug output messages during device initialization, you must set
 *       the default VP_OPTION_ID_DEBUG_SELECT value accordingly.
 */
#define VP_OPTION_DEFAULT_DEBUG_SELECT  0

/******************************************************************************
 * COMPILE-TIME OPTIONS:: Conditionally-Compiled API Libraries                *
 *****************************************************************************/

/*
 * Define (or undefine) the appropriate compile time switches based on your
 * application needs.
 *
 * NOTE: Such library selection compile time option(s) MUST be defined before
 * including any other file from VoicePath library.
 *
 * NOTE: More than one Library modules can be built simultaneosuly (if needed).
 */
#ifndef ZARLINK_CFG_INTERNAL

  /*
   * Define the API libraries that will be included in this build
   * only if the kernel wrapper library is not supported.
   */

  #undef  VP_CC_790_SERIES   /**< define to build 790 specific API library;
                              *   undef to exclude this library. */

#if defined(ZARLINK_SLIC_VE792)
  #define  VP_CC_792_SERIES    /**< define to build 792 specific API library;
                             *   undef to exclude this library. */
#endif

#if defined(ZARLINK_SLIC_VE880)
  #define  VP_CC_880_SERIES   /**< define to build 880 specific API library;
                             *   undef to exclude this library. */
#endif

  #undef  VP_CC_890_SERIES   /**< define to build 890 specific API library;
                              *   undef to exclude this library. */

  #undef  VP_CC_580_SERIES   /**< define to build 580 specific API library;
                              *   undef to exclude this library. */

  /* VCP (VCP) library */
  #undef  VP_CC_VCP_SERIES   /**< define to build VCP specific API library;
                              *   undef to exclude this library. */
  /* VCP2 library */
  #undef  VP_CC_VCP2_SERIES  /**< define to build VCP2 specific API library;
                              *   undef to exclude this library. */

  #undef  VP_CC_MELT_SERIES  /**< define to build Melt specific API library;
                              *   undef to exclude this library. */

  #undef VP_CC_KWRAP

  #if defined(VP_CC_KWRAP) && defined(__KWRAP__)
    #error "VP_CC_KWWAP is only applicable to user space builds"
  #endif

#endif

/******************************************************************************
 * Include Files for the API                                                  *
 *****************************************************************************/
/* Include the API types for this architecture */
#include "vp_api_types.h"

/*
 * Undef the following to remove the code necessary to run the API-II sequencer
 * for CSLAC devices and to enable compile of API-II Lite. This is useful to
 * reduce code space if the host is not using API-II "advanced" features.
 */
#if defined(VP_CC_790_SERIES) || defined(VP_CC_880_SERIES) || \
    defined(VP_CC_890_SERIES) || defined(VP_CC_580_SERIES)
    /**< VP_CSLAC_SEQ_EN
     * This is used to support all cadencing type operations on both FXO and
     * FXS line types. This includes Ringing and Tone Cadencing, Metering
     * Pulse, and VpSendSignal(). When set to #undef the VE890 code reduces by
     * ~17K, the VE880 code reduces by ~20K.
     */
#define VP_CSLAC_SEQ_EN
#endif

#if defined(VP_CC_880_SERIES) || defined(VP_CC_890_SERIES)
    /**< VP_CSLAC_RUNTIME_CAL_ENABLED
     * This is used to support run-time calibration via VpInitDevice() and
     * VpCalLine(). With this #undef, System Coefficients can still be applied
     * using VpCal(). Set to #undef saves VE880 code size by ~19K bytes (if
     * both Tracker and ABS are supported) and VE890 by ~10K bytes.
     */
    #define VP_CSLAC_RUNTIME_CAL_ENABLED

    /**< VP_HIGH_GAIN_MODE_SUPPORTED
     * This is used to support High Gain States VP_LINE_HOWLER and
     * VP_LINE_HOWLER_POLREV. Set to #undef saves code size for both VE880 and
     * VE890 by ~2K bytes.
     */
    #define VP_HIGH_GAIN_MODE_SUPPORTED

    /**< UK Howler Tone Specifications
     * VE880 and VE890 Applications only. The values below affect how UK Howler
     * Tone will be provided on the line. To send UK Howler Tone the
     * application must do the following:
     *
     *      - Have the full version of VP_API-II (i.e., Le71SK0002)
     *      - Set VP_CLSAC_SEQ_EN to #define. Note: if setting VP_CLSAC_SEQ_EN
     *        to #define causes compiler errors, the Make process may either
     *        not be including the required *.c and *.h files or may be using
     *        the "Lite" version of the API (i.e., Le71SDKAPIL).
     *      - Create a Tone Cadence Profile with UK Howler Tone.
     *      - Provide this tone cadence to VpSetLineTone()
     *
     * There are two types of UK Howler Tones supported in the API:
     *
     *      - VP_UK_HOWLER_BTNR_VER15
     *          The BTNR Version 15 Howler Tone will perform a frequency sweep
     *          between [800Hz to 3200Hz] at a 500ms period. Total sweep from
     *          [800-3200-800] will be nominal 1 second +/-166ms. At each 1
     *          second interval over a total duration of 12 seconds the tone
     *          level will increase by 3dB for total level increase of 36dB.
     *          The final level will be a nominal +14.5dBm @ 600ohm on T/R
     *          when also used with VP_LINE_HOWLER or VP_LINE_HOWLER_POLREV
     *          line states (see VpSetLineState() in API Reference Guide).
     *
     *      - VP_UK_HOWLER_BTNR_DRAFT_G
     *          Draft 960-G Howler Tone is very similar to Version 15 with
     *          only a few differences. The frequency sweep is between [800Hz
     *          to 2500Hz] and the level varies by 3dB during the sweep
     *          interval such that the level at 2500Hz is ~3dB higher than the
     *          level at 800Hz. The total sweep time of 12 seconds is the same
     *          as well as the final output level of +14.5dBm @ 600ohm on T/R
     *          when also used with VP_LINE_HOWLER or VP_LINE_HOWLER_POLREV
     *          line states (see VpSetLineState() in API Reference Guide).
     *
     * The Application may choose which BTNR Specification to use when
     * generating the UK Howler Tone by setting VP_UK_HOWLER_IN_USE value as
     * follows:
     *
     *      To use BTNR Version 15:
     *          #define VP_UK_HOWLER_IN_USE VP_UK_HOWLER_BTNR_VER15
     *
     *      To use BTNR Draft-960 G:
     *          #define VP_UK_HOWLER_IN_USE VP_UK_HOWLER_BTNR_DRAFT_G
     *
     *      Previous releases (P2.18.0 and earlier) provided UK Howler Tone
     *      per VP_UK_HOWLER_BTNR_VER15.
     */
    #define VP_UK_HOWLER_BTNR_VER15     0
    #define VP_UK_HOWLER_BTNR_DRAFT_G   1
    #define VP_UK_HOWLER_IN_USE         VP_UK_HOWLER_BTNR_VER15

#endif  /* defined(VP_CC_880_SERIES) || defined(VP_CC_890_SERIES) */

/**< EXTENDED_FLASH_HOOK
 * Define the following to include the onHookMin member in the
 * VpOptionPulseType struct.
 */
#undef  EXTENDED_FLASH_HOOK

/******************************************************************************
 * Library Specific COMPILE-TIME OPTIONS and defines                          *
 *****************************************************************************/
#ifdef VP_CC_790_SERIES
/* VE790 library can be configured to handle the interrupt in four different
 * configurations. Choose (only) one of the mechanisms by defining one of the
 * following. */
#define VP790_SIMPLE_POLLED_MODE        /* Define to use simple polled interrupt
                                         * handling mode */
#undef VP790_EFFICIENT_POLLED_MODE      /* Define to use efficient polled
                                         * interrupt handling mode */
#undef VP790_INTERRUPT_LEVTRIG_MODE     /* Define to use level triggered
                                         * interrupt handling mode */
#undef VP790_INTERRUPT_EDGETRIG_MODE    /* Define to use edge triggered
                                         * interrupt handling mode */
#endif /* VP_CC_790_SERIES */

#ifdef VP_CC_880_SERIES

/*******************************************************************************
 * Applications can reduce VP-API-II driver size by changing the following for
 * only the required silicon and termination types needed
 *
 * See VP-API-II User's Guide for Details.
 ******************************************************************************/
/**< Device Type Support.
 * One of these MUST be defined if FXS Support is enabled. If you're not sure
 * whether the silicon used in your application is ABS or Tracker, contact
 * Microsemi customer support.
 *
 * Note that disabling either ABS or Tracker removes the associated calibration
 * and line specific support (e.g., if disabling Tracker, then LPM also must be
 * disabled).
 */
#define VP880_ABS_SUPPORT   /**< VP880_ABS_SUPPORT
                             *  #define this to support VE880 ABS type silicon.
                             * This includes but not limited to Le88266/86 and
                             * Le8822x/24x. When set to #undef reduces code
                             * size by ~9K bytes.
                             */

#define VP880_TRACKER_SUPPORT   /**< VP880_TRACKER_SUPPORT
                                 * #define this to support VE880 Tracker type
                                 * silicon. This includes but not limited to
                                 * VE8820 Chipset and Le88276.
                                 */

/**< VE880 Termination Type Support.
 *
 * Note: Both ABS and Tracker can be #undef while FXO defined. This results in
 * support for FXO only silicon, and FXO/FXS silicon support for the FXO
 * termination type only. FXS support requires either ABS or Tracker support.
 */
#define VP880_FXO_SUPPORT
#define VP880_FXS_SUPPORT

/**< VP880_LP_SUPPORT
 * When set to #undef, all of the code in the VE880 API that supports
 * termination types VP_TERM_FXS_LOW_PWR, VP_TERM_FXS_SPLITTER_LP, and
 * VP_TERM_FXS_ISOLATE_LP will be removed. When disabled, the VE880 code
 * size is reduced by ~7K bytes.
 */
#define VP880_LP_SUPPORT

/**< Specific FXO Algorithm support.
 *
 * The VP880_CLARE Ringing Detect algorithm uses features of the Clare and VE880
 * silicon to provide maximum performance of signal detection on FXO lines.
 * Alternative methods (with VP880_CLARE_RINGING_DETECT set to #undef) do not
 * require connection of I/O4 to Clare device, but have the following drawbacks:
 *
 *    1. Ringing Signals MUST contain polarity reversal to be detected AND
 *       must exceed ~70V peak.
 *    2. Line-In-Use is disabled.
 *    3. Frequency Discriminaation is poor (limited to tickrate).
 *
 * It is recommended that all designs connect the CLARE Ringing output to IO4
 * of the silicon. In that case, this value should be #define.
 *
 * Note: If FXO Support is disabled, this setting has no affect.
 */
#define VP880_CLARE_RINGING_DETECT

/**< VP880_FXO_FULL_WAVE_RINGING
 * Selects whether the Ringing input is 1/2-wave of full-wave. This must be
 * set to #undef when using CPC5621 since this device provides only 1/2-wave
 * ringing. For all other Clare devices, set to #define.
 */
#define VP880_FXO_FULL_WAVE_RINGING

/**< VP880_ALWAYS_USE_INTERNAL_TEST_TERMINATION
 * Define this option to always use the internal test termination for relay
 * state VP_RELAY_BRIDGED_TEST, even for devices that have the test load
 * switch. This is Microsemi's recommendation since the Internal Test
 * Termination provides better isolation from the customer load and loop
 * conditions when compared to the external test load.
 */
#undef  VP880_ALWAYS_USE_INTERNAL_TEST_TERMINATION

/* VE880 library can be configured to handle the interrupt in four different
 * configurations. Choose (only) one of the mechanisms by defining one of the
 * following. */
#define VP880_SIMPLE_POLLED_MODE        /* Define to use simple polled interrupt
                                         * handling mode */
#undef  VP880_EFFICIENT_POLLED_MODE     /* Define to use efficient polled
                                         * interrupt handling mode */
#undef  VP880_INTERRUPT_LEVTRIG_MODE    /* Define to use level triggered
                                         * interrupt handling mode */
#undef  VP880_INTERRUPT_EDGETRIG_MODE   /* Define to use edge triggered
                                         * interrupt handling mode */
#undef  VP880_INCLUDE_TESTLINE_CODE     /* Defines whether or not to enable
                                         * 880 line test capabilities */

/**< VE880 Line Test Package Defines
 * Defines all possible VE880 test line packages. There should be no reason to
 * change these values. But if they are changed, they must be different from
 * each other.
 */
#define VP880_LINE_TEST_AUDITOR 1
#define VP880_LINE_TEST_PROFESSIONAL 2

#ifdef VP880_INCLUDE_TESTLINE_CODE

/**< VP880_INCLUDE_LINE_TEST_PACKAGE
 * Selects the VE880 Line Test Package to include. Note that both Auditor and
 * Professional cannot be included in the same build.
 */
#define VP880_INCLUDE_LINE_TEST_PACKAGE VP880_LINE_TEST_PROFESSIONAL

/**< VP880_EZ_MPI_PCM_COLLECT
 * #define this to have the api collect PCM samples via the MPI bus.
 * If #undef, the Line Test code will expect the PCM samples from an
 * external process (DSP or uProcessor) connected to the PCM BUS
 */
#define  VP880_EZ_MPI_PCM_COLLECT

/**< VP880_PCM_CALCULATION_TIME
 * Define the maximum expected computation time to arrive at results for
 * VpTestLineCallback() after having collected the necessary PCM samples.
 */
#define VP880_PCM_CALCULATION_TIME 1000     /* time is in ms */

#endif /* VP880_INCLUDE_TESTLINE_CODE */

#endif /* VP_CC_880_SERIES */

#ifdef VP_CC_580_SERIES
/* VE580 library can be configured to handle the interrupt in four different
 * configurations. Choose (only) one of the mechanisms by defining one of the
 * following. */
#define VP580_SIMPLE_POLLED_MODE        /* Define to use simple polled interrupt
                                         * handling mode */
#undef VP580_EFFICIENT_POLLED_MODE      /* Define to use efficient polled
                                         * interrupt handling mode */
#undef VP580_INTERRUPT_LEVTRIG_MODE     /* Define to use level triggered
                                         * interrupt handling mode */
#undef VP580_INTERRUPT_EDGETRIG_MODE    /* Define to use edge triggered
                                         * interrupt handling mode */
#endif

#ifdef VP_CC_890_SERIES

/**< VE890 Generic Termination Type Support */
#define VP890_FXO_SUPPORT
#define VP890_FXS_SUPPORT

/**< VP890_LP_SUPPORT
 * When set to #undef, all of the code in the VE890 API that supports
 * termination types VP_TERM_FXS_LOW_PWR, VP_TERM_FXS_SPLITTER_LP, and
 * VP_TERM_FXS_ISOLATE_LP will be removed. When disabled, the VE890 code
 * size is reduced by ~5K bytes.
 */
#define VP890_LP_SUPPORT

#undef  VP890_INCLUDE_TESTLINE_CODE     /* Defines whether or not to enable
                                         * 890 line test capabilities */
#ifdef VP890_INCLUDE_TESTLINE_CODE

/**< VE890 Line Test Package Defines
 * Defines all possible VE890 test line packages. There should be no reason to
 * change these values. But if they are changed, they must be different from
 * each other.
 */
#define VP890_LINE_TEST_AUDITOR 1
#define VP890_LINE_TEST_PROFESSIONAL 2

/**< VP890_INCLUDE_LINE_TEST_PACKAGE
 * Selects the VE890 Line Test Package to include. Note that both Auditor and
 * Professional cannot be included in the same build.
 */
#define VP890_INCLUDE_LINE_TEST_PACKAGE VP890_LINE_TEST_PROFESSIONAL

/**< VP890_EZ_MPI_PCM_COLLECT
 * #define this to have the api collect PCM samples via the MPI bus.
 * If #undef, the Line Test code will expect the PCM samples from an
 * external process (DSP or uProcessor) connected to the PCM BUS
 */
#define VP890_EZ_MPI_PCM_COLLECT

/**< VP890_PCM_CALCULATION_TIME
 * Define the maximum expected computation time to arrive at results for
 * Vp890TestLineCallback() after having collected the necessary PCM samples.
 */
#define VP890_PCM_CALCULATION_TIME 1000     /* time is in ms */

#endif /* VP890_INCLUDE_TESTLINE_CODE */

/* VE890 library can be configured to handle the interrupt in four different
 * configurations. Choose (only) one of the mechanisms by defining one of the
 * following. */
#define VP890_SIMPLE_POLLED_MODE        /* Define to use simple polled interrupt
                                         * handling mode */
#undef  VP890_EFFICIENT_POLLED_MODE     /* Define to use efficient polled
                                         * interrupt handling mode */
#undef  VP890_INTERRUPT_LEVTRIG_MODE    /* Define to use Level Triggered
                                         * interrupt handling mode */
#undef  VP890_INTERRUPT_EDGETRIG_MODE   /* Define to use edge triggered
                                         * interrupt handling mode */
#endif /* VP_CC_890_SERIES*/

typedef uint8 VpScratchMemType;

/* Include internal options required to build the VP-API-II library */
#include "vp_api_cfg_int.h"

#endif /* VP_API_CFG_H */
