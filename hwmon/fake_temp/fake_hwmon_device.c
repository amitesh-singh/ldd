#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>

static struct platform_device *pdev;

static int __init fake_hwmon_dev_init(void)
{
	pdev = platform_device_register_simple("fake_hwmon", -1, NULL, 0);
	return IS_ERR(pdev) ? PTR_ERR(pdev) : 0;
}

static void __exit fake_hwmon_dev_exit(void)
{
	platform_device_unregister(pdev);
}

module_init(fake_hwmon_dev_init);
module_exit(fake_hwmon_dev_exit);

MODULE_LICENSE("GPL");

