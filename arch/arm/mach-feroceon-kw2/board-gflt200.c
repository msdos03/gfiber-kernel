#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/sysfs.h>

#define BOARD_NAME		"gflt200"

int __init board_init(void)
{
	int rc;
	struct platform_device *pdev;

	/* /sys/devices/platform/<board_name> */
	pdev = platform_device_register_simple(BOARD_NAME, -1, NULL, 0);
	if (IS_ERR(pdev)) {
		rc = PTR_ERR(pdev);
		pr_err(BOARD_NAME ": error %d registering device\n", rc);
		return rc;
	}

	/* /sys/devices/platform/board -> /sys/devices/platform/<board_name> */
	rc = sysfs_create_link(&pdev->dev.parent->kobj, &pdev->dev.kobj,
				"board");
	if (rc)
		pr_err(BOARD_NAME ": error %d creating link 'board'\n", rc);

	return 0;
}

device_initcall(board_init);
