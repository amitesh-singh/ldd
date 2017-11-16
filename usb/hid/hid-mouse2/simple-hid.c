#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/usb.h> //for usb
#include <linux/hid.h> //for usb hid

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("hid driver example");
MODULE_VERSION("0.1");

//I tried with 0x160c (vendor) but it never worked.
// the reason which i think is because 0x160c is not added
// into the ignore list. using apple vendor and device id
// and it seems to work fine. :) - lesson learned
#define USB_VENDOR_ID 0x05ac
#define USB_DEVICE_ID 0x0304

static const struct hid_device_id avr_devices[] = {
       { HID_USB_DEVICE(USB_VENDOR_ID, USB_DEVICE_ID),
          .driver_data = 0 },
       { }
};

MODULE_DEVICE_TABLE(hid, avr_devices);

static int
avr_raw_event(struct hid_device *hd, struct hid_report *report,
              u8 *data, int size)
{
   printk("avr raw event\n");
   return 0;
}

static int
avr_probe(struct hid_device *hd,
          const struct hid_device_id *id)
{
   int ret;
   printk(KERN_ALERT "avr probe");

   ret = hid_parse(hd);
   if (ret)
     {
        printk("Failed to parse hid");
        return ret;
     }

   //ret = hid_hw_start(hd, HID_CONNECT_HIDRAW);
   //this will make hid device to act like a hid mouse
   // tested with avr random mouse
   //HID_CONNECT_DEFAULT is better option than
   // HID_CONNECT_HIDINPUT only
   ret = hid_hw_start(hd, HID_CONNECT_DEFAULT);
   if (ret)
     {
        printk("Failed to parse hid");
        return ret;
     }


   return 0;
}

static void
avr_remove(struct hid_device *hd)
{
   //call hid_hw_close(hdev) if hid_hw_open() is called in probe()
   printk("avr remove\n");
   //hid_hw_close(hd);
   hid_hw_stop(hd);
}

static int
avr_event(struct hid_device *hd, struct hid_field *hf,
          struct hid_usage *hu, __s32 value)
{
   input_event(hf->hidinput->input,
               hu->type, hu->code, value);
   return 1;
}

static struct hid_driver avr_driver = {
     .name = "atmega16",
     .id_table = avr_devices,
     .raw_event = avr_raw_event,
     .probe = avr_probe,
     .remove = avr_remove,
     //.event = avr_event,
};

static int __init
_hid_init(void)
{
   printk("HID device init\n");

   return hid_register_driver(&avr_driver);
}

static void
_hid_exit(void)
{
   printk("HID device exit\n");
   hid_unregister_driver(&avr_driver);
}

module_init(_hid_init);
module_exit(_hid_exit);
