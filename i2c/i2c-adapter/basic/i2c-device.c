/*
 * i2c-device.c
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

static void
_dev_release(struct device *dev)
{
   printk(KERN_ALERT "device release():");
}

static struct platform_device i2c_platform_device =
{
   .name = "ami-i2c-device",
   .id = -1,
   .dev.release = _dev_release,
};

static int __init
_i2c_module_init(void)
{
   platform_device_register(&i2c_platform_device);

   return 0;
}

static void  __exit
_i2c_module_exit(void)
{
   platform_device_unregister(&i2c_platform_device);
}

module_init(_i2c_module_init);
module_exit(_i2c_module_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Amitesh Singh <singh.amitesh@gmail.com>");
MODULE_DESCRIPTION("a sample platform driver for i2c protocol.");
