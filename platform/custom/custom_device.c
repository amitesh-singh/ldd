#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("custom soc chip driver");
MODULE_VERSION("0.1");

struct my_device_platform_data
{
   int reset_gpio;
   int power_gpio;

   void (*power_on)(struct my_device_platform_data *);
   void (*power_off)(struct my_device_platform_data *);
   void (*reset)(struct my_device_platform_data *);
};

static void
_power_on(struct my_device_platform_data *sd)
{
   printk(KERN_INFO "custom device is powered on");
}

static void
_power_off(struct my_device_platform_data *sd)
{
   printk(KERN_INFO "custom device is powered off");
}

static void
_reset(struct my_device_platform_data *sd)
{
   printk(KERN_INFO "custom device is powered reset");
}

static struct my_device_platform_data my_device_data = {
     .reset_gpio = 435,
     .power_gpio = 436,
     .power_on = _power_on,
     .power_off = _power_off,
     .reset = _reset,
};

static void
_release(struct device *dev)
{
   printk("device.release()");
}

static struct platform_device my_device = {
     .name = "ami-custom-platform-device",
     .id = -1, //let kernel device 
     .dev.platform_data = &my_device_data,
     .dev.release = _release,
};

static int __init
_platform_driver_init(void)
{
   printk(KERN_INFO "platform device init");
   platform_device_register(&my_device);

   return 0;
}

static void __exit
_platform_driver_exit(void)
{
   printk(KERN_INFO "platform device exit");
   platform_device_unregister(&my_device);
}

module_init(_platform_driver_init);
module_exit(_platform_driver_exit);
