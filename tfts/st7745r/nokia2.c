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
#include <linux/gpio.h>

/*****************************************************************************
          Command of LCD NOKIA6100(phillips chipset)
*****************************************************************************/
#define ST7735_NOP         (0x0)
#define ST7735_SWRESET     (0x01)
#define ST7735_SLPIN       (0x10)
#define ST7735_SLPOUT      (0x11)
#define ST7735_PTLON       (0x12)
#define ST7735_NORON       (0x13)
#define ST7735_INVOFF      (0x20)
#define ST7735_INVON       (0x21)
#define ST7735_GAMSET      (0x26)
#define ST7735_DISPOFF     (0x28)
#define ST7735_DISPON      (0x29)
#define ST7735_CASET       (0x2A)
#define ST7735_RASET       (0x2B)
#define ST7735_RAMWR       (0x2C)
#define ST7735_RAMRD       (0x2E)
#define ST7735_PTLAR       (0x30)
#define ST7735_TEOFF       (0x34)
#define ST7735_PEON        (0x35)
#define ST7735_MADCTL      (0x36)
#define ST7735_IDMOFF      (0x38)
#define ST7735_IDMON       (0x39)
#define ST7735_COLMOD      (0x3A)

#define ST7735_FRMCTR1     (0xB1)
#define ST7735_INVCTR      (0xB4)
#define ST7735_DISSET5     (0xB6)
#define ST7735_PWCTR1      (0xC0)
#define ST7735_PWCTR2      (0xC1)
#define ST7735_PWCTR3      (0xC2)
#define ST7735_VMCTR1      (0xC5)
#define ST7735_PWCTR6      (0xFC)
#define ST7735_GMCTRP1     (0xE0)
#define ST7735_GMCTRN1     (0xE1)
/* lcd resolution */
#define X_RES 128
#define Y_RES 160
#define B_PP 16
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
   .red.offset = 11,
   .red.offset = 11,
   .red.length = 5,
   .green.offset = 5,
   .green.length = 6,
   .blue.offset = 0,
   .blue.length = 5,
   .transp.offset = 0,
   .transp.length = 0,
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

        spi_command(spi, ST7735_CASET);   // column start/end ram (x)
        spi_data(spi, dx);            
        spi_data(spi, dx+w-1);          
        spi_command(spi, ST7735_RASET);   // page start/end ram (y)
        spi_data(spi, dy);               
        spi_data(spi, dy+h-1); 

        spi_command(spi, ST7735_RAMWR);   // write some stuff
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

#define CS_HIGH gpio_set_value(25, 1)
#define CS_LOW gpio_set_value(25, 0)

#define DC_HIGH gpio_set_value(24, 1)
#define DC_LOW gpio_set_value(24, 0)

#define RST_HIGH gpio_set_value(25, 1)
#define RST_LOW gpio_set_value(25, 0)

static int  pcf8833_probe(struct spi_device *spi)
{
   struct fb_info *info;
   int retval;
   int videomemorysize;
   unsigned char *videomemory;
   struct pcf8833 *par;

   printk(KERN_ALERT "probe called...");

   spi->master->bus_num = 0;
   spi->chip_select = 0;
   /*   spi->max_speed_hz = 50 * 1000 * 1000; // ??  */
   spi->max_speed_hz = 20 * 1000 * 1000;

   spi->mode = SPI_MODE_0;
   spi->bits_per_word = 8;
   retval = spi_setup(spi);
   if (retval < 0)
     return retval;   
   /* Software Reset LCD */
   gpio_request_one(25, GPIOF_OUT_INIT_HIGH,
                    "ST7735 Reset Pin");
   gpio_request_one(24, GPIOF_OUT_INIT_LOW,
                    "ST7735 Data/Command Pin");

   RST_LOW;
   udelay(20);
   RST_HIGH;
   mdelay(120);
   spi_command(spi, ST7735_SWRESET); // software reset
   mdelay(150);
   //CS_HIGH;

   spi_command(spi, ST7735_SLPOUT);  // out of sleep mode
   //CS_HIGH;
   mdelay(120);

   spi_command(spi, ST7735_COLMOD);  // set color mode
   // 011 --> 12 bits/pixel
   // 101 --> 16 bits/pixel
   // 110 --> 18 bits/pixel
   spi_data(spi, 16);          // 16-bit color

   spi_command(spi, ST7735_FRMCTR1); // frame rate control
   spi_data(spi, 0x00);          // fastest refresh
   spi_data(spi, 0x06);          // 6 lines front porch
   spi_data(spi, 0x03);          // 3 lines backporch

   spi_command(spi, ST7735_MADCTL);  // memory access control (directions)

#define _BV(x) (1 << x)
   // if (_isLandscape)
   spi_data(spi, _BV(7)| _BV(5) | _BV(3));
   // else
   // _writeData( _BV(3));

   spi_command(spi,ST7735_DISSET5); // display settings #5
   spi_data(spi,0x15);          // 1 clock cycle nonoverlap, 2 cycle gate rise, 3 cycle oscil. equalize
   spi_data(spi,0x02);          // fix on VTL

   /*
      _writeCmd(ST7735_INVCTR);  // display inversion control
      _writeData(0x0);           // line inversion
      ST7735_CS_HI;
    */


   spi_command(spi,ST7735_PWCTR1);  // power control
   spi_data(spi,0x02);          // GVDD = 4.7V
   spi_data(spi,0x70);          // 1.0uA

   spi_command(spi,ST7735_PWCTR2);  // power control
   spi_data(spi,0x05);          // VGH = 14.7V, VGL = -7.35V
   spi_data(spi,ST7735_PWCTR3);  // power control
   spi_data(spi,0x01);          // Opamp current small
   spi_data(spi,0x02);          // Boost frequency

   spi_command(spi,ST7735_VMCTR1);  // power control
   spi_data(spi,0x3C);          // VCOMH = 4V
   spi_data(spi,0x38);          // VCOML = -1.1V

   spi_command(spi,ST7735_PWCTR6);  // power control
   spi_data(spi,0x11);
   spi_data(spi,0x15);

   spi_command(spi,ST7735_GMCTRP1);
   spi_data(spi,0x09);
   spi_data(spi,0x16);
   spi_data(spi,0x09);
   spi_data(spi,0x20);
   spi_data(spi,0x21);
   spi_data(spi,0x1B);
   spi_data(spi,0x13);
   spi_data(spi,0x19);
   spi_data(spi,0x17);
   spi_data(spi,0x15);
   spi_data(spi,0x1E);
   spi_data(spi,0x2B);
   spi_data(spi,0x04);
   spi_data(spi,0x05);
   spi_data(spi,0x02);
   spi_data(spi,0x0E);
   spi_command(spi,ST7735_GMCTRN1);
   spi_data(spi,0x0B);
   spi_data(spi,0x14);
   spi_data(spi,0x08);
   spi_data(spi,0x1E);
   spi_data(spi,0x22);
   spi_data(spi,0x1D);
   spi_data(spi,0x18);
   spi_data(spi,0x1E);
   spi_data(spi,0x1B);
   spi_data(spi,0x1A);
   spi_data(spi,0x24);
   spi_data(spi,0x2B);
   mdelay(120);
   //spi_data(spi, 0x3f);
   //mdelay(10);

   //spi_command(spi, DISPON);

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
          //.bus      = &spi_bus_type,
          .owner      = THIS_MODULE,
     },
     .probe      = pcf8833_probe,
     .remove      = (pcf8833_remove),
};

static void
_platform_device_release(struct device *dev)
{
   printk(KERN_ALERT "device relase():");
}

static struct platform_device simple_device = {
     .name = "pcf8833",
     .id = -1,
     .dev = {
          .release = _platform_device_release
     }
};

struct spi_device *spi_device;
static int __init pcf8833_init(void)
{
   printk("pcf8833 spi fb driver\n");
   spi_register_driver(&pcf8833_driver);
   platform_device_register(&simple_device);

   struct spi_master *spi_master;
   struct device *pdev;

   spi_master = spi_busnum_to_master(0);
   spi_device = spi_alloc_device(spi_master);
   spi_device->chip_select = 0;
   spi_device->max_speed_hz = 20 * 1000 * 1000;
   spi_device->mode = SPI_MODE_0;
   spi_device->bits_per_word = 8;
   spi_device->irq = -1;
   spi_device->controller_state = NULL;
   spi_device->controller_data = NULL;

   strlcpy(spi_device->modalias, "pcf8833", 8);
   int status;
   status = spi_add_device(spi_device);

   if (status < 0)
     printk(KERN_ALERT "Failed to add device");

   return 0;
}

static void __exit pcf8833_exit(void)
{
   //spi_remove_device(spi_device);
   platform_device_unregister(&simple_device);
   spi_unregister_driver(&pcf8833_driver);
   spi_unregister_device(spi_device);
}

module_init(pcf8833_init);
module_exit(pcf8833_exit);

MODULE_AUTHOR("Alexander Kudjashev");
MODULE_DESCRIPTION("pcf8833 spi fb driver");
MODULE_LICENSE("GPL");
