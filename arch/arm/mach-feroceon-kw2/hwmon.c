/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <linux/hwmon.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/sysfs.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/platform_device.h>
#include <linux/cpu.h>
#include <asm/io.h>
#include "mvOs.h"
#include "ctrlEnv/mvCtrlEnvLib.h"


#define TIMEOUT 10000
#define KW2_OVERHEAT_TEMP	105000		/* milidegree Celsius */
#define KW2_OVERCOOL_TEMP	10000		/* milidegree Celsius */


#define THERMAL_STATUS_REG 0x184d8
#define THERMAL_CALIBRATION_BIT 0
#define THERMAL_CALIBRATION_MASK (1 << THERMAL_CALIBRATION_BIT)
#define THERMAL_VALID_BIT 9
#define THERMAL_VALID_MASK (1 << THERMAL_VALID_BIT)
#define THERMAL_TEMPERATURE_OFFSET 10
#define THERMAL_TEMPERATURE_MASK (0x1FF << THERMAL_TEMPERATURE_OFFSET)

#define STABILITY_ITERATIONS 4

#define RAW_TO_TEMP(x) ((((322 - x) * 10000)/13625)*1000)

typedef enum { 
	SHOW_TEMP, 
	TEMP_MAX,
	TEMP_MIN,
	SHOW_NAME } SHOW;

static struct device *hwmon_dev;



/* attributes methods */
static ssize_t show_name(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "kw2Thermal\n");
}


static ssize_t show_temp(struct device *dev,
			 struct device_attribute *devattr, char *buf) {
	int ret;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);

	if (attr->index == SHOW_TEMP)
		ret = sprintf(buf, "%d\n", mvCtrlTemperatureGet()*1000); /* lm sensors uses mili celsius! */
	else if (attr->index == TEMP_MAX)
		ret = sprintf(buf, "%d\n", KW2_OVERHEAT_TEMP);
	else if (attr->index == TEMP_MIN)
		ret = sprintf(buf, "%d\n", KW2_OVERCOOL_TEMP);
	else
		ret = sprintf(buf, "%d\n", -1);

	return ret;
}




/* sysfs attributes define */
static SENSOR_DEVICE_ATTR(name, S_IRUGO, show_name, NULL, SHOW_NAME);
static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, show_temp, NULL, SHOW_TEMP);
static SENSOR_DEVICE_ATTR(temp1_max, S_IRWXUGO, show_temp, NULL, TEMP_MAX);
static SENSOR_DEVICE_ATTR(temp1_min, S_IRWXUGO, show_temp, NULL, TEMP_MIN);

static struct attribute *kw2ThermalAttributes[] = {
	&sensor_dev_attr_name.dev_attr.attr,
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp1_max.dev_attr.attr,
	&sensor_dev_attr_temp1_min.dev_attr.attr,
	NULL,
};

static const struct attribute_group kw2_thermal_group = {
	.attrs	= kw2ThermalAttributes,
};



/* sysfs  group create, and register hwmon driver */
static int kw2_thermal_probe(struct platform_device * plat_dev)
{
	int err;
	/* init_thermal_sensor(); */
	
	/* Register sysfs hooks */
	err = sysfs_create_group(&plat_dev->dev.kobj, &kw2_thermal_group);
	if (err)
		goto exit;

	/* Register a new directory entry with module sensors */
	hwmon_dev = hwmon_device_register(&plat_dev->dev);
	if (IS_ERR(hwmon_dev)) {
		printk("hwmon device register failed\n");
		goto exit_remove_group;
	}

	return 0;

exit_remove_group:
	sysfs_remove_group(&plat_dev->dev.kobj, &kw2_thermal_group);
exit:
	return err;
}



static int __devexit kw2_thermal_remove(struct platform_device *plat_dev)
{
	hwmon_device_unregister(hwmon_dev);
	sysfs_remove_group(&plat_dev->dev.kobj, &kw2_thermal_group);
	return 0;
}


static struct platform_driver kw2_thermal_driver = {
	.probe		= kw2_thermal_probe,
	.remove		= __devexit_p(kw2_thermal_remove),
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "KW2Thermal",
	},
};

static int __init thermal_init(void)
{
	return platform_driver_register(&kw2_thermal_driver);
}

static void __exit thermal_exit(void)
{
	platform_driver_unregister(&kw2_thermal_driver);
}

module_init(thermal_init);
module_exit(thermal_exit);

MODULE_AUTHOR("Marvell Semiconductors");
MODULE_DESCRIPTION("KW2 - Thermal sensor");
MODULE_LICENSE("GPL");
