/* spi bitbang linux driver
 * Copyright (C) 2016 Amitesh Singh <singh.amitesh@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/fs.h>

//TODO:
// 1. pass pin values via arguments
// 2. give option to read too? check wiki
// 3. Remove unneccessary printk() After real hw testing

#define MOSI_PIN  432
#define MISO_PIN  -10
#define RST_PIN  433
#define SCK_PIN  434
#define LSB_ORDER 1


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh <singh.amitesh@gmail.com>");
MODULE_DESCRIPTION("a very fast spi bitbang");
MODULE_VERSION("0.1");

enum { MOSI = 0,
     SCK,
     RST,
};

enum BITORDER
{
   LSB = 0,
   MSB
};

static ssize_t get_value(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t set_value(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

static DEVICE_ATTR(value, 0660, get_value,
                   set_value);

struct spi_device_platform_data
{
   unsigned mosi, miso, sck, rst;
   struct gpio_desc *desc[3]; // MOSI (data), SCK(clock), RST (latch)?
   enum BITORDER bit_order;
   struct platform_device *pdev;
   int (*power_on)(struct spi_device_platform_data *, struct platform_device *pdev);
   void (*power_off)(struct spi_device_platform_data *,struct platform_device *pdev);
   void (*reset)(struct spi_device_platform_data *, struct platform_device *pdev);
   uint8_t (*tx)(struct spi_device_platform_data *, struct platform_device *, uint8_t val);
};

static int
_power_on(struct spi_device_platform_data *sd, struct platform_device *pdev)
{
   int result;
   uint8_t i = 0;

   printk(KERN_INFO "custom device is powered on");
   result = device_create_file(&pdev->dev,
                               &dev_attr_value);

   if (result < 0)
     {
        printk(KERN_ALERT "failed to create file");
        return -EINVAL;
     }
   else
     printk("successfully created file");

   //we could push this in a func()?
   for (; i < 3; ++i)
     {
        if (i == 0)
          {
             sd->desc[0] = gpio_to_desc(sd->mosi);
             if (!sd->desc[0])
               {
                  printk(KERN_ALERT "Failed to get desc");
                  return -EINVAL;
               }
             result = gpio_request(sd->mosi, "sysfs");
             if (result < 0)
               {
                  printk(KERN_ALERT "Failed to get the gpio request");
                  return -EINVAL;
               }
          }
        else if (i == 1)
          {
             sd->desc[1] = gpio_to_desc(sd->sck);
             if (!sd->desc[1])
               {
                  printk(KERN_ALERT "Failed to get desc");
                  return -EINVAL;
               }
             result = gpio_request(sd->sck, "sysfs");
             if (result < 0)
               {
                  printk(KERN_ALERT "Failed to get the gpio request");
                  return -EINVAL;
               }
          }
        else
          {
             sd->desc[2] = gpio_to_desc(sd->rst);
             if (!sd->desc[2])
               {
                  printk(KERN_ALERT "Failed to get desc");
                  return -EINVAL;
               }
             result = gpio_request(sd->rst, "sysfs");
             if (result < 0)
               {
                  printk(KERN_ALERT "Failed to get the gpio request");
                  return -EINVAL;
               }
          }
        result = gpiod_export(sd->desc[i], false);
        gpiod_direction_output(sd->desc[i], false);
     }

   return 0;
}

static void
_power_off(struct spi_device_platform_data *sd, struct platform_device *pdev)
{
   int i = 0;
   printk(KERN_INFO "spi device is powered off");
   for (; i < 3; ++i)
     {
        gpiod_unexport(sd->desc[i]);
     }
   gpio_free(sd->mosi);
   gpio_free(sd->sck);
   gpio_free(sd->rst);

   device_remove_file(&pdev->dev, &dev_attr_value);
}

//Not sure when to use.
static void
_reset(struct spi_device_platform_data *sd, struct platform_device *pdev)
{
   sd->power_off(sd, pdev);
   sd->power_on(sd, pdev);
   printk(KERN_INFO "spi device is reset");
}

static uint8_t
_tx(struct spi_device_platform_data *sd, struct platform_device *pdev, uint8_t val)
{
   uint8_t i = 0;
   printk(KERN_ALERT "Sending data: %d", val);
   // make latch pin to go LOW
   // write each bit of data to data pin based on bitOrder
   //     after each bit write, Pulse clock pin
   // make latch pin to go HIGH
   //

   // Latch pin - RST
   // data pin - MOSI
   // clock pin - SCK

//set latch pin low
   gpiod_set_value(sd->desc[RST], false);

   for (; i < 8; ++i)
     {
        if (sd->bit_order == LSB)
          {
             if (val & 0b00000001)
               gpiod_set_value(sd->desc[MOSI], true);
             else
               gpiod_set_value(sd->desc[MOSI], false);

             udelay(1);
             gpiod_set_value(sd->desc[SCK], true);
             udelay(1);
             gpiod_set_value(sd->desc[SCK], false);

             udelay(1);
             val = val >> 1;
          }
        else
          {
             if (val & 0b10000000)
               gpiod_set_value(sd->desc[MOSI], true);
             else
               gpiod_set_value(sd->desc[MOSI], false);
             udelay(1);
             gpiod_set_value(sd->desc[SCK], true);
             udelay(1);
             gpiod_set_value(sd->desc[SCK], false);
             udelay(1);
             val = val << 1;
          }
     }

   udelay(1);
   gpiod_set_value(sd->desc[RST], true);

   printk(KERN_ALERT " done with sending data: %d", val);
   //TODO: it is possible to return read value too
   // Check wiki..
   return val;
}

static struct spi_device_platform_data my_device_data = {
     .mosi      = MOSI_PIN,
     .miso      = MISO_PIN,
     .sck       = SCK_PIN,
     .rst       = RST_PIN,
     .bit_order = LSB_ORDER,
     .power_on  = _power_on,
     .power_off = _power_off,
     .reset     = _reset,
     .tx        = _tx,
};

static long old_value;

static ssize_t get_value(struct device *dev,
                         struct device_attribute *attr, char *buf)
{
   return sprintf(buf, "%ld", old_value);
}

static ssize_t set_value(struct device *dev,
                         struct device_attribute *attr,
                         const char *buf, size_t count)
{
   long new_value = 0;
   uint8_t val = 0;

   if (kstrtol(buf, 10, &new_value) < 0)
     {
        return -EINVAL;
     }
   val = new_value & 0xFF; // only 8 bit transfer

   //TODO: remove this hack..
   my_device_data.tx(&my_device_data, my_device_data.pdev, val);

   old_value = new_value;

   return count;
}

static void
_release(struct device *dev)
{
   printk("device.release()");
}

static struct platform_device my_device = {
     .name = "fast-spi-gpio",
     .id = -1, //let kernel decide
     .dev.platform_data = &my_device_data,
     .dev.release = _release,
};

static int
_spi_platform_driver_probe(struct platform_device *pdev)
{
   struct spi_device_platform_data *data;
   int result = 0;

   printk(KERN_INFO "SPI device connected/probed");

   data = dev_get_platdata(&pdev->dev);
   data->pdev = pdev;

   if (data->power_on && (result = data->power_on(data, pdev) < 0))
     {
        printk(KERN_ALERT "Failed to power on");
        return result;
     }
   else
     {
        printk(KERN_ALERT "SPI device is Powered on...");
     }

   return 0;
}

static int
_spi_platform_driver_remove(struct platform_device *pdev)
{
   struct spi_device_platform_data *data;

   data = dev_get_platdata(&pdev->dev);

   if (data->power_off) data->power_off(data, pdev);

   printk(KERN_INFO "platfrom device removed");
   return 0;
}

static struct platform_driver spi_platform_driver = {
     .probe = _spi_platform_driver_probe,
     .remove = _spi_platform_driver_remove,
     .driver = {
          .name = "fast-spi-gpio", //platform_device will also use same name
     },
};

static int __init
_spi_driver_init(void)
{
   printk(KERN_INFO "platform driver init");
   platform_driver_register(&spi_platform_driver);
   platform_device_register(&my_device);

   return 0;
}

static void __exit
_spi_driver_exit(void)
{
   printk(KERN_INFO "platform driver exit");
   platform_driver_unregister(&spi_platform_driver);
   platform_device_unregister(&my_device);
}

module_init(_spi_driver_init);
module_exit(_spi_driver_exit);
