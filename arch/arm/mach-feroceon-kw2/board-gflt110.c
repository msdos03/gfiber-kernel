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
#include <linux/ctype.h>


struct gflt_led_data {
	unsigned gpio;
	unsigned active_low;
	struct led_classdev cdev;
};

static ssize_t board_hw_ver_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	return sprintf(buf, "1.0\n");
}

static DEVICE_ATTR(hw_ver, S_IRUGO, board_hw_ver_show, NULL);
static void gflt_led_brightness_set(struct led_classdev *led_cdev,
				    enum led_brightness enum_brightness);

static struct gflt_led_data gflt110_board_gpio_leds[] = {
	{
		.gpio = 12,
		.cdev = {
			.name = "sys-blue",
			.brightness_set = gflt_led_brightness_set,
			.max_brightness = 100,
		},
	},
	{
		.gpio = 13,
		.cdev = {
			.name = "sys-red",
			.brightness_set = gflt_led_brightness_set,
			.max_brightness = 100,
		},
	},
};

static struct gflt_led_data gflt300_board_gpio_leds[] = {
	{
		.gpio = 9,
		.active_low = 1,
		.cdev = {
			.name = "sys-blue",
			.brightness_set = gflt_led_brightness_set,
			.max_brightness = 100,
		},
	},
	{
		.gpio = 10,
		.active_low = 1,
		.cdev = {
			.name = "sys-red",
			.brightness_set = gflt_led_brightness_set,
			.max_brightness = 100,
		},
	},
};

static void gflt_led_brightness_set(struct led_classdev *led_cdev,
				    enum led_brightness enum_brightness)
{
	struct gflt_led_data *led_data =
               container_of(led_cdev, struct gflt_led_data, cdev);

	unsigned gpio = led_data->gpio;
	unsigned brightness = (unsigned)enum_brightness;

	MV_U32 mask = 1 << (gpio % 32);
	MV_U32 group = gpio / 32;
	MV_U32 cycles_per_ms = mvBoardTclkGet() / 1000;

	if (brightness && brightness < led_cdev->max_brightness) {
		unsigned long delay_on;
		unsigned long delay_off;

		delay_on = cycles_per_ms * brightness / led_cdev->max_brightness;
		delay_off = cycles_per_ms - delay_on;
		if (led_data->active_low)
			mvGppBlinkCntrSet(MV_GPP_BLINK_CNTR_A,
					  delay_off,
					  delay_on);
		else
			mvGppBlinkCntrSet(MV_GPP_BLINK_CNTR_A,
					  delay_on,
					  delay_off);

		mvGppBlinkEn(group, mask, mask);
	} else if(brightness) {
		mvGppValueSet(group, mask, led_data->active_low ? 0 : mask);
		mvGppBlinkEn(group, mask, 0);
	} else {
		mvGppValueSet(group, mask, led_data->active_low ? mask : 0);
		mvGppBlinkEn(group, mask, 0);
	}
}

static int register_gfltleds(struct platform_device *pdev,
				struct gflt_led_data* gflt_leds,
				unsigned gflt_leds_num_leds)
{
	int ret;
	int i;

	for (i = 0; i < gflt_leds_num_leds; i++) {
		ret = led_classdev_register(&pdev->dev, &gflt_leds[i].cdev);
		if (ret)
			goto err_reg;
	}
	return 0;

err_reg:
	for (i-- ; i >= 0; i--) {
		led_classdev_unregister(&gflt_leds[i].cdev);
	}
	return ret;
}

#define BOARDNAME_LEN 32

int __init board_init(void)
{
	struct platform_device *pdev;
	MV_U32 boardID;
	MV_BOARD_INFO *pBoardInfo;
	char boardName[BOARDNAME_LEN];
	int rc, i;
	char c;
	struct gflt_led_data *led_data;
	int led_data_max;


	boardID = mvBoardIdGet();

	if ((boardID != GFLT300_ID) && (boardID != GFLT110_ID)) {
		printk("Unsupported board id:%d\n", boardID);
		return -1;
	}

	pBoardInfo = mvBoardInfoGet();
	for(i = 0; i < (BOARDNAME_LEN -1); i++) {
		c = pBoardInfo->boardName[i];
		if (!c)
			break;
		boardName[i] = tolower(c);
	}
	boardName[i] = 0;

	printk("Detected board id:%x name:%s\n", boardID, boardName);
	/* /sys/devices/platform/<board_name> */
	pdev = platform_device_register_simple(boardName, -1, NULL, 0);
	if (IS_ERR(pdev)) {
		rc = PTR_ERR(pdev);
		pr_err("%s: error %d registering device\n", boardName, rc);
		return rc;
	}

	/* /sys/devices/platform/board -> /sys/devices/platform/<board_name> */
	rc = sysfs_create_link(&pdev->dev.parent->kobj, &pdev->dev.kobj,
			       "board");
	if (rc)
		pr_err("%s: error %d creating link 'board'\n", boardName, rc);

	/* /sys/devices/platform/board/hw_ver */
	rc = device_create_file(&pdev->dev, &dev_attr_hw_ver);
	if (rc)
		pr_err("%s: error %d creating attribute 'hw_ver'\n", boardName, rc);

	led_data = gflt110_board_gpio_leds;
	led_data_max = ARRAY_SIZE(gflt110_board_gpio_leds);
	if (boardID == GFLT300_ID) {
		led_data = gflt300_board_gpio_leds;
		led_data_max = ARRAY_SIZE(gflt300_board_gpio_leds);
	}
	rc = register_gfltleds(pdev, led_data, led_data_max);
	if (rc)
		pr_err("%s: error %d registering GFLT LED device\n", boardName, rc);

	return 0;
}

device_initcall(board_init);
