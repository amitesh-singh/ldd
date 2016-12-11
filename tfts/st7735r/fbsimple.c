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

#define ST7735_COLOR(red, green, blue)   ((unsigned int)( (( blue >> 3 ) << 11 ) | (( green >> 2 ) << 5  ) |  ( red  >> 3 )))

#define BLACK                       ST7735_COLOR(0x00, 0x00, 0x00)
#define WHITE                       ST7735_COLOR(0xFF, 0xFF, 0xFF)
#define RED                         ST7735_COLOR(0xFF, 0x00, 0x00)
#define GREEN                       ST7735_COLOR(0x00, 0xFF, 0x00)
#define BLUE                        ST7735_COLOR(0x00, 0x00, 0xFF)
#define YELLOW                      ST7735_COLOR(0xFF, 0xFF, 0x00)
#define MAGENTA                     ST7735_COLOR(0xFF, 0x00, 0xFF)
#define CYAN                        ST7735_COLOR(0x00, 0xFF, 0xFF)
#define GRAY                        ST7735_COLOR(0x80, 0x80, 0x40)
#define SILVER                      ST7735_COLOR(0xA0, 0xA0, 0x80)
#define GOLD                        ST7735_COLOR(0xA0, 0xA0, 0x40)
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

#define X_RES 128
#define Y_RES 160
#define BPP 16

#define MEM_LEN X_RES*Y_RES*BPP/8

#define RST 25
#define DC 24

#define CS_HIGH gpio_set_value(25, 1)
#define CS_LOW gpio_set_value(25, 0)

#define DC_HIGH gpio_set_value(DC, 1)
#define DC_LOW gpio_set_value(DC, 0)

#define RST_HIGH gpio_set_value(RST, 1)
#define RST_LOW gpio_set_value(RST, 0)


static void spi_command(struct spi_device *spi, unsigned int c)
{
   DC_LOW;

   spi_write(spi, (u8 *)&c, 1);
   DC_HIGH;
}

static void spi_data(struct spi_device *spi, unsigned int d)
{
   DC_HIGH;

   spi_write(spi, (u8 *)&d, 1);
   DC_LOW;
}

void _writeWord(struct spi_device *spi, uint16_t word)
{
   spi_data(spi, (word >> 8));
   spi_data(spi, (word & 0xFF));
}

void _setAddrWindow(struct spi_device *spi, uint8_t x0, uint8_t y0,
                    uint8_t x1, uint8_t y1)
{
   spi_command(spi, ST7735_CASET);
   _writeWord(spi, x0);
   _writeWord(spi, x1);

   spi_command(spi, ST7735_RASET);
   _writeWord(spi, y0);
   _writeWord(spi, y1);

   spi_command(spi, ST7735_RAMWR);
}


static void init_gpio(uint8_t gpio)
{
   int status;

   status = gpio_request(gpio, "sysfs");
   if (status < 0)
     {
        printk (KERN_ALERT "Failed in gpio request");
     }
   gpio_direction_output(gpio, 1);

   //The below api will make gpio chip to seen in sysfs /sys/class/gpio/gpio25
   //gpio_export(gpio);
}

static struct spi_device *spi;

//this is for testing if we initialize our display well.

void fillRec(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color)
{
   unsigned int i = 0;

   _setAddrWindow(spi, x, y, x + w - 1, y + h -1);

   for (; i < (w * h); ++i)
     {
        _writeWord(spi, color);
     }

   spi_command(spi, ST7735_NOP);
   //   CS_HIGH;
}

static int  _init_display(void)
{
   init_gpio(RST);
   init_gpio(DC);

   /*
      19 is MOSI, no need to explicitly export 19 since spi interface will do that for us.

      gpio_request(19, "sysfs");
      if (status < 0)
      {
      printk (KERN_ALERT "Failed to export 19");
      }
      gpio_direction_output(19, 1);
      gpio_export(19, false);
    */
   mdelay(100);

   RST_LOW;
   udelay(20);
   RST_HIGH;
   spi_command(spi, ST7735_SWRESET); // software reset
   mdelay(150);

   spi_command(spi, ST7735_SLPOUT);  // out of sleep mode
   //CS_HIGH;
   mdelay(500);

   spi_command(spi, 0xB1);
   spi_data(spi, 0x01);
   spi_data(spi, 0x2C);
   spi_data(spi, 0x2D);

   spi_command(spi, 0xB2);
   spi_data(spi, 0x01);
   spi_data(spi, 0x2C);
   spi_data(spi, 0x2D);

   spi_command(spi, 0xB3);
   spi_data(spi, 0x01);
   spi_data(spi, 0x2C);
   spi_data(spi, 0x2D);
   spi_data(spi, 0x01);
   spi_data(spi, 0x2C);
   spi_data(spi, 0x2D);

   spi_command(spi, 0xB4);
   spi_data(spi, 0x07);

   spi_command(spi, 0xC0);
   spi_data(spi, 0xA2);
   spi_data(spi, 0x02);
   spi_data(spi, 0x84);

   spi_command(spi, 0xC1);
   spi_data(spi, 0xC5);

   spi_command(spi, 0xC2);
   spi_data(spi, 0x0A);
   spi_data(spi, 0x00);

   spi_command(spi, 0xC3);
   spi_data(spi, 0x8A);
   spi_data(spi, 0x2A);

   spi_command(spi, 0xC4);
   spi_data(spi, 0x8A);
   spi_data(spi, 0xEE);

   spi_command(spi, 0xC5);
   spi_data(spi, 0x0E);

   spi_command(spi, 0x3A);
   spi_data(spi, 0x05);

   //GAMMA correction 1
   spi_command(spi, 0xE0);
   spi_data(spi, 0x0F);
   spi_data(spi, 0x1A);
   spi_data(spi, 0x0F);
   spi_data(spi, 0x18);
   spi_data(spi, 0x2F);
   spi_data(spi, 0x28);
   spi_data(spi, 0x20);
   spi_data(spi, 0x22);
   spi_data(spi, 0x1F);
   spi_data(spi, 0x1B);
   spi_data(spi, 0x23);
   spi_data(spi, 0x37);
   spi_data(spi, 0x00);
   spi_data(spi, 0x07);
   spi_data(spi, 0x02);
   spi_data(spi, 0x10);

   spi_command(spi, 0xE1);
   spi_data(spi, 0x0F);
   spi_data(spi, 0x1B);
   spi_data(spi, 0x0F);
   spi_data(spi, 0x17);
   spi_data(spi, 0x33);
   spi_data(spi, 0x2C);
   spi_data(spi, 0x29);
   spi_data(spi, 0x2E);
   spi_data(spi, 0x30);
   spi_data(spi, 0x30);
   spi_data(spi, 0x39);
   spi_data(spi, 0x3F);
   spi_data(spi, 0x00);
   spi_data(spi, 0x07);
   spi_data(spi, 0x03);
   spi_data(spi, 0x10);

   //Display
   spi_command(spi, 0x29);
   mdelay(100);
   spi_command(spi, 0x13);
   mdelay(10);

   return 0;
}

struct MyDevice
{
   struct fb_info  *info;
   //other datas
   u8 *vmem;
   u16 *ssbuf;
};

static void
_fb_device_release(struct device *dev)
{
   //fixes kernel warnings
}

static struct platform_device fb_device = {
     .name = "fb_device",
     .id = -1,
     //TODO:
     //.dev.platform_data = &fb_device_data,
     .dev.release = _fb_device_release
};

static struct fb_fix_screeninfo st7735_fix = {
     .id = "st7735r",
     .type = FB_TYPE_PACKED_PIXELS,
     .visual = FB_VISUAL_TRUECOLOR,
     .xpanstep = 0,
     .ypanstep = 0,
     .ywrapstep = 0,
     .line_length = X_RES * BPP / 8,
     .accel = FB_ACCEL_NONE,
};

static void
_update_display(struct MyDevice *sd)
{
   unsigned int i;
   u8 *mem = sd->info->screen_base;
   u16 *vmem16 = (u16 *) mem;
   u16 *ssbuf = sd->ssbuf;
   for (i = 0; i < MEM_LEN/2; ++i)
     {
        ssbuf[i] = swab16(vmem16[i]);
     }

   _setAddrWindow(spi, 0, 0, X_RES - 1, Y_RES - 1);
   spi_command(spi, ST7735_RAMWR);

   DC_HIGH;
   //for (i = 0; i < X_RES *  Y_RES * BPP/8/2; ++i)
    // {

        //spi_data(spi, (ssbuf[i] & 0xFF));
        //spi_data(spi, (ssbuf[i] >> 8));
    // }
  spi_write(spi, (u8 *)ssbuf, MEM_LEN);
   DC_LOW;
}

static ssize_t st7735_write(struct fb_info *info, const char __user *buf,
                            size_t count, loff_t *ppos)
{
   ssize_t res;
   res = fb_sys_write(info, buf, count, ppos);
   //_update_display(sd);
    schedule_delayed_work(&info->deferred_work, HZ/30);
   return res;
}

void st7735_fillrect(struct fb_info *info, const struct fb_fillrect *rect)
{
   sys_fillrect(info, rect);
   //_update_display(sd);
   schedule_delayed_work(&info->deferred_work, HZ/30);

}

void st7735_copyarea(struct fb_info *info, const struct fb_copyarea *area)
{
   sys_copyarea(info, area);
   //_update_display(sd);
   schedule_delayed_work(&info->deferred_work, HZ/30);

}

void st7735_imageblit(struct fb_info *info, const struct fb_image *image)
{
   sys_imageblit(info, image);
   //_update_display(sd);
   schedule_delayed_work(&info->deferred_work, HZ/30);
}

static struct fb_ops st7735_ops = {
     .owner		    = THIS_MODULE,
     .fb_read	    = fb_sys_read,
     .fb_write	    = st7735_write,
     .fb_fillrect	 = st7735_fillrect,
     .fb_copyarea	 = st7735_copyarea,
     .fb_imageblit = st7735_imageblit,
};

static struct fb_var_screeninfo st7735_var;
struct MyDevice *sdGlobal;

static void st7735_deferred_io(struct fb_info *info,
                               struct list_head *pagelist)
{
   struct MyDevice *sd = info->par;
   _update_display(sd);
}


static struct fb_deferred_io st7735_defio = {
     .delay      = HZ/30,
     .deferred_io   = st7735_deferred_io,
};

static int _fb_platform_driver_probe(struct platform_device *pdev)
{
   int ret;
   int vmemsize;
   int retval = 0;
   struct fb_info *info;
   //struct MyDevice *sd;
   u8 *vmem;

   vmemsize = MEM_LEN;

   vmem = vzalloc(vmemsize);

   info = framebuffer_alloc(sizeof(struct MyDevice), &pdev->dev);

   info->screen_buffer = vmem;
   info->fbops = &st7735_ops;
   info->fix = st7735_fix;
   info->fix.smem_len = vmemsize;
   info->var = st7735_var;
   info->var.xres = X_RES;
   info->var.yres = Y_RES;
   info->var.xres_virtual =   info->var.xres;
   info->var.yres_virtual =   info->var.yres;
   info->var.bits_per_pixel = BPP;
   info->var.nonstd =         1;

   info->var.red.offset = 11;
   info->var.red.length = 5;
   info->var.green.offset = 5;
   info->var.green.length = 6;
   info->var.blue.offset = 0;
   info->var.blue.length = 5;
   info->var.transp.offset = 0;
   info->var.transp.length = 0;
   info->flags = FBINFO_FLAG_DEFAULT | FBINFO_VIRTFB;
   info->fbdefio = &st7735_defio;
   fb_deferred_io_init(info);

   retval = register_framebuffer(info);
   if (retval < 0)
     {
        printk(KERN_ALERT "Failed to register framebuffer");
        return -ENODEV;
     }

   sdGlobal = info->par;
   sdGlobal->info = info;
   sdGlobal->vmem = vmem;
   sdGlobal->ssbuf = vzalloc(vmemsize);

   //spi init
   struct spi_master *master;
   struct spi_board_info spi_device_info = {
        .modalias = "st7735",
        .max_speed_hz = 62000000, //speed of your device splace can handle
        .bus_num = 0, //BUS number
        .chip_select = 0,
        .mode = SPI_MODE_0,  //SPI mode 3, 2 and 0 works
   };

   printk(KERN_INFO "spi basic driver init");

   master = spi_busnum_to_master(spi_device_info.bus_num);
   if (!master)
     {
        printk(KERN_ALERT "Failed to create master device");
        return -ENODEV;
     }
   //create a slave new device, given the master and device info
   spi = spi_new_device(master, &spi_device_info);
   if (!spi)
     {
        printk(KERN_ALERT "Failed to create slave device");
        return -ENODEV;
     }

   spi->bits_per_word = 8;

   ret = spi_setup(spi);
   if (ret)
     {
        printk(KERN_ALERT "Failed to setup slave");
        spi_unregister_device(spi);
        return -ENODEV;
     }

   mdelay(1000);
   printk (KERN_ALERT "init dispaly");
   _init_display();
   //fillRec(0, 0, 128, 160, GOLD);
   //mdelay(1000);
   //fillRec(10, 100, 20, 10, GREEN);

   return 0;
}

static int _fb_platform_driver_remove(struct platform_device *pdev)
{
   unregister_framebuffer(sdGlobal->info);
   fb_deferred_io_cleanup(sdGlobal->info);
   vfree(sdGlobal->vmem);
   vfree(sdGlobal->ssbuf);
   framebuffer_release(sdGlobal->info);
   if (spi)
     {
        spi_unregister_device(spi);
     }

   gpio_free(RST);
   gpio_free(DC);

   return 0;
}

static struct platform_driver fb_platform_driver = {
     .probe = _fb_platform_driver_probe,
     .remove = _fb_platform_driver_remove,
     .driver = { .name = "fb_device"
     },
};

static int __init
_fb_init(void)
{
   printk (KERN_ALERT "fb init");

   platform_driver_register(&fb_platform_driver);
   platform_device_register(&fb_device);

   return 0;
}

static void __exit _fb_exit(void)
{
   platform_device_unregister(&fb_device);
   platform_driver_unregister(&fb_platform_driver);
   printk(KERN_ALERT "fb exit");
}

module_init(_fb_init);
module_exit(_fb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
