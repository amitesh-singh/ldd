#include <linux/init.h> // macros used to markup functions e.g. __init, __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/kernel.h> // Contains types, macros, functions for the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model

#include <linux/usb.h> //for usb stuffs

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("first usb driver module");
MODULE_VERSION("0.1"); // --> module version of kernel driver

#define MY_USB_PRODUCT_ID 0x1843
#define MY_USB_VENDOR_ID 0x8080

//one structure for each connected device
struct my_usb {
};

//you might need to add file_operations support too if you want
// the device to be accessed from user-space. For simplicity, i did not add it

static struct usb_device_id my_usb_table[] = {
       { USB_DEVICE(MY_USB_VENDOR_ID, MY_USB_PRODUCT_ID) },
       {},
};

//This is added to support Hot-plugging, by loading/inserting the driver
// if not already loaded
// its need to be placed in /lib/module/`uname -r`/kernel
// then run depmod -a
/*

   Manually -> insmod modulename.ko or modprob modulename.ko

   Automatically-> There are multiple ways.

   1.	copy to /lib/modules/`uname -r`/kernel/modulename.ko and run depmod -a

   2.	Write a script/command to load the module.ko for an specific harware add/change/remove event in a udev rule /etc/udev/rules.d/10-local.rules. You can do both load/unload using this method.
   3.	Code your module with MODULE_DEVICE_TABLE registration. Then load your modulename.ko once and run depmod command [sudo depmod -a] to add the new module to /lib/modules/3.16.0-34-generic/modules.alias /lib/modules/3.16.0-34-generic/modules.dep files. As I know, system will load only if the module is not loaded.

   You can monitor module loading/unloading using udev events using :

   udevadm monitor
 */
MODULE_DEVICE_TABLE(usb, my_usb_table);

//called when a usb device is connected to PC
static int
my_usb_probe(struct usb_interface *interface,
             const struct usb_device_id *id)
{
   printk(KERN_INFO "usb device is connected");

   return 0;
}

//called when unplugging a USB device
static void
my_usb_disconnect(struct usb_interface *interface)
{
   printk(KERN_INFO "usb device is disconnected");
}

static struct usb_driver my_usb_driver = {
     .name = "my first usb driver",
     .id_table = my_usb_table,
     .probe = my_usb_probe,
     .disconnect = my_usb_disconnect,
};

//called on module loading
static int __init
_usb_init(void)
{
   int result;
   printk(KERN_INFO "usb driver is loaded");

   result = usb_register(&my_usb_driver);
   if (result)
     {
        printk(KERN_ALERT "device registeration failed!!");
     }
   else
     {
        printk(KERN_INFO "device registered");
     }

   return result;
}

//called on module unloading
static void __exit
_usb_exit(void)
{
   usb_deregister(&my_usb_driver);
   printk(KERN_INFO "usb driver is unloaded");
}

module_init(_usb_init);
module_exit(_usb_exit);
