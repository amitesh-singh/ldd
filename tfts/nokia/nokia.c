/*
 * Copyright (C) 2009, Alexander Kudjashev <Kudjashev@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of this archive for
 * more details.
 *
 * Layout is based on skeletonfb.c by James Simmons and Geert Uytterhoeven
 *
 * This driver was written to be used with nokia 6100 lcd
 * (philips pcf8833).
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/uaccess.h>


/*****************************************************************************
          Command of LCD NOKIA6100(phillips chipset)
*****************************************************************************/
#define NOP         0x00 // nop
#define SWRESET     0x01 // software reset
#define BSTROFF     0x02 // booster voltage OFF
#define BSTRON      0x03 // booster voltage ON
#define RDDIDIF     0x04 // read display identification
#define RDDST       0x09 // read display status
#define SLEEPIN     0x10 // sleep in
#define SLEEPOUT    0x11 // sleep out
#define PTLON       0x12 // partial display mode
#define NORON       0x13 // display normal mode
#define INVOFF      0x20 // inversion OFF
#define INVON       0x21 // inversion ON
#define DALO        0x22 // all pixel OFF
#define DAL         0x23 // all pixel ON
#define SETCON      0x25 // write contrast
#define DISPOFF     0x28 // display OFF
#define DISPON      0x29 // display ON
#define CASET       0x2A // column address set
#define PASET       0x2B // page address set
#define RAMWR       0x2C // memory write
#define RGBSET      0x2D // colour set
#define PTLAR       0x30 // partial area
#define VSCRDEF     0x33 // vertical scrolling definition
#define TEOFF       0x34 // test mode
#define TEON        0x35 // test mode
#define MADCTL      0x36 // memory access control
#define SEP         0x37 // vertical scrolling start address
#define IDMOFF      0x38 // idle mode OFF
#define IDMON       0x39 // idle mode ON
#define COLMOD      0x3A // interface pixel format
#define SETVOP      0xB0 // set Vop
#define BRS         0xB4 // bottom row swap
#define TRS         0xB6 // top row swap
#define DISCTR      0xB9 // display control
#define DOR         0xBA // data order
#define TCDFE       0xBD // enable/disable DF temperature compensation
#define TCVOPE      0xBF // enable/disable Vop temp comp
#define EC          0xC0 // internal or external oscillator
#define SETMUL      0xC2 // set multiplication factor
#define TCVOPAB     0xC3 // set TCVOP slopes A and B
#define TCVOPCD     0xC4 // set TCVOP slopes c and d
#define TCDF        0xC5 // set divider frequency
#define DF8COLOR    0xC6 // set divider frequency 8-color mode
#define SETBS       0xC7 // set bias system
#define RDTEMP      0xC8 // temperature read back
#define NLI         0xC9 // n-line inversion
#define RDID1       0xDA // read ID1
#define RDID2       0xDB // read ID2
#define RDID3       0xDC // read ID3

/* lcd resolution */
#define X_RES       130
#define Y_RES       130
#define B_PP        8
#define BLOCK_NUM   10

#define MEM_LEN     (X_RES * Y_RES * B_PP / 8)
#define BLOCK_LEN   (MEM_LEN / BLOCK_NUM)

#define SPI_SPEED   6 * 1000 * 1000;

struct pcf8833_par {
   struct spi_device   *spi;
   u8                  *buffer;
   u8                  *screen;
   u16                 rect[6];
   u16                 pref;
   u16                 post;
   struct spi_message  msg;
   struct spi_transfer xfer_rect;
   struct spi_transfer xfer[BLOCK_NUM][3];
   struct task_struct  *pcf8833_thread_task;
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
   .id             = "pcf8833",
   .type           = FB_TYPE_PACKED_PIXELS,
   .visual         = FB_VISUAL_TRUECOLOR,
   .xpanstep       = 0,
   .ypanstep       = 0,
   .ywrapstep      = 0,
   .line_length    = X_RES * B_PP / 8,
   .accel          = FB_ACCEL_NONE,
};

static struct fb_var_screeninfo pcf8833_var  = {
   .xres           = X_RES,
   .yres           = Y_RES,
   .xres_virtual   = X_RES,
   .yres_virtual   = Y_RES,
   .height         = -1,
   .width          = -1,
   .activate       = FB_ACTIVATE_NOW,
   .vmode          = FB_VMODE_NONINTERLACED,
   .bits_per_pixel = B_PP,
   .red            = { 5, 3, 0 },
   .green          = { 2, 3, 0 },
   .blue           = { 0, 2, 0 },
   .nonstd         = 0,
};

static int pcf8833_blank(int blank_mode, struct fb_info *info)
{
   struct pcf8833_par *par = info->par;

   switch (blank_mode) {
   case FB_BLANK_UNBLANK:
      spi_command(par->spi, DISPON);
      break;
   case FB_BLANK_NORMAL:
   case FB_BLANK_VSYNC_SUSPEND:
   case FB_BLANK_HSYNC_SUSPEND:
   case FB_BLANK_POWERDOWN:
      spi_command(par->spi, DISPOFF);
      break;
   default:
      return -EINVAL;
   }

   return 0;
}

static struct fb_ops pcf8833_ops = {
   .owner          = THIS_MODULE,
   .fb_read        = fb_sys_read,
   .fb_write       = fb_sys_write,
   .fb_fillrect    = sys_fillrect,
   .fb_copyarea    = sys_copyarea,
   .fb_imageblit   = sys_imageblit,
   .fb_blank       = pcf8833_blank,
};

static int lcd_init(struct spi_device *spi)
{
   int retval;

   spi->master->bus_num = 1;
   spi->chip_select = 0;
   spi->max_speed_hz = SPI_SPEED;
   spi->mode = SPI_MODE_0;
   spi->bits_per_word = 9;

   retval = spi_setup(spi);
   if(retval < 0)
      return retval;   

   /* Software Reset LCD */
    spi_command(spi, SWRESET);
   mdelay(10);

   spi_command(spi, SLEEPOUT);
   mdelay(10);

   /* RGB 8 bit */
   spi_command(spi, COLMOD);
   spi_data(spi, 0x02);
   /* MY MX V LAO RGB X X X */
   spi_command(spi, MADCTL);
   spi_data(spi, 0x00);
   /* set max contrast */
   spi_command(spi, SETCON);
   spi_data(spi, 0x3f);
   mdelay(10);

   spi_command(spi, DISPON);

   return 0;
}

static void message_init(struct pcf8833_par *par)
{
   int i;
   u16 *rect;
   struct spi_message *m;
   struct spi_transfer (*x)[3], *xr;

   rect = par->rect;
   m = &par->msg;                                                          
   xr = &par->xfer_rect;
   x = par->xfer;                                       

   spi_message_init(m);

   /* column start/end ram */
   rect[0] = CASET;
   rect[1] = 1|0x100;
   rect[2] = 130|0x100;
   /* page start/end ram */
   rect[3] = PASET;
   rect[4] = 1|0x100;
   rect[5] = 130|0x100;

   xr->tx_buf = rect;
   xr->len = 12;
   spi_message_add_tail(xr, m);
   
   par->pref = RAMWR;
   par->post = NOP;

   for(i = 0; i < BLOCK_NUM; i++) {
      x[i][0].tx_buf = &par->pref;
      x[i][0].len = 2;
      spi_message_add_tail(&x[i][0], m);

      x[i][1].tx_buf = &par->screen[i * BLOCK_LEN * 2];
      x[i][1].len = BLOCK_LEN * 2;
      spi_message_add_tail(&x[i][1], m);

      x[i][2].tx_buf = &par->post;
      x[i][2].len = 2;
      spi_message_add_tail(&x[i][2], m);
   }
}

static int pcf8833_thread(void *param)
{
   int i;
   struct pcf8833_par *par = (struct pcf8833_par *)param;

//   set_user_nice(current, 5); 

   while(!kthread_should_stop()) {
      for(i = 0; i < MEM_LEN; i++)
         par->screen[i*2] = par->buffer[i];

      spi_sync(par->spi, &par->msg);
   }

   return 0;
}

static int pcf8833_probe(struct spi_device *spi)
{
   struct fb_info *info;
   int retval;
   struct pcf8833_par *par;
   
   retval = lcd_init(spi);
   if(retval < 0)
      return retval;   

   retval = -ENOMEM;
      
   info = framebuffer_alloc(sizeof(struct pcf8833_par), &spi->dev);
   if(!info)
      return retval;

   info->screen_base = alloc_pages_exact(MEM_LEN, GFP_DMA | __GFP_ZERO);
   if(!info->screen_base)
      goto err;

   info->fbops = &pcf8833_ops;
   info->var = pcf8833_var;
   info->fix = pcf8833_fix;

   info->fix.smem_len = MEM_LEN;
   info->fix.smem_start = (unsigned long)virt_to_phys(info->screen_base);
   info->screen_size = info->fix.smem_len;
   info->flags = FBINFO_FLAG_DEFAULT;

   par = info->par;
   par->spi = spi;
   par->buffer = info->screen_base;

   par->screen = kmalloc(MEM_LEN * 2, GFP_KERNEL | GFP_DMA);
   if(!par->screen)
      goto err1;
   memset(par->screen, 0xff, MEM_LEN * 2);

   retval = register_framebuffer(info);
   if (retval < 0)
      goto err2;

   dev_set_drvdata(&spi->dev, info);

   message_init(par);
   par->pcf8833_thread_task = kthread_run(pcf8833_thread, par, "pcf8833");
   retval = -EIO;
   if(IS_ERR(par->pcf8833_thread_task))
      goto err2;

   printk(KERN_INFO "fb%d: %s frame buffer device, %dK of video memory\n",
          info->node, info->fix.id, info->fix.smem_len >> 10);

   return 0;

err2:
   kfree(par->screen);
err1:
   free_pages_exact(par->buffer, MEM_LEN);
err:
   framebuffer_release(info);

   return retval;
}

static int  pcf8833_remove(struct spi_device *spi)
{
   struct fb_info *info = dev_get_drvdata(&spi->dev);

   if (info) {
      struct pcf8833_par *par = info->par;

      unregister_framebuffer(info);
      free_pages_exact(info->screen_base, MEM_LEN);
      kfree(par->screen);
      kthread_stop(par->pcf8833_thread_task);
      framebuffer_release(info);
   }
   return 0;
}

static struct spi_driver pcf8833_driver = {
   .driver = {
      .name       = "pcf8833",
      .bus        = &spi_bus_type,
      .owner      = THIS_MODULE,
   },
   .probe      = pcf8833_probe,
   .remove     = (pcf8833_remove),
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
//http://www.at91.com/discussions/viewtopic.php/f,9/t,5103/
