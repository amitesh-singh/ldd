#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/spi/spi_gpio.h>

//TODO: write a char device to pass data from userspace
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("custom soc chip driver to create fake GPIOs");
MODULE_VERSION("0.1");

static struct spi_gpio_platform_data spi_data = {
   .sck = 11,
   .mosi = 12,
   .miso = 13,
   .num_chipselect = 4,
};

static void
_device_release(struct device *pdev)
{
   printk(KERN_ALERT "Device is released");
}

static struct platform_device my_device = {
     .name = "spi_gpio",
     .id = -1, //let kernel decide 
     .dev.platform_data = &spi_data,
     .dev.release = _device_release,
};

static int __init
_platform_device_init(void)
{
   printk(KERN_INFO "platform device init");
   platform_device_register(&my_device);

   return 0;
}

static void __exit
_platform_device_exit(void)
{
   printk(KERN_INFO "platform device exit");
   platform_device_unregister(&my_device);
}

module_init(_platform_device_init);
module_exit(_platform_device_exit);
