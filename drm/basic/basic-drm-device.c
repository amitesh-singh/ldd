#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");

static void
_release(struct device *dev)
{
	pr_info("device.release()");
}

static struct platform_device my_drm_device = {
	.name="x,display",
	.id = -1,
	.dev.release = _release,
};

static int __init
device_init(void)
{
	platform_device_register(&my_drm_device);
	return 0;
}


static void __exit
device_exit(void)
{
	platform_device_unregister(&my_drm_device);
}

module_init(device_init);
module_exit(device_exit);
