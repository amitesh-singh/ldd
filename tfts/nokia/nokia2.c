/*
 * Copyright (C) 2008, Alexander Kudjashev <Kudjashev@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of this archive for
 * more details.
 *
 * Layout is based on skeletonfb.c by James Simmons and Geert Uytterhoeven
 * and arcfb.c by Jaya Kumar.
 *
 * This driver was written to be used with nokia 6100 lcd
 * (philips pcf8833 132*132@256 only).
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/uaccess.h>

/*****************************************************************************
          Command of LCD NOKIA6100(phillips chipset)
*****************************************************************************/
#define NOP       0x00 // nop
#define SWRESET    0x01 // software reset
#define BSTROFF    0x02 // booster voltage OFF
#define BSTRON       0x03 // booster voltage ON
#define RDDIDIF    0x04 // read display identification
#define RDDST       0x09 // read display status
#define SLEEPIN    0x10 // sleep in
#define SLEEPOUT    0x11 // sleep out
#define PTLON       0x12 // partial display mode
#define NORON       0x13 // display normal mode
#define INVOFF       0x20 // inversion OFF
#define INVON       0x21 // inversion ON
#define DALO       0x22 // all pixel OFF
#define DAL       0x23 // all pixel ON
#define SETCON       0x25 // write contrast
#define DISPOFF    0x28 // display OFF
#define DISPON       0x29 // display ON
#define CASET       0x2A // column address set
#define PASET       0x2B // page address set
#define RAMWR       0x2C // memory write
#define RGBSET       0x2D // colour set
#define PTLAR       0x30 // partial area
#define VSCRDEF    0x33 // vertical scrolling definition
#define TEOFF       0x34 // test mode
#define TEON       0x35 // test mode
#define MADCTL       0x36 // memory access control
#define SEP       0x37 // vertical scrolling start address
#define IDMOFF       0x38 // idle mode OFF
#define IDMON       0x39 // idle mode ON
#define COLMOD       0x3A // interface pixel format
#define SETVOP       0xB0 // set Vop
#define BRS       0xB4 // bottom row swap
#define TRS       0xB6 // top row swap
#define DISCTR       0xB9 // display control
#define DOR       0xBA // data order
#define TCDFE       0xBD // enable/disable DF temperature compensation
#define TCVOPE       0xBF // enable/disable Vop temp comp
#define EC          0xC0 // internal or external oscillator
#define SETMUL       0xC2 // set multiplication factor
#define TCVOPAB    0xC3 // set TCVOP slopes A and B
#define TCVOPCD    0xC4 // set TCVOP slopes c and d
#define TCDF       0xC5 // set divider frequency
#define DF8COLOR    0xC6 // set divider frequency 8-color mode
#define SETBS       0xC7 // set bias system
#define RDTEMP       0xC8 // temperature read back
#define NLI       0xC9 // n-line inversion
#define RDID1       0xDA // read ID1
#define RDID2       0xDB // read ID2
#define RDID3       0xDC // read ID3

/* lcd resolution */
#define X_RES 132
#define Y_RES 132
#define B_PP 8
#define MEM_LEN X_RES*Y_RES*B_PP/8

struct pcf8833 {
   struct fb_info       *info;
   struct spi_device    *spi;
   u16             *scr;
};

static void spi_command(struct spi_device *spi, unsigned int c)
{
   unsigned int w = c & ~0x0100;

   spi_write(spi, (u8 *)&w, 2);   
}

static void spi_data(struct spi_device *spi, unsigned int d)
{
   unsigned int w = d | 0x0100;
   
   spi_write(spi, (u8 *)&w, 2);   
}

static struct fb_fix_screeninfo pcf8833_fix  = {
   .id          = "pcf8833",
   .type          = FB_TYPE_PACKED_PIXELS,
   .visual       = FB_VISUAL_PSEUDOCOLOR,
   .xpanstep       = 0,
   .ypanstep       = 0,
   .ywrapstep       = 0,
   .line_length    = X_RES*B_PP/8,
   .smem_len       = MEM_LEN,
   .accel          = FB_ACCEL_NONE,
};

static struct fb_var_screeninfo pcf8833_var  = {
   .xres          = X_RES,
   .yres          = Y_RES,
   .xres_virtual    = X_RES,
   .yres_virtual    = Y_RES,
   .bits_per_pixel   = B_PP,
   .nonstd         = 0,
};

static void pcf8833_gram_update(struct pcf8833 *par,
                         unsigned int dx,
                         unsigned int dy,
                         unsigned int w,
                         unsigned int h)
{
   struct spi_device *spi = par->spi;
   unsigned int x, y, i;
   u16 *scr = par->scr;
   u8 *videomemory = par->info->screen_base;

   if (w && h) {
      i = 0;
      for(y = dy; y < dy+h; y++)
         for(x = dx; x < dx+w; x++)   {
            scr[i] = videomemory[y*X_RES + x] | 0x100; 
            i++;
         }

      spi_command(spi, CASET);   // column start/end ram (x)
      spi_data(spi, dx);            
      spi_data(spi, dx+w-1);          
      spi_command(spi, PASET);   // page start/end ram (y)
      spi_data(spi, dy);               
      spi_data(spi, dy+h-1); 
   
      spi_command(spi, RAMWR);   // write some stuff
      spi_write(spi, (u8 *)scr, i*2);
   }
}

static ssize_t pcf8833_read(struct fb_info *info,
                            char *buf, 
                            size_t count, 
                            loff_t * ppos)
{
   unsigned long p = *ppos;
   unsigned int fb_mem_len;

   fb_mem_len = MEM_LEN;

   if (p >= fb_mem_len)
      return 0;
   if (count >= fb_mem_len)
      count = fb_mem_len;
   if (count + p > fb_mem_len)
      count = fb_mem_len - p;

   if (count) {
      char *base_addr;

      base_addr = info->screen_base;
      count -= copy_to_user(buf, base_addr + p, count);

      if (!count)
         return -EFAULT;
      *ppos += count;
   }
   return count;
}

static ssize_t pcf8833_write(struct fb_info *info, 
                         const char *buf, 
                         size_t count, 
                         loff_t * ppos)
{
   struct pcf8833 *par = info->par;
   unsigned long p = *ppos;
   unsigned int fb_mem_len;
   int err;

   fb_mem_len = MEM_LEN;

   if (p > fb_mem_len)
      return -ENOSPC;
   if (count >= fb_mem_len)
      count = fb_mem_len;
   err = 0;
   if (count + p > fb_mem_len) {
      count = fb_mem_len - p;
      err = -ENOSPC;
   }

   if (count) {
      char *base_addr;

      base_addr = info->screen_base;
      count -= copy_from_user(base_addr + p, buf, count);
      *ppos += count;
      err = -EFAULT;
   }

   if (count) {
      pcf8833_gram_update(par, 0, p/X_RES, X_RES, (p+count)/X_RES-p/X_RES+1);
      return count;
   }
   return err;
}

static void pcf8833_fillrect(struct fb_info *info,
                             const struct fb_fillrect *rect)
{
   struct pcf8833 *par = info->par;

   sys_fillrect(info, rect);

   /* update the physical lcd */
   pcf8833_gram_update(par, rect->dx, rect->dy, rect->width, rect->height);
}

static void pcf8833_copyarea(struct fb_info *info,
                             const struct fb_copyarea *area)
{
   struct pcf8833 *par = info->par;

   sys_copyarea(info, area);

   /* update the physical lcd */
   pcf8833_gram_update(par, area->dx, area->dy, area->width, area->height);
}

static void pcf8833_imageblit(struct fb_info *info,
                              const struct fb_image *image)
{
   struct pcf8833 *par = info->par;

   sys_imageblit(info, image);

   /* update the physical lcd */
   pcf8833_gram_update(par, image->dx, image->dy, image->width, image->height);
}

static struct fb_ops pcf8833_ops = {
   .owner          = THIS_MODULE,
   .fb_read       = pcf8833_read,
   .fb_write       = pcf8833_write,
   .fb_fillrect    = pcf8833_fillrect,
   .fb_copyarea    = pcf8833_copyarea,
   .fb_imageblit    = pcf8833_imageblit,
};

static int  pcf8833_probe(struct spi_device *spi)
{
   struct fb_info *info;
   int retval;
   int videomemorysize;
   unsigned char *videomemory;
   struct pcf8833 *par;

   spi->master->bus_num = 1;
   spi->chip_select = 0;
/*   spi->max_speed_hz = 50 * 1000 * 1000; // ??  */
        spi->max_speed_hz = 20 * 1000 * 1000;

   spi->mode = SPI_MODE_0;
   spi->bits_per_word = 9;
   retval = spi_setup(spi);
   if (retval < 0)
      return retval;   
   /* Software Reset LCD */
    spi_command(spi, SWRESET);
   mdelay(10);

   spi_command(spi, SLEEPOUT);
   mdelay(10);

   spi_command(spi, COLMOD);
   spi_data(spi, 0x02); // RGB 8 bit

   spi_command(spi, MADCTL);
   spi_data(spi, 0x00); // MY MX V LAO RGB X X X

   spi_command(spi, SETCON);
   spi_data(spi, 0x3f);
   mdelay(10);

   spi_command(spi, DISPON);

   retval = -ENOMEM;
   videomemorysize = MEM_LEN;
   
   if (!(videomemory = vmalloc(videomemorysize)))
      return retval;

   memset(videomemory, 0, videomemorysize);

   info = framebuffer_alloc(sizeof(struct pcf8833), &spi->dev);
   if (!info)
      goto err;

   info->screen_base = (char __iomem *)videomemory;
   info->fbops = &pcf8833_ops;
   info->var = pcf8833_var;
   info->fix = pcf8833_fix;
   info->flags = FBINFO_FLAG_DEFAULT;

   par = info->par;
   par->info = info;
   par->spi = spi;
/*   par->scr = kzalloc(MEM_LEN*2, GFP_KERNEL);   */
        par->scr = kzalloc(MEM_LEN*2, GFP_KERNEL | GFP_DMA);                    
        if (!par->scr)                                                          
            goto err; 

   retval = register_framebuffer(info);
   if (retval < 0)
      goto err1;
   dev_set_drvdata(&spi->dev, info);

   printk(KERN_INFO "fb%d: %s frame buffer device, %dK of video memory\n",
          info->node, info->fix.id, videomemorysize >> 10);

   return 0;
err1:
   vfree(par->scr);
   framebuffer_release(info);
err:
   vfree(videomemory);
   return retval;
}

static int pcf8833_remove(struct spi_device *spi)
{
   struct fb_info *info = dev_get_drvdata(&spi->dev);

   if (info) {
      struct pcf8833 *par = info->par;

      unregister_framebuffer(info);
      vfree((void __force *)info->screen_base);
      vfree((void __force *)par->scr);
      framebuffer_release(info);
   }
   return 0;
}

static struct spi_driver pcf8833_driver = {
   .driver = {
      .name      = "pcf8833",
      .bus      = &spi_bus_type,
      .owner      = THIS_MODULE,
   },
   .probe      = pcf8833_probe,
   .remove      = (pcf8833_remove),
};

static int __init pcf8833_init(void)
{
   printk("pcf8833 spi fb driver\n");
   return spi_register_driver(&pcf8833_driver);
}

static void __exit pcf8833_exit(void)
{
   spi_unregister_driver(&pcf8833_driver);
}

module_init(pcf8833_init);
module_exit(pcf8833_exit);

MODULE_AUTHOR("Alexander Kudjashev");
MODULE_DESCRIPTION("pcf8833 spi fb driver");
MODULE_LICENSE("GPL");
