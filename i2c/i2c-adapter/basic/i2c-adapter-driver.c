#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#include <linux/slab.h>
#include <linux/i2c.h>

typedef struct _i2c_device_info
{
   struct i2c_adapter adapter;
   struct i2c_client *i2c_device;

} i2c_device_info;

static int
_i2c_master_xfer(struct i2c_adapter *adapter, struct i2c_msg *msgs, int num)
{
   i2c_device_info *data = adapter->algo_data;
   int i, len;
   struct i2c_msg *pmsg;

   for (i = 0; i < num; ++i)
     {
        //the first message (i = 0) is i2c io begin
        // the last message (i = num - 1) is i2c io end
        pmsg = &msgs[i];
        //slave address either in 7 or 10 bits.
        printk(KERN_ALERT "address: %.4x", pmsg->addr);
        printk(KERN_ALERT "message length: %d", pmsg->len); //pmsg->buf: msg data
        if (pmsg->len == 1)
          printk(KERN_ALERT "data: %.4x", pmsg->buf[0]);
     }

   return 0;
}

static u32
_i2c_func(struct i2c_adapter *adapter)
{
   //i2c_device_info *data = adapter->algo_data;

   /*
      The tiny_func function is small and tells the I2C core what types of I2C messages this algorithm
      can support. For this driver, we want to be able to support a few different I2C message types:

      static u32 tiny_func(struct i2c_adapter *adapter)
      {
      return I2C_FUNC_SMBUS_QUICK |
      I2C_FUNC_SMBUS_BYTE |
      I2C_FUNC_SMBUS_BYTE_DATA |
      I2C_FUNC_SMBUS_WORD_DATA |
      I2C_FUNC_SMBUS_BLOCK_DATA;
      }
    */

   // I2C_FUNC_I2C
   // you could do i2cdetect -F 8 to see what functionality has been enabled.
   return I2C_FUNC_I2C | I2C_FUNC_SMBUS_QUICK |
      I2C_FUNC_SMBUS_BYTE |
      I2C_FUNC_SMBUS_BYTE_DATA |
      I2C_FUNC_SMBUS_WORD_DATA |
      I2C_FUNC_SMBUS_BLOCK_DATA;
}

/*
 *
 * int (*smbus_xfer) (struct i2c_adapter *adap, u16 addr, unsigned short flags, char read_write, u8 command, int size, union i2c_smbus_data *data);:
 a function pointer to be set if this algorithm driver can do SMB bus accesses.
 Most PCI-based I2C bus drivers are able to do this, and they should set this function pointer. If it is set, 
 this function is called whenever an I2C chip driver wants to communicate with the chip device. If it is set to NULL,
 *  the master_xfer function is used instead.
 *
 */
static const struct i2c_algorithm i2c_transfer_algo =
{
   // a function pointer to be set if this algorithm driver can do I2C direct-level accesses.
   // If it is set, this function is called whenever an I2C chip driver wants to communicate with the chip device.
   // If it is set to NULL, the smbus_xfer function is used instead.

   .master_xfer = _i2c_master_xfer,
   /*
    * u32 (*functionality) (struct i2c_adapter *);:
    a function pointer called by the
    *  I2C core to determine what kind of reads and writes the I2C adapter driver can do.
    *
    */
   .functionality = _i2c_func,
};

static int
_i2c_platform_driver_probe(struct platform_device *pdev)
{
   i2c_device_info *data;

   printk(KERN_ALERT "i2c platform driver probe");

   data = kzalloc(sizeof(i2c_device_info), GFP_KERNEL);
   if (!data)
     {
        printk(KERN_ALERT "Failed to allocate data.");
        return -ENODEV;
     }

   // set to the value (THIS_MODULE) that allows the proper module reference counting.
   data->adapter.owner = THIS_MODULE;

   //the type of I2C class devices that this driver supports.
   // Usually this is set to the value I2C_ADAP_CLASS_SMBUS.
   //data->adapter.class = I2C_CLASS_HWMON;
   //data->adapter.class = I2C_CLASS ;
   //a pointer to the struct i2c_algorithm structure that describes the way data is transferred through this I2C bus controller.
   //
   data->adapter.algo = &i2c_transfer_algo;
   data->adapter.algo_data = data;
   /*
      If this parent pointer is not set up, the I2C adapter is positioned on the
      legacy bus and shows up in the sysfs tree at /sys/devices/legacy.
      Here is what happens to our example driver when it is registered:

      $ tree /sys/devices/legacy/
      /sys/devices/legacy/
      |-- detach_state
      |-- floppy0
      |   |-- detach_state
      |   `-- power
      |       `-- state
      |-- i2c-0
      |   |-- detach_state
      |   |-- name
      |   `-- power
      |       `-- state
      `-- power
      `-- state
    */
   /* set up sysfs linkage to our parent device */
   data->adapter.dev.parent = &pdev->dev;

   // set to a descriptive name of the I2C bus driver.
   // This value shows up in the sysfs filename associated with this I2C adapter.
   sprintf(data->adapter.name, "ami-i2c-device");

   //Attach to i2c layer
   i2c_add_adapter(&data->adapter);

   //set the data so that we can use in other callbacks
   platform_set_drvdata(pdev, data);

   return 0;
}

static int
_i2c_platform_driver_remove(struct platform_device *pdev)
{
   i2c_device_info *data = platform_get_drvdata(pdev);

   printk(KERN_ALERT "i2c platform driver remove");

   i2c_del_adapter(&data->adapter);
   kfree(data);

   return 0;
}

static struct platform_driver i2c_platform_driver =
{
   .probe = _i2c_platform_driver_probe,
   .remove = _i2c_platform_driver_remove,
   .driver = {
        .name = "ami-i2c-device",
        .owner = THIS_MODULE,
   },
};

static int __init
_i2c_module_init(void)
{
   platform_driver_register(&i2c_platform_driver);

   return 0;
}

static void  __exit
_i2c_module_exit(void)
{
   platform_driver_unregister(&i2c_platform_driver);
}

module_init(_i2c_module_init);
module_exit(_i2c_module_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Amitesh Singh <singh.amitesh@gmail.com>");
MODULE_DESCRIPTION("a sample platform driver for i2c protocol.");
