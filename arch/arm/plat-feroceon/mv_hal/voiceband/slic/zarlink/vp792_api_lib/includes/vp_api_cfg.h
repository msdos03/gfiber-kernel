/** \file vp_api_cfg.h
 * vp_api_cfg.h
 *
 * This file contains the configuration and compile time settings for
 * building appropriate VP-API library modules needed for any application.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 8674 $
 * $LastChangedDate: 2011-06-22 10:13:07 -0500 (Wed, 22 Jun 2011) $
 */

#ifndef VP_API_CFG_H
#define VP_API_CFG_H

#ifdef MV_KERNEL_SLIC_SUPPORT
#include "mvOs.h"
#include "mvSysTdmConfig.h"
#endif

#include "vp_debug_masks.h"

/******************************************************************************
 * COMPILE-TIME OPTIONS:: API DEBUG OUTPUT SELECTIONS                         *
 *****************************************************************************/

/*
 * If VP_DEBUG is undefined, then ALL debug messages will be compiled out.
 * In this case, the code size will be smaller, but debug output CANNOT be
 * enabled at runtime.
 */
#define VP_DEBUG

/*
 * Choose which types of debug messages to be compiled in.  Only these message
 * types can be enabled at runtime.  See vp_debug_masks.h for a list of debug
 * message types.
 */
#define VP_CC_DEBUG_SELECT (VP_DBG_ALL)

/*
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

#define VP_CC_792_SERIES    /**< define to build 792 specific API library;
                             *   undef to exclude this library. */
 

/******************************************************************************
 * Include Files for the API                                                  *
 *****************************************************************************/
/* Include the API types for this architecture */
#include "vp_api_types.h"

/******************************************************************************
 * Library Specific COMPILE-TIME OPTIONS and defines                          *
 *****************************************************************************/
#ifdef VP_CC_792_SERIES

#undef VP_COMMON_ADDRESS_SPACE         /* Define if all VP-API-II data
                                           structures (Line Objects, Line Contexts,
                                           Device Objects, Device Contexts, Profiles)
                                           are accessible at the same address in
                                           all processes (tasks) which call VP-API-II
                                           functions. */

#define VP_CC_792_GROUP                 /* Define to include support for sharing
                                         * an interrupt pin between multiple
                                         * VP792 devices. */

#define VP792_MAILBOX_SPINWAIT 50000    /* Number of times to poll the device's
                                           command mailbox before returning
                                           VP_STATUS_MAILBOX_BUSY. */

#define VP792_SUPPORT_792388_REV_F_SILICON /* Define this if you want to support
                                              Revision F of the Le792388 silicon. An
                                              appropriate SLAC firmware patch will
                                              be compiled in. */
#define VP792_SUPPORT_792588_REV_B_SILICON /* Define this if you want to support
                                              Revision B of the Le792588 silicon. An
                                              appropriate SLAC firmware patch will
                                              be compiled in. */
#define EXTENDED_FLASH_HOOK

#endif /* VP_CC_792_SERIES */

typedef uint8 VpScratchMemType;

/* Include internal options required to build the VP-API-II library */
#include "vp_api_cfg_int.h"

#endif /* VP_API_CFG_H */


