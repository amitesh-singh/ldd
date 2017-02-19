#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>


static void
_release(struct device *dev)
{
   printk("device.release()");
}


static struct platform_device xboard_pwm_device = {
    .name = "xboard-pwm",
    .id = -1, //let kernel device 
    //.dev.platform_data = &my_device_data,
    .dev.release = _release,
};

static int __init
_pwm_device_init(void)
{
    platform_device_register(&xboard_pwm_device);
    return 0;
}

static void __exit
_pwm_device_exit(void)
{
    platform_device_register(&xboard_pwm_device);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("xboard pwm device driver");
MODULE_VERSION("0.1");

module_init(_pwm_device_init);
module_exit(_pwm_device_exit);