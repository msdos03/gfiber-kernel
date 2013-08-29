#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/sysfs.h>

#define BOARD_NAME		"gflt200"
#define GPIO_BOARD_VER_0	13
#define GPIO_BOARD_VER_1	15
#define GPIO_BOARD_VER_2	18

static ssize_t board_hw_ver_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	int hw_ver = gpio_get_value(GPIO_BOARD_VER_0)
			| (gpio_get_value(GPIO_BOARD_VER_1) << 1)
			| (gpio_get_value(GPIO_BOARD_VER_2) << 2);

	return sprintf(buf, "%d\n", hw_ver);
}

static DEVICE_ATTR(hw_ver, S_IRUGO, board_hw_ver_show, NULL);

static struct i2c_board_info board_i2c_devices[] = {
	{
		I2C_BOARD_INFO("pcf8523", 0x68),
	},
};

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

	/* /sys/devices/platform/board/hw_ver */
	rc = device_create_file(&pdev->dev, &dev_attr_hw_ver);
	if (rc)
		pr_err(BOARD_NAME ": error %d creating attribute 'hw_ver'\n",
			rc);

	rc = i2c_register_board_info(0, board_i2c_devices,
					ARRAY_SIZE(board_i2c_devices));
	if (rc)
		pr_err(BOARD_NAME ": error %d registering board I2C devices\n",
			rc);

	return 0;
}

device_initcall(board_init);
