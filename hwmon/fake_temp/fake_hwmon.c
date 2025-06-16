// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/slab.h>
#include <linux/mutex.h>

struct fake_hwmon_data {
	int temp;
	struct mutex lock;
};

/* Called when sensors like temp1_input are read */
static int fake_hwmon_read(struct device *dev,
			   enum hwmon_sensor_types type,
			   u32 attr, int channel, long *val)
{
	struct fake_hwmon_data *data = dev_get_drvdata(dev);

	if (type != hwmon_temp || attr != hwmon_temp_input || channel != 0)
		return -EOPNOTSUPP;

	mutex_lock(&data->lock);
	data->temp += 100; // Increment 0.1°C on every read
	if (data->temp > 80000)
		data->temp = 25000;
	*val = data->temp;
	mutex_unlock(&data->lock);

	return 0;
}

static umode_t fake_hwmon_is_visible(const void *data,
                                     enum hwmon_sensor_types type,
                                     u32 attr, int channel)
{
	if (type == hwmon_temp && attr == hwmon_temp_input && channel == 0)
		return 0444;  // readable
	return 0;
}

/* HWMON interface setup */
static const struct hwmon_ops fake_hwmon_ops = {
	.read = fake_hwmon_read,
	.is_visible = fake_hwmon_is_visible
};

static const struct hwmon_channel_info *fake_hwmon_info[] = {
	HWMON_CHANNEL_INFO(temp, HWMON_T_INPUT),
	NULL
};

static const struct hwmon_chip_info fake_chip_info = {
	.ops = &fake_hwmon_ops,
	.info = fake_hwmon_info,
};

/* .probe() is called when the platform device is matched */
static int fake_hwmon_probe(struct platform_device *pdev)
{
	struct fake_hwmon_data *data;
	struct device *dev = &pdev->dev;
	struct device *hwmon_dev;

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	mutex_init(&data->lock);
	data->temp = 25000;

	platform_set_drvdata(pdev, data);

	hwmon_dev = devm_hwmon_device_register_with_info(dev, "fake_hwmon", data,
							 &fake_chip_info, NULL);
	if (IS_ERR(hwmon_dev))
		return PTR_ERR(hwmon_dev);

	dev_info(dev, "Fake HWMON sensor registered\n");
	return 0;
}

static struct platform_driver fake_hwmon_driver = {
	.driver = {
		.name = "fake_hwmon",
	},
	.probe = fake_hwmon_probe,
};

module_platform_driver(fake_hwmon_driver);

MODULE_AUTHOR("You");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Fake HWMON driver using platform_driver");

