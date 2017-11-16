/*
 * i2c-client-driver.c
 *
 *  Created on: Jan 5, 2017
 *      Author: ami
 */


#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#include <linux/slab.h>
#include <linux/i2c.h>

//this is a I2C module connected to our i2c bus somewhere with address
// 0x50
// we could add multiple entries here if other i2c devices are connected
// to GPIO12 i2c bus
/*
static struct i2c_board_info ds3231_rtc[] = {
       { I2C_BOARD_INFO("dts-3231", 0x68), },
};
*/

struct i2c_client *i2c_device;
struct i2c_adapter *adapter;

static int
_i2c_driver_probe(struct i2c_client *client,
                  const struct i2c_device_id *id)
{
   printk(KERN_ALERT "i2c client driver probe");

   return 0;
}

static int
_i2c_driver_remove(struct i2c_client *client)
{
   printk(KERN_ALERT "i2c client driver  remove");

   return 0;
}

static const struct i2c_device_id _gpio12_id[] =
{
     {"ami-i2c-device", 0},
     {}
};

MODULE_DEVICE_TABLE(i2c, _gpio12_id);

static struct i2c_driver _i2c_driver =
{
   .probe = _i2c_driver_probe,
   .remove = _i2c_driver_remove,
   .driver = {
        .name = "ami-i2c-device",

   },
   .id_table = _gpio12_id,
};

//module_i2c_driver(_i2c_driver);

static int __init
_i2c_client_driver_init(void)
{
   i2c_add_driver(&_i2c_driver);
   printk(KERN_ALERT "i2c lcient driver init");

   //adapter =  i2c_get_adapter(8);
   /*
   if (!adapter)
     {
        printk(KERN_ALERT "Failed to add adapter.");
        return -ENODATA;
     }
     */
   //i2c_put_adapter(adapter);
   //i2c_device = i2c_new_device(adapter, ds3231_rtc);

   return 0;
}

static void __exit
_i2c_client_driver_exit(void)
{
   if (i2c_device)
     i2c_unregister_device(i2c_device);
   i2c_del_driver(&_i2c_driver);
}

module_init(_i2c_client_driver_init);
module_exit(_i2c_client_driver_exit);

MODULE_DESCRIPTION("i2c client driver.");
MODULE_AUTHOR("Amitesh Singh");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

