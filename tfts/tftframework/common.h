#ifndef __TFTS_COMMON_H
#define __TFTS_COMMON_H


struct tft_device_data
{
   uint16_t width, height;
   uint16_t framerate;
   uint8_t bpp;
   uint8_t red_length, red_offset;
   uint8_t green_length, green_offset;
   uint8_t blue_length, blue_offset;
   void *info;
   int (*init_connection)(struct tft_device_data *);
   void (*send_command)(struct tft_device_data *, uint16_t command);
   void (*send_data)(struct tft_device_data *, uint16_t data);
   void (*init_display)(struct tft_device_data *);
   void (*set_addr_window)(struct tft_device_data *, uint16_t x0, uint16_t y0,
			 uint16_t x1, uint16_t y1);
   void (*shutdown_connection)(struct tft_device_data *);
   void (*update_display)(struct tft_device_data *, uint8_t *mem, ssize_t size);
};

#endif
