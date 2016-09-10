#include <linux/init.h> // macros used to markup functions e.g. __init, __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/kernel.h> // Contains types, macros, functions for the kernel

#include <linux/platform_device.h>

#include "common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("simple platform device module");
MODULE_VERSION("0.1");

static struct resource sample_resources[] = {
       {
          .start = 0x100000,
          .end = 0x1FFFFF,
          .flags = IORESOURCE_MEM,
       },

       {
          .start = 0x300000,
          .end = 0x3FFFFF,
          .flags = IORESOURCE_MEM,
       },

       {
          .start = 6,
          .end = 6,
          .flags = IORESOURCE_IRQ,
       },
};

static void
_platform_device_release(struct device *dev)
{
   printk(KERN_ALERT "device relase():");
}

static struct platform_device sample_platform_device = {
   .name = DRIVER_NAME,
   .id = -1,
   .num_resources = ARRAY_SIZE(sample_resources),
   .resource = sample_resources,
   .dev = {
        .release = _platform_device_release
           // device release() is called
   }
};

static int __init
_platform_device_init(void)
{
   printk(KERN_INFO "platform driver init");
   platform_device_register(&sample_platform_device);
   return 0;
}

static void __exit
_platform_device_exit(void)
{
   printk(KERN_INFO "platform driver exit");
   platform_device_unregister(&sample_platform_device);
}

module_init(_platform_device_init);
module_exit(_platform_device_exit);
