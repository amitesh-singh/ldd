/*
 * 4dtft.c
 *
 *  Created on: Dec 17, 2016
 *      Author: ami
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>

#include <linux/platform_device.h>
#include <linux/vmalloc.h>

#include "common.h"

#define RS 23
#define DC 23
#define CS 24
#define WR 10

#define RST 9
//data pins for parallel bus
#define D0 4
#define D1 14
#define D2 3
#define D3 15
#define D4 17
#define D5 18
#define D6 27
#define D7 22

#define RS_LOW  gpio_set_value(RS, 0)
#define RS_HIGH gpio_set_value(RS, 1)

#define WR_LOW gpio_set_value(WR, 0)
#define WR_HIGH gpio_set_value(WR, 1)

#define RST_HIGH gpio_set_value(RST, 1)
#define RST_LOW gpio_set_value(RST, 0)

#define CS_LOW gpio_set_value(CS, 0)
#define CS_HIGH  gpio_set_value(CS, 1)

struct tft4d_data
{
   uint8_t gpio[8];
};

static struct tft4d_data info = {{D0, D1, D2, D3, D4, D5, D6, D7}};



static void init_gpio(uint8_t gpio)
{
   int status;

   status = gpio_request(gpio, "sysfs");
   if (status < 0)
     {
        printk (KERN_ALERT "Failed in gpio request");
     }
   gpio_direction_output(gpio, 1);
}

static int _init_connection(struct tft_device_data *tdd)
{
   uint8_t i = 0;

   init_gpio(DC);
   init_gpio(WR);
   init_gpio(CS);
   init_gpio(RST);

   for (; i < 8; ++i)
     {
        init_gpio(info.gpio[i]);
     }
   return 0;
}

static void _shutdown_connection(struct tft_device_data *tdd)
{
   uint8_t i = 0;
   gpio_free(WR);
   gpio_free(CS);
   gpio_free(RST);
   gpio_free(DC);
   for (; i < 8; ++i)
     gpio_free(info.gpio[i]);
}

static void write_data(uint8_t data)
{
   uint8_t i = 0;

   for (; i < 8; ++i)
     {
        gpio_set_value(info.gpio[i], data & 0x01);
        data >>= 1;
     }
}

static void _send_command(struct tft_device_data *tdd, uint16_t command)
{

   RS_LOW;
   WR_LOW;

   write_data(command);

   WR_HIGH;
   RS_HIGH;
}

static void _send_data(struct tft_device_data *tdd, uint16_t data)
{
   RS_HIGH;
   WR_LOW;

   write_data(data);

   WR_HIGH;
   RS_LOW;
}

static void  _init_display(struct tft_device_data *tdd)
{
   //do reset few times
   RST_HIGH;
   mdelay(5);
   RST_LOW;
   mdelay(15);
   RST_HIGH;
   mdelay(15);

   CS_LOW;
   tdd->send_command(tdd,  0x11);
   mdelay(50);

   tdd->send_command(tdd,  0xD0);
   tdd->send_data(tdd, 0x07);
   tdd->send_data(tdd, 0x42);
   tdd->send_data(tdd, 0x18);

   tdd->send_command(tdd,  0xD1);
   tdd->send_data(tdd, 0x00);
   tdd->send_data(tdd, 0x47);
   tdd->send_data(tdd, 0x10);

   tdd->send_command(tdd,  0xD2);
   tdd->send_data(tdd, 0x01);
   tdd->send_data(tdd, 0x02);

   tdd->send_command(tdd,  0xC0);
   tdd->send_data(tdd, 0x10);

   tdd->send_data(tdd, 0x3B);
   tdd->send_data(tdd, 0x00);
   tdd->send_data(tdd, 0x02);
   tdd->send_data(tdd, 0x11);

   tdd->send_command(tdd,  0xC5);
   tdd->send_data(tdd, 0x03);

   tdd->send_command(tdd,  0x3A);
   tdd->send_data(tdd, 0x55);
   //Gamma
   tdd->send_command(tdd,  0xC8);
   tdd->send_data(tdd, 0x00);
   tdd->send_data(tdd, 0x32);
   tdd->send_data(tdd, 0x36);
   tdd->send_data(tdd, 0x45);
   tdd->send_data(tdd, 0x06);
   tdd->send_data(tdd, 0x16);
   tdd->send_data(tdd, 0x37);
   tdd->send_data(tdd, 0x75);
   tdd->send_data(tdd, 0x77);
   tdd->send_data(tdd, 0x54);
   tdd->send_data(tdd, 0x0C);
   tdd->send_data(tdd, 0x00);

   // disp on
   tdd->send_command(tdd,  0x29);

   CS_HIGH;
}

static void write_word(struct tft_device_data *tdd, uint16_t word)
{
   tdd->send_data(tdd, (word >> 8));
   tdd->send_data(tdd, (word & 0xFF));
}

static void _set_addr_window(struct tft_device_data *tdd, uint16_t x0, uint16_t y0,
                             uint16_t x1, uint16_t y1)
{
   tdd->send_command(tdd, 0x2A);
   write_word(tdd, x0);
   write_word(tdd, x1);

   tdd->send_command(tdd, 0x2B);
   write_word(tdd, y0);
   write_word(tdd, y1);

   tdd->send_command(tdd, 0x2C);
}

static void
_update_display(struct tft_device_data *tdd, uint8_t *mem, ssize_t size)
{
   unsigned i = 0;
   for (; i < size; i+=2)
     {
        tdd->send_data(tdd, mem[i + 1]);
        tdd->send_data(tdd, mem[i]);
     }
}

static struct tft_device_data tft4d_device =
{
   .width = 320,
   .height = 480,
   .framerate = HZ/30, //30 Frame/s
   .bpp = 16,
   .info = &info,
   .red_offset = 11,
   .red_length = 5,
   .green_offset = 5,
   .green_length = 6,
   .blue_offset = 0,
   .blue_length = 5,
   .init_connection = _init_connection,
   .send_command = _send_command,
   .send_data = _send_data,
   .init_display = _init_display,
   .shutdown_connection = _shutdown_connection,
   .set_addr_window = _set_addr_window,
   .update_display = _update_display,
};

static void
_device_release(struct device *dev)
{
   //fixes kernel warnings
}

static struct platform_device tft4d =
{
   .name = "fb_device",
   .id = -1,
   .dev.release = _device_release,
   .dev.platform_data = &tft4d_device,

};

static int __init _tft4d_init(void)
{
   return platform_device_register(&tft4d);
}

static void __exit _tft4d_exit(void)
{
   platform_device_unregister(&tft4d);
}

module_init(_tft4d_init);
module_exit(_tft4d_exit);

MODULE_AUTHOR("Amitesh Singh");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("driver for 4d dimension tft"); //ili9481 display.
