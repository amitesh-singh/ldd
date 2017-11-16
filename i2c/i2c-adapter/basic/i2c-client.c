#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#include <linux/slab.h>
#include <linux/i2c.h>

static struct i2c_board_info ds3231_rtc[] = {
       { I2C_BOARD_INFO("dts-3231", 0x50), },
};

struct i2c_client *i2c_device;
struct i2c_adapter *adapter;


static int __init
_i2c_client_driver_init(void)
{
   adapter =  i2c_get_adapter(8);
   if (!adapter)
     {
        printk(KERN_ALERT "Failed to add adapter.");
        return -ENODATA;
     }
   i2c_put_adapter(adapter);
   i2c_device = i2c_new_device(adapter, ds3231_rtc);
   printk(KERN_ALERT "Add new i2c new device");

   return 0;
}

static void __exit
_i2c_client_driver_exit(void)
{
   if (i2c_device)
     i2c_unregister_device(i2c_device);
}

module_init(_i2c_client_driver_init);
module_exit(_i2c_client_driver_exit);

MODULE_DESCRIPTION("i2c client driver.");
MODULE_AUTHOR("Amitesh Singh");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

