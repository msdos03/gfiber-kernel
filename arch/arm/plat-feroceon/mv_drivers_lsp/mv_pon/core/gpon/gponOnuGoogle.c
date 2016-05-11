/*
 * arch/arm/plat-feroceon/mv_drivers_lsp/mv_pon/core/gpon/gponOnuGoogle.c
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#include "gponOnuHeader.h"


int onuGponLaserChannelPrint(char* buf)
{
  int off, rc;
  MV_32 msgCount;
  MV_U8 ponIdBytes[ONU_GPON_PON_ID_BYTES_LEN];

  msgCount = onuGponDbPONIdGetCount();
  rc = -1;
  if (msgCount > 0) {
    onuGponDbPONIdBytesInfoGet(ponIdBytes);
    rc = ponIdBytes[0] & 0x3f;
  }

  off = mvOsSPrintf(buf, "%d\n", rc);
  return off;
}

