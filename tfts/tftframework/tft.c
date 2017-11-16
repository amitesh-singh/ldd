/*
 * tft.c
 *
 *  Created on: Dec 12, 2016
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
#include <linux/spi/spi.h>
#include <linux/gpio.h>

#include <linux/platform_device.h>
#include <linux/vmalloc.h> //for vzalloc

#include <linux/fb.h> //for framebuffer
#include "common.h"

struct fb_driver_data
{
   struct fb_info *info;
   u8 *vmem;
   u8 *ssbuf;
   struct tft_device_data *device_data;
};

static struct fb_fix_screeninfo _fb_fix = {
     .id = "st7735r",
     .type = FB_TYPE_PACKED_PIXELS,
     .visual = FB_VISUAL_TRUECOLOR,
     .xpanstep = 0,
     .ypanstep = 0,
     .ywrapstep = 0,
     //.line_length = X_RES * BPP / 8,
     .accel = FB_ACCEL_NONE,
};

static void
_update_display(struct fb_driver_data *sd)
{
   unsigned int i;

   u8 *mem = sd->info->screen_base;
   u8 *ssbuf = sd->ssbuf;

   for (i = 0; i < sd->info->fix.smem_len; ++i)
     {
        ssbuf[i] = (mem[i]);
     }

   //printk (KERN_ALERT "update display: length: %d", sd->info->fix.smem_len);
   sd->device_data->set_addr_window(sd->device_data, 0, 0,
		   	   	   	   	   	   	    sd->info->var.xres - 1, sd->info->var.yres - 1);
   sd->device_data->update_display(sd->device_data, ssbuf, sd->info->fix.smem_len);
}

static ssize_t _write(struct fb_info *info, const char __user *buf,
                      size_t count, loff_t *ppos)
{
   ssize_t res;

   res = fb_sys_write(info, buf, count, ppos);
   //_update_display(sd);
   schedule_delayed_work(&info->deferred_work, HZ/30);

   return res;
}

/*

struct fb_copyarea {
	__u32 dx;
	__u32 dy;
	__u32 width;
	__u32 height;
	__u32 sx;
	__u32 sy;
};

struct fb_fillrect {
	__u32 dx;	// screen-relative 
	__u32 dy;
	__u32 width;
	__u32 height;
	__u32 color;
	__u32 rop;
};

struct fb_image {
	__u32 dx;		// Where to place image
	__u32 dy;
	__u32 width;		// Size of image 
	__u32 height;
	__u32 fg_color;		// Only used when a mono bitmap
	__u32 bg_color;
  __u8  depth;		// Depth of the image 
  const char *data;	// Pointer to image data
	struct fb_cmap cmap;	// color map info 
};
*/

void _fillrect(struct fb_info *info, const struct fb_fillrect *rect)
{
   sys_fillrect(info, rect);
   //_update_display(sd);
   schedule_delayed_work(&info->deferred_work, HZ/30);

}

void _copyarea(struct fb_info *info, const struct fb_copyarea *area)
{
   sys_copyarea(info, area);
   //_update_display(sd);
   schedule_delayed_work(&info->deferred_work, HZ/30);
}

void _imageblit(struct fb_info *info, const struct fb_image *image)
{
   sys_imageblit(info, image);
   //_update_display(sd);
   schedule_delayed_work(&info->deferred_work, HZ/30);
}

static struct fb_ops _fb_ops = {
     .owner		    = THIS_MODULE,
     .fb_read	    = fb_sys_read,
     .fb_write	    = _write,
     .fb_fillrect	 = _fillrect,
     .fb_copyarea	 = _copyarea,
     .fb_imageblit = _imageblit,
};

static struct fb_driver_data *sdGlobal;

static void _deferred_io(struct fb_info *info,
                         struct list_head *pagelist)
{
   struct fb_driver_data *sd = info->par;
   _update_display(sd);
}

static struct fb_var_screeninfo _fb_var;

static struct fb_deferred_io _defio = {
     .delay      = HZ/30,
     .deferred_io   = _deferred_io,
};

static int _fb_driver_probe(struct platform_device *pdev)
{
   int retval;
   int vmemsize;
   struct fb_info *info;
   struct tft_device_data *device_data;


   info = framebuffer_alloc(sizeof(struct fb_driver_data), &pdev->dev);

   device_data = dev_get_platdata(&pdev->dev);
   sdGlobal = info->par;
   sdGlobal->device_data = device_data;
   vmemsize = sdGlobal->device_data->width * sdGlobal->device_data->height * sdGlobal->device_data->bpp /8;

   info->screen_buffer 		= 	vzalloc(vmemsize);
   info->fbops         		= 	&_fb_ops;
   info->fix           		= 	_fb_fix;
   info->fix.line_length    =   sdGlobal->device_data->width * sdGlobal->device_data->bpp / 8;
   info->fix.smem_len  		= 	vmemsize;
   info->var           		= 	_fb_var;
   info->var.xres      		= 	device_data->width;
   info->var.yres      		= 	device_data->height;
   info->var.xres_virtual 	=   info->var.xres;
   info->var.yres_virtual 	=   info->var.yres;
   info->var.bits_per_pixel = 	device_data->bpp;
   info->var.nonstd 		= 	1;

   info->var.red.offset = device_data->red_offset;
   info->var.red.length = device_data->red_length;
   info->var.green.offset = device_data->green_offset;
   info->var.green.length = device_data->green_length;
   info->var.blue.offset = device_data->blue_offset;
   info->var.blue.length = device_data->blue_length;
   info->var.transp.offset = 0;
   info->var.transp.length = 0;
   info->flags = FBINFO_FLAG_DEFAULT | FBINFO_VIRTFB;
   info->fbdefio = &_defio;
   fb_deferred_io_init(info);

   retval = register_framebuffer(info);
   if (retval < 0)
     {
        printk(KERN_ALERT "Failed to register framebuffer");
        return -ENODEV;
     }

   sdGlobal->info = info;
   sdGlobal->vmem = info->screen_base;
   sdGlobal->ssbuf = vzalloc(vmemsize);

   //init device
   if (device_data->init_connection(device_data) != 0)
     {
        printk (KERN_ALERT "Failed to init device.");

        return -ENODEV;
     }

   device_data->init_display(device_data);

   return 0;
}

static int _fb_driver_remove(struct platform_device *pdev)
{
   struct tft_device_data *device_data = dev_get_platdata(&pdev->dev);

   unregister_framebuffer(sdGlobal->info);
   fb_deferred_io_cleanup(sdGlobal->info);
   vfree(sdGlobal->vmem);
   vfree(sdGlobal->ssbuf);
   framebuffer_release(sdGlobal->info);

   device_data->shutdown_connection(device_data);

   return 0;
}

static struct platform_driver fb_driver = {
     .probe = _fb_driver_probe,
     .remove = _fb_driver_remove,
     .driver = { .name = "fb_device"
     },
};

static int __init _fb_init(void)
{
   return platform_driver_register(&fb_driver);
}

static void __exit _fb_exit(void)
{
   platform_driver_unregister(&fb_driver);
}

module_init(_fb_init);
module_exit(_fb_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("tft core driver");
