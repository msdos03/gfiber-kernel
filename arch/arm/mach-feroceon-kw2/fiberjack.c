#include <linux/init.h>
#include <linux/string.h>
#include "fiberjack.h"

const char* get_model_name(void) {
  if (strstr(boot_command_line, "model=gflt300") != NULL)
    return "gflt300";
  else if (strstr(boot_command_line, "model=gflt110") != NULL)
    return "gflt110";
  else
    /* If we can't find a match, assume the default which is GFLT110 */
    return "gflt110";
}

int parse_model_from_cmdline(const char* model) {
  if (strstr(boot_command_line, model) != NULL)
    return 1;
  return 0;
}

int is_gflt300(void) {
  return parse_model_from_cmdline("gflt300");
}
