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

static int
_sample_platform_driver_probe(struct platform_device *pdev)
{
   struct my_device_platform_data *data;

   printk(KERN_INFO "platfrom device connected/probed");
   ///struct my_driver_data *mdd;

   data = dev_get_platdata(&pdev->dev);

   if (data->power_on) data->power_on(data);

   return 0;
}

static int
_sample_platform_driver_remove(struct platform_device *pdev)
{
   struct my_device_platform_data *data;

   data = dev_get_platdata(&pdev->dev);

   if (data->power_off) data->power_off(data);

   printk(KERN_INFO "platfrom device removed");
   return 0;
}

static struct platform_driver sample_platform_driver = {
     .probe = _sample_platform_driver_probe,
     .remove = _sample_platform_driver_remove,
     .driver = {
          .name = "ami-custom-platform-device", //platform_device will also use same name
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
