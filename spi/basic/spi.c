#include <linux/init.h> // macros used to markup functions e.g. __init, __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/kernel.h> // Contains types, macros, functions for the kernel
#include <linux/spi/spi.h> //for spi

//In this module, we are communicating with a spi-slave
// using spi-master. It does not create the spi master or slave.

//slave device
static struct spi_device *sdev;

static int __init
_spi_init(void)
{
   int ret;
   unsigned char ch = 0x00;
   struct spi_master *master;
   struct spi_board_info spi_device_info = {
        .modalias = "ami-spi-device",
        .max_speed_hz = 1, //speed of your device splace can handle
        .bus_num = 435, //BUS number
        .chip_select = 0,
        .mode = 3,
   };

   printk(KERN_INFO "spi basic driver init");

   master = spi_busnum_to_master(spi_device_info.bus_num);
   if (!master)
     {
        printk(KERN_ALERT "Failed to create master device");
        return -ENODEV;
     }
   //create a slave new device, given the master and device info
   sdev = spi_new_device(master, &spi_device_info);
   if (!sdev)
     {
        printk(KERN_ALERT "Failed to create slave device");
        return -ENODEV;
     }

   sdev->bits_per_word = 8;


   ret = spi_setup(sdev);
   if (ret)
     {
        printk(KERN_ALERT "Failed to setup slave");
        spi_unregister_device(sdev);
        return -ENODEV;
     }

   spi_write(sdev, &ch, sizeof(ch));

   return 0;
}

static void __exit
_spi_exit(void)
{
   printk(KERN_INFO "spi basic driver exit");
   if (sdev)
     {
        unsigned char ch = 0xff;
        spi_write(sdev, &ch, sizeof(ch));
        spi_unregister_device(sdev);
     }
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh <singh.amitesh@gmail.com>");
MODULE_DESCRIPTION("SPI basic driver");
MODULE_VERSION("0.1");

module_init(_spi_init);
module_exit(_spi_exit);
