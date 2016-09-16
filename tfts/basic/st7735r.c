#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/spi/spi.h>
#include <linux/workqueue.h>


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

static struct spi_board_info spi_device_info = {
     .modalias = "st7735r",
     .max_speed_hz = 60000000,
     .bus_num = 0,
     .chip_select = 0,
     .mode = SPI_MODE_0
};

static struct spi_master *master;
static struct spi_device *spi;

#define RST 25
#define DC 24

#define RST_HIGH gpio_set_value(RST, 1)
#define RST_LOW gpio_set_value(RST, 0)

#define DC_HIGH gpio_set_value(DC, 1)
#define DC_LOW gpio_set_value(DC, 0)

static void spi_command(uint8_t c)
{
   DC_LOW;

   spi_write(spi, (u8 *)&c, 1);
   DC_HIGH;
}

static void spi_data(uint8_t c)
{
   DC_HIGH;
   spi_write(spi, &c, 1);
   DC_LOW;
}

static void write_word(uint16_t word)
{
   spi_data((word >> 8));
   spi_data(word & 0xFF);
}

static void
set_addr_window(uint8_t x0, uint8_t y0,
                uint8_t x1, uint8_t y1)
{
   spi_command(ST7735_CASET);
   write_word(x0);
   write_word(x1);

   spi_command(ST7735_RASET);
   write_word(y0);
   write_word(y1);

   spi_command(ST7735_RAMWR);
}

static void
init_gpio(uint8_t gpio, uint8_t dir)
{
   int status;

   status = gpio_request(gpio, "sysfs");
   if (status < 0)
     {
        printk (KERN_ALERT "Failed in gpio request");
     }
   gpio_direction_output(gpio, dir);

   //gpio_export(gpio);
}

static void
init_display(void)
{
   //initialize gpio pins
   init_gpio(RST, 1);
   init_gpio(DC, 1);

   //! --- Display init sequence
   RST_LOW;
   udelay(20);
   RST_HIGH;
   mdelay(120);

   spi_command(ST7735_SWRESET);
   mdelay(150);

   spi_command(ST7735_SLPOUT);
   mdelay(120);

   spi_command(ST7735_COLMOD);
   // 011 --> 12 bits/pixel
   // 101 --> 16 bits/pixel
   // 110 --> 18 bits/pixel
   spi_data(0x05);          // 16-bit color

   spi_command(ST7735_FRMCTR1); // frame rate control
   spi_data(0x00);          // fastest refresh
   spi_data(0x06);          // 6 lines front porch
   spi_data(0x03);          // 3 lines backporch

   spi_command(ST7735_MADCTL);

#define _BV(x) (1 << x)
   spi_data(_BV(3));

   spi_command(ST7735_DISSET5); // display settings #5
   spi_data(0x15);          // 1 clock cycle nonoverlap, 2 cycle gate rise, 3 cycle oscil. equalize
   spi_data(0x02);   

   spi_command(ST7735_PWCTR1);  // power control
   spi_data(0x02);          // GVDD = 4.7V
   spi_data(0x70);          // 1.0uA

   spi_command(ST7735_PWCTR2);  // power control
   spi_data(0x05);          // VGH = 14.7V, VGL = -7.35V
   spi_data(ST7735_PWCTR3);  // power control
   spi_data(0x01);          // Opamp current small
   spi_data(0x02);          // Boost frequency

   spi_command(ST7735_VMCTR1);  // power control
   spi_data(0x3C);          // VCOMH = 4V
   spi_data(0x38);          // VCOML = -1.1V

   spi_command(ST7735_PWCTR6);  // power control
   spi_data(0x11);
   spi_data(0x15);

   spi_command(ST7735_GMCTRP1);

   spi_data(0x09);
   spi_data(0x16);
   spi_data(0x09);
   spi_data(0x20);
   spi_data(0x21);
   spi_data(0x1B);
   spi_data(0x13);
   spi_data(0x19);
   spi_data(0x17);
   spi_data(0x15);
   spi_data(0x1E);
   spi_data(0x2B);
   spi_data(0x04);
   spi_data(0x05);
   spi_data(0x02);
   spi_data(0x0E);

   spi_command(ST7735_GMCTRN1);
   spi_data(0x0B);
   spi_data(0x14);
   spi_data(0x08);
   spi_data(0x1E);
   spi_data(0x22);
   spi_data(0x1D);
   spi_data(0x18);
   spi_data(0x1E);
   spi_data(0x1B);
   spi_data(0x1A);
   spi_data(0x24);
   spi_data(0x2B);
   mdelay(120);
   spi_data(0x3f);
   mdelay(10);
   spi_command(ST7735_DISPON);
   mdelay(120);
   //! --- display exit.
}

static void
update_screen(unsigned int x, unsigned int y, unsigned int w,
              unsigned int h)
{
   set_addr_window(x, y, w, h);
   for (int i = 0; i < w * h; ++i)
     {
        spi_data(color);
     }
}

static void
fill_rec(uint8_t x, uint8_t y,
         uint8_t w, uint8_t h, uint16_t color)
{
   uint16_t i = 0;

   set_addr_window(x, y, x + w - 1, y + h - 1);

   for (; i < (w * h); ++i)
     {
        write_word(color);
     }

   spi_command(ST7735_NOP);
}

#define WIDTH 128
#define HEIGHT 160

struct fb_info *info;
static int __init
_st7735r_init(void)
{
   int ret;

   printk(KERN_INFO "driver init");

   //Get master
   master = spi_busnum_to_master(spi_device_info.bus_num);
   if (!master)
     {
        printk(KERN_ALERT "Failed to create master device:");
        return -ENODEV;
     }

   //create slave device.
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
        printk(KERN_ALERT "Failed to setup slave device");
        spi_unregister_device(spi);
        return -ENODEV;
     }

   init_display();
   fill_rec(10, 10, 100, 60, 255);


   //! -- framebuffer init
   struct fb_ops *fbops;
   struct fb_deferred_io *fbdefio;
   u8 *vmem;

   vmem = vzalloc(WIDTH * HEIGHT * 8/8);
   if (!vmem)
     {
     }
   fbops = devm_kzalloc(spi->dev, sizeof(struct fb_ops), GFP_KERNEL);
   if (!fbops)
     {
     }

   fbdefio = devm_kzalloc(spi->dev, sizeof(struct fb_deferred_io), GFP_KERNEL);
   if (!fbdefio)
     {
     }

   info = framebuffer_alloc(0, spi->dev);
   if(!info)
     {
     }
   info->screen_buffer = vmem;
   info->fbops = fbops;
   info->fbdefio = fbdefio;

   fbops->fb_read      =      fb_sys_read;
   fbops->fb_write     =      fbtft_fb_write;
   fbops->fb_fillrect  =      fbtft_fb_fillrect;
   fbops->fb_copyarea  =      fbtft_fb_copyarea;
   fbops->fb_imageblit =      fbtft_fb_imageblit;
   fbops->fb_setcolreg =      fbtft_fb_setcolreg;
   fbops->fb_blank     =      fbtft_fb_blank;

   fbdefio->delay =           HZ/50;
   fbdefio->deferred_io =     fbtft_deferred_io;
   fb_deferred_io_init(info);

   info->var.xres = WIDTH;
   info->var.yres = HEIGHT;

   info->var.bits_per_pixel = 8;
   info->var.red.offset =     11;
   info->var.red.length =     5;
   info->var.green.offset =   5;
   info->var.green.length =   6;
   info->var.blue.offset =    0;
   info->var.blue.length =    5;

   ret = register_framebuffer(info);
   if (ret <0)
     {
        printk (KERN_ALERT "Failed to register framebuffer");
        return -ENODEV;
     }

   return 0;
}

static void __exit
_st7735r_exit(void)
{
   fb_deferred_io_cleanup(info);
   vfree(info->screen_buffer);
   frame_release(info);

   if (spi)
     spi_unregister_device(spi);

   gpio_free(RST);
   gpio_free(DC);
}

module_init(_st7735r_init);
module_exit(_st7735r_exit);

MODULE_AUTHOR("Amitesh Singh");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("FB driver for st7735 1.8 chinese display");
