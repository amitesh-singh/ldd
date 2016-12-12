#ifndef __TFTS_COMMON_H
#define __TFTS_COMMON_H


struct tft_device_data
{
   uint16_t width, height;
   uint16_t framerate;
   uint8_t bpp;
   void *info;
   void (*init_connection)(struct tft_device_data *);
   void (*send_command)(struct tft_device_data *, uint16_t command);
   void (*send_data)(struct tft_device_data *, uint16_t data);
   void (*init_display)(struct tft_device_data *, int8_t rst, int8_t dc);
   void (*set_addr_window)(struct tft_device_data *);
   void (*shutdown_display)(struct tft_device_data *);
   void (*update_display)(struct tft_device_data *)
};

#endif
