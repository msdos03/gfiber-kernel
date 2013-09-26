#include <boardEnv/mvBoardEnvLib.h>
#include <gpp/mvGpp.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/sysfs.h>

#define BOARD_NAME		"gflt110"

static ssize_t board_hw_ver_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	return sprintf(buf, "1.0\n");
}

static DEVICE_ATTR(hw_ver, S_IRUGO, board_hw_ver_show, NULL);

static struct gpio_led board_gpio_leds[] = {
	{
		.name = "sys-blue",
		.gpio = 12,
		.default_state = LEDS_GPIO_DEFSTATE_OFF,
	},
	{
		.name = "sys-red",
		.gpio = 13,
		.default_state = LEDS_GPIO_DEFSTATE_ON,
	},
};

static int board_gpio_blink_set(unsigned gpio, unsigned long *delay_on,
				unsigned long *delay_off);

static struct gpio_led_platform_data board_gpio_leds_data = {
	.gpio_blink_set = board_gpio_blink_set,
};

int board_gpio_blink_set(unsigned gpio, unsigned long *delay_on,
			 unsigned long *delay_off)
{
	int i;
	int active_low;
	MV_U32 mask = 1 << (gpio %32);
	MV_U32 group = gpio / 32;
	MV_U32 cycles_per_ms = mvBoardTclkGet() / 1000;
	unsigned long max_delay = ~0 / cycles_per_ms;

	for (i = 0; i < board_gpio_leds_data.num_leds; i++) {
		if (gpio == board_gpio_leds_data.leds[i].gpio) {
			active_low = board_gpio_leds_data.leds[i].active_low;
			break;
		}
	}

	if (i == board_gpio_leds_data.num_leds)
		return -EINVAL;

	*delay_on = min(*delay_on, max_delay);
	*delay_off = min(*delay_off, max_delay);

	if (*delay_on && *delay_off) {
		if (active_low)
			mvGppBlinkCntrSet(MV_GPP_BLINK_CNTR_A,
                                          *delay_off * cycles_per_ms,
                                          *delay_on * cycles_per_ms);
		else
			mvGppBlinkCntrSet(MV_GPP_BLINK_CNTR_A,
                                          *delay_on * cycles_per_ms,
                                          *delay_off * cycles_per_ms);

		mvGppBlinkEn(group, mask, mask);
	}
	else
		mvGppBlinkEn(group, mask, 0);

	return 0;
}

static struct platform_device board_gpio_leds_device = {
        .name = "leds-gpio",
        .id = -1,
        .dev.platform_data = &board_gpio_leds_data,
};

int __init board_init(void)
{
	int rc;
	struct platform_device *pdev;

	board_gpio_leds_data.num_leds
			= ARRAY_SIZE(board_gpio_leds);
	board_gpio_leds_data.leds = board_gpio_leds;

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

	rc = platform_device_register(&board_gpio_leds_device);
	if (rc)
		pr_err(BOARD_NAME ": error %d registering GPIO LEDs device\n",
			rc);

	return 0;
}

device_initcall(board_init);
