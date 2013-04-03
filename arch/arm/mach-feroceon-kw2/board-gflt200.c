#include <linux/init.h>

int __init board_init(void)
{
	return 0;
}

device_initcall(board_init);
