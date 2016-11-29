#include "boardEnv/mvBoardEnvLib.h"
#include "boardEnv/mvBoardEnvSpec.h"
#include "gflt.h"

int gfiber_is_fiber_jack(void) {
  MV_U32 boardId = mvBoardIdGet();
  return boardId == GFLT110_ID ||
      boardId == GFLT200_ID ||
      boardId == GFLT300_ID;
}

int gfiber_is_coax_jack(void) {
  MV_U32 boardId = mvBoardIdGet();
  return boardId == GFLT400_ID;
}

int gfiber_is_any_jack(void) {
  return gfiber_is_fiber_jack() || gfiber_is_coax_jack;
}
