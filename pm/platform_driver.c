#include <linux/init.h> // macros used to markup functions e.g. __init, __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/kernel.h> // Contains types, macros, functions for the kernel

#include <linux/platform_device.h>
#include <linux/pm.h> //for power management

#include "common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("simple platform module");
MODULE_VERSION("0.1"); // --> module version of kernel

static int
_sample_platform_driver_probe(struct platform_device *pdev)
{
   struct resource *res1, *res2;
   printk(KERN_INFO "platfrom device connected/probed");

   //get the first memory address
   res1 = platform_get_resource(pdev, IORESOURCE_MEM, 0);
   if (unlikely(!res1))
     {
        printk(KERN_ALERT "failed to get rest1");
        return -1;
     }

   printk(KERN_INFO "memory area 1: Start: %x, end: %x, Size: %d",
          (unsigned long)res1->start,
          (unsigned long) res1->end,
          resource_size(res1));

   //get the 2nd memory information
   res2 = platform_get_resource(pdev, IORESOURCE_MEM, 1);
   if (unlikely(!res2))
     {
        printk(KERN_ALERT "Failed to get res2");
        return -1;
     }
   printk(KERN_INFO "memory area 2: Start: %x, end: %x, Size: %d",
          (unsigned long)res2->start,
          (unsigned long) res2->end,
          resource_size(res2));

   //get IRQ of device
   printk(KERN_ALERT "IRQ: %d", platform_get_irq(pdev, 0));
   return 0;
}

static int
_sample_platform_driver_remove(struct platform_device *pdev)
{
   printk(KERN_INFO "platfrom device removed");
   return 0;
}


static int _suspend(struct device *dev)
{
	pr_err("driver suspend");
	return 0;
}

static int _resume(struct device *dev)
{
	pr_err("driver resumed");
	return 0;
}

//static const struct dev_pm_ops pm_ops = {
//	.suspend = _suspend,
//	.resume = _resume,
//}; or

static const struct dev_pm_ops pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(_suspend, _resume)
};
static struct platform_driver sample_platform_driver = {
     .probe = _sample_platform_driver_probe,
     .remove = _sample_platform_driver_remove,
     .driver = {
          .name = DRIVER_NAME, //platform_device will also use same name
          .owner = THIS_MODULE, //good practice to declare it
	  .pm = &pm_ops,
     },
};

static int __init
_platform_driver_init(void)
{
   printk(KERN_INFO "platform driver init");
   platform_driver_register(&sample_platform_driver);

   return 0;
}

static void __exit
_platform_driver_exit(void)
{
   printk(KERN_INFO "platform driver exit");
   platform_driver_unregister(&sample_platform_driver);
}

module_init(_platform_driver_init);
module_exit(_platform_driver_exit);
