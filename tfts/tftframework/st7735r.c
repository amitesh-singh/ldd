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

#define RST 25
#define DC 24
#define DC_HIGH gpio_set_value(DC, 1)
#define DC_LOW gpio_set_value(DC, 0)

#define RST_HIGH gpio_set_value(RST, 1)
#define RST_LOW gpio_set_value(RST, 0)

struct st7735r_info
{
   struct spi_device *spi;
};

static struct st7735r_info info;

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

static int _init_connection(struct tft_device_data *tdd)
{
   struct st7735r_info *info;
   int ret;
   struct spi_device *spi;
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

   info = tdd->info;
   info->spi = spi;

   return 0;
}

static void _send_command(struct tft_device_data *tdd, uint16_t command)
{
   uint8_t c = (command & 0xFF);
   struct spi_device *spi = ((struct st7735r_info *)(tdd->info))->spi;

   DC_LOW;
   spi_write(spi, (u8 *)&c, 1);
   DC_HIGH;
}

static void _send_data(struct tft_device_data *tdd, uint16_t data)
{
   uint8_t c = (data & 0xFF);
   struct spi_device *spi = ((struct st7735r_info *)(tdd->info))->spi;

   DC_HIGH;
   spi_write(spi, (u8 *)&c, 1);
   DC_LOW;
}

static void  _init_display(struct tft_device_data *tdd)
{
   //struct spi_device *spi = ((struct st7735r_info *)(tdd->info))->spi;
   init_gpio(RST);
   init_gpio(DC);

   mdelay(100);

   RST_LOW;
   udelay(20);
   RST_HIGH;
   tdd->send_command(tdd, 0x01); // software reset
   mdelay(150);

   tdd->send_command(tdd, 0x11);  // out of sleep mode
   //CS_HIGH;
   mdelay(500);

   tdd->send_command(tdd,  0xB1);
   tdd->send_data(tdd, 0x01);
   tdd->send_data(tdd, 0x2C);
   tdd->send_data(tdd, 0x2D);

   tdd->send_command(tdd, 0xB2);
   tdd->send_data(tdd, 0x01);
   tdd->send_data(tdd, 0x2C);
   tdd->send_data(tdd, 0x2D);

   tdd->send_command(tdd, 0xB3);
   tdd->send_data(tdd, 0x01);
   tdd->send_data(tdd, 0x2C);
   tdd->send_data(tdd, 0x2D);
   tdd->send_data(tdd, 0x01);
   tdd->send_data(tdd, 0x2C);
   tdd->send_data(tdd, 0x2D);

   tdd->send_command(tdd, 0xB4);
   tdd->send_data(tdd, 0x07);

   tdd->send_command(tdd, 0xC0);
   tdd->send_data(tdd, 0xA2);
   tdd->send_data(tdd, 0x02);
   tdd->send_data(tdd, 0x84);

   tdd->send_command(tdd, 0xC1);
   tdd->send_data(tdd, 0xC5);

   tdd->send_command(tdd, 0xC2);
   tdd->send_data(tdd, 0x0A);
   tdd->send_data(tdd, 0x00);

   tdd->send_command(tdd, 0xC3);
   tdd->send_data(tdd, 0x8A);
   tdd->send_data(tdd, 0x2A);

   tdd->send_command(tdd, 0xC4);
   tdd->send_data(tdd, 0x8A);
   tdd->send_data(tdd, 0xEE);

   tdd->send_command(tdd, 0xC5);
   tdd->send_data(tdd, 0x0E);

   tdd->send_command(tdd, 0x3A);
   tdd->send_data(tdd, 0x05);

   //GAMMA correction 1
   tdd->send_command(tdd, 0xE0);
   tdd->send_data(tdd, 0x0F);
   tdd->send_data(tdd, 0x1A);
   tdd->send_data(tdd, 0x0F);
   tdd->send_data(tdd, 0x18);
   tdd->send_data(tdd, 0x2F);
   tdd->send_data(tdd, 0x28);
   tdd->send_data(tdd, 0x20);
   tdd->send_data(tdd, 0x22);
   tdd->send_data(tdd, 0x1F);
   tdd->send_data(tdd, 0x1B);
   tdd->send_data(tdd, 0x23);
   tdd->send_data(tdd, 0x37);
   tdd->send_data(tdd, 0x00);
   tdd->send_data(tdd, 0x07);
   tdd->send_data(tdd, 0x02);
   tdd->send_data(tdd, 0x10);

   tdd->send_command(tdd, 0xE1);
   tdd->send_data(tdd, 0x0F);
   tdd->send_data(tdd, 0x1B);
   tdd->send_data(tdd, 0x0F);
   tdd->send_data(tdd, 0x17);
   tdd->send_data(tdd, 0x33);
   tdd->send_data(tdd, 0x2C);
   tdd->send_data(tdd, 0x29);
   tdd->send_data(tdd, 0x2E);
   tdd->send_data(tdd, 0x30);
   tdd->send_data(tdd, 0x30);
   tdd->send_data(tdd, 0x39);
   tdd->send_data(tdd, 0x3F);
   tdd->send_data(tdd, 0x00);
   tdd->send_data(tdd, 0x07);
   tdd->send_data(tdd, 0x03);
   tdd->send_data(tdd, 0x10);

   //Display
   tdd->send_command(tdd, 0x29);
   mdelay(100);
   tdd->send_command(tdd, 0x13);
   mdelay(10);
}

static void _shutdown_connection(struct tft_device_data *tdd)
{
   struct spi_device *spi = ((struct st7735r_info *)(tdd->info))->spi;

   if (spi)
     spi_unregister_device(spi);
   gpio_free(RST);
   gpio_free(DC);
}

static void write_word(struct tft_device_data *tdd, uint16_t word)
{
   tdd->send_data(tdd, (word >> 8));
   tdd->send_data(tdd, (word & 0xFF));
}

static void _set_addr_window(struct tft_device_data *tdd, uint16_t x0, uint16_t y0,
							 uint16_t x1, uint16_t y1)
{
   //struct spi_device *spi = ((struct st7735r_info *)(tdd->info))->spi;
   tdd->send_command(tdd, ST7735_CASET);
   write_word(tdd, x0);
   write_word(tdd, x1);

   tdd->send_command(tdd, ST7735_RASET);
   write_word(tdd, y0);
   write_word(tdd, y1);

   tdd->send_command(tdd, ST7735_RAMWR);
}

static void
_update_display(struct tft_device_data *tdd, uint8_t *mem, ssize_t size)
{
   struct spi_device *spi = ((struct st7735r_info *)(tdd->info))->spi;
   unsigned i = 0;

   DC_HIGH;
   for (; i < size; i+=2)
     {
        spi_write(spi, &mem[i+1], 1);
        spi_write(spi, &mem[i], 1);
     }
   DC_LOW;
}

static struct tft_device_data st7735r_device =
{
   .width = 128,
   .height = 160,
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

static struct platform_device st7735r = {
     .name = "fb_device",
     .id = -1,
     .dev.release = _device_release,
     .dev.platform_data = &st7735r_device,
};


static int __init _st7735r_device_init(void)
{
   return platform_device_register(&st7735r);
}

static void __exit _st7735r_device_exit(void)
{
   platform_device_unregister(&st7735r);
}

module_init(_st7735r_device_init);
module_exit(_st7735r_device_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("1.8 inch st7735r tft display driver");
