#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/gpio.h>

#include <linux/platform_device.h>
#include <linux/vmalloc.h>

#include "common.h"

struct st7735r_info
{
};

static struct st7735r_info info;

static void _init_connection(struct tft_device_data *tdd)
{
}

static void _send_command(struct tft_device_data *tdd, uint16_t command)
{

}

static void _send_data(struct tft_device_data *tdd, uint16_t data)
{

}

static struct tft_device_data st7735r_device =
{
	.width = 128,
	.height = 160,
	.framerate = HZ/30, //30 Frame/s
	.bpp = 16,
	.info = &info,
	.init_connection = _init_connection,
	.send_command = _send_command,
	.send_data = _send_data,
	.init_display = _init_display,
	.shutdown_display = _shutdown_display,
	.set_addr_window = _set_addr_window
};

