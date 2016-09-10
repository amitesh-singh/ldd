#define DRIVER_NAME  "myboard_spi2"
#define  SPI_MISO_GPIO  119
#define  SPI_MOSI_GPIO  120
#define  SPI_SCK_GPIO   121
#define  SPI_N_CHIPSEL  4
#include <linux/spi/spi.h>
#include <linux/spi/spi_bitbang.h>
#include <linux/spi/spi_gpio.h>

#include "spi-gpio.c"

