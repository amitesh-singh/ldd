### HID notes

1. To connect like hid input,
   call hid_hw_start(hdev, HID_CONNECT_HIDINPUT);  
   Better to use HID_CONNECT_DEFAULT instead. supports both hidinput and hidraw, nice.

### How to load custom hid driver

   usb_generic is a default driver which will detect your mouse/keyb or other hid type devices on hotplug,  
   just `rmmod usb_generic`  
   Other option is to add your device vendor and product id in ignore_list array of hid-core.c and recompile kernel.  
   Another option is to use already ignored vendor/product id from ignore_list[] into your device firmware.
