//
// A basic spi communication example with st7735 TFT LCD 1.8".

#include <linux/init.h> // macros used to markup functions e.g. __init, __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/kernel.h> // Contains types, macros, functions for the kernel
#include <linux/spi/spi.h> //for spi
#include <linux/delay.h> //for mdelay, udelay
#include <linux/gpio.h> // for gpio_ apis.

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

/* lcd resolution */
#define X_RES 128
#define Y_RES 160
#define B_PP 8
#define MEM_LEN X_RES*Y_RES*B_PP/8

//slave device
static struct spi_device *spi;

#define RST_HIGH gpio_set_value(25, 1)
#define RST_LOW gpio_set_value(25, 0)

#define DC_HIGH gpio_set_value(24, 1)
#define DC_LOW gpio_set_value(24, 0)

static void spi_command(struct spi_device *spi, unsigned char c)
{
   DC_LOW;

   spi_write(spi, (u8 *)&c, 1);
   DC_HIGH;
}

static void spi_data(struct spi_device *spi, unsigned char c)
{
   DC_HIGH;

   spi_write(spi, (u8 *)&c, 1);
   DC_LOW;
}

void _writeWord(uint16_t word)
{
   spi_data(spi, (word >> 8));
   spi_data(spi, (word & 0xFF));
}

void _setAddrWindow(uint8_t x0, uint8_t y0,
                    uint8_t x1, uint8_t y1)
{
   spi_command(spi, ST7735_CASET);
   _writeWord(x0);
   _writeWord(x1);

   spi_command(spi, ST7735_RASET);
   _writeWord(y0);
   _writeWord(y1);

   spi_command(spi, ST7735_RAMWR);
}

void fillRec(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint32_t color)
{
   unsigned int i = 0;

   _setAddrWindow(x, y, x + w - 1, y + h -1);
   uint8_t r = color & 0xFF;
   uint8_t g = (color >> 8) & 0xFF ;
   uint8_t b = (color >> 16) & 0xFF ;

   for (; i < (w * h); ++i)
     {
        // _writeWord(color);
        spi_write(spi, &r, 1);
        spi_write(spi, &g, 1);
        spi_write(spi, &b, 1);
     }

   spi_command(spi, ST7735_NOP);
   //   CS_HIGH;
}

#define RST 25
#define DC 24

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
   mdelay(120);
   spi_command(spi, ST7735_SWRESET); // software reset
   mdelay(150);

   //CS_HIGH; //not using CS

   spi_command(spi, ST7735_SLPOUT);  // out of sleep mode
   //CS_HIGH;
   mdelay(120);

   spi_command(spi, ST7735_COLMOD);  // set color mode
   // 011 --> 12 bits/pixel
   // 101 --> 16 bits/pixel
   // 110 --> 18 bits/pixel
   spi_data(spi, 0x06);          // 16-bit color

   spi_command(spi, ST7735_FRMCTR1); // frame rate control
   spi_data(spi, 0x00);          // fastest refresh
   spi_data(spi, 0x06);          // 6 lines front porch
   spi_data(spi, 0x03);          // 3 lines backporch

   spi_command(spi, ST7735_MADCTL); 

#define _BV(x) (1 << x)
   spi_data( spi, _BV(3));

   spi_command(spi,ST7735_DISSET5); // display settings #5
   spi_data(spi,0x15);          // 1 clock cycle nonoverlap, 2 cycle gate rise, 3 cycle oscil. equalize
   spi_data(spi,0x02);          // fix on VTL


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
   spi_data(spi, 0x3f);
   mdelay(10);
   spi_command(spi, ST7735_DISPON);
   mdelay(120);

   return 0;
}

static int __init
_spi_init(void)
{
   int ret;
   struct spi_master *master;
   struct spi_board_info spi_device_info = {
        .modalias = "st7735",
        .max_speed_hz = 62000000, //speed of your device splace can handle
        .bus_num = 0, //BUS number
        .chip_select = 0,
        .mode = SPI_MODE_2,  //SPI mode 3, 2 and 0 works
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
   uint32_t color = 0xFF;
   fillRec(0, 0, 128, 160, color);
   //mdelay(1000);
   //fillRec(10, 100, 20, 10, GREEN);
   printk (KERN_ALERT "done writing to display");

   return 0;
}

static void __exit
_spi_exit(void)
{
   printk(KERN_INFO "spi basic driver exit");
   if (spi)
     {
        spi_unregister_device(spi);
     }
   //gpio_unexport(RST);
   // gpio_unexport(DC);
   gpio_free(RST);
   gpio_free(DC);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh <singh.amitesh@gmail.com>");
MODULE_DESCRIPTION("SPI basic driver");
MODULE_VERSION("0.1");

module_init(_spi_init);
module_exit(_spi_exit);
