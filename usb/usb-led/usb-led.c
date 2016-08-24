#include <linux/init.h> // macros used to markup functions e.g. __init, __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/kernel.h> // Contains types, macros, functions for the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model

#include <linux/usb.h> //for usb stuffs
#include <linux/workqueue.h> //for work_struct, INIT_WORK, PREPARE_WORK
#include <linux/slab.h> //for

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("first usb driver module");
MODULE_VERSION("0.1"); // --> module version of kernel driver


//one structure for each connected device
struct my_usb {
     struct usb_device *udev;
     struct work_struct work;
};

//you might need to add file_operations support too if you want
// the device to be accessed from user-space. For simplicity, i did not add it

#define MY_USB_VENDOR_ID 0x16c0
#define MY_USB_PRODUCT_ID 0x03e8
static struct usb_device_id my_usb_table[] = {
       { USB_DEVICE(MY_USB_VENDOR_ID, MY_USB_PRODUCT_ID) },
       {},
};

MODULE_DEVICE_TABLE(usb, my_usb_table);


static void
my_usb_work_func(struct work_struct *work)
{
   struct my_usb *data = container_of(work, struct my_usb, work);
   int err;

   err = usb_control_msg(data->udev,
                         usb_sndctrlpipe(data->udev, 0),
                         2, USB_TYPE_VENDOR | USB_DIR_OUT,
                         0, 0,
                         NULL, 0,
                         1000);

   //do jobs send usb control msg
}

//called when a usb device is connected to PC
static int
my_usb_probe(struct usb_interface *interface,
             const struct usb_device_id *id)
{
   struct usb_device *udev = interface_to_usbdev(interface);
   struct my_usb *data;
   int err;

   printk(KERN_INFO "manufacturer: %s", udev->manufacturer);
   printk(KERN_INFO "product: %s", udev->product);

   data = kzalloc(sizeof(struct my_usb), GFP_KERNEL);
   if (data == NULL)
     {
        //handle error
     }

   //increase ref count, make sure u call usb_put_dev() in disconnect()
   data->udev = usb_get_dev(udev);
   usb_set_intfdata(interface, data);

   err = usb_control_msg(data->udev,
                         usb_sndctrlpipe(data->udev, 0),
                         1, USB_TYPE_VENDOR | USB_DIR_OUT,
                         0, 0,
                         NULL, 0,
                         1000);
   if (err < 0)
     {
        //handle error
     }
   printk(KERN_INFO "usb device is connected");

            //work_struct var, worker fuction => void (*func)(work_struct *)
   INIT_WORK(&data->work, my_usb_work_func);

   return 0;
}

//called when unplugging a USB device
static void
my_usb_disconnect(struct usb_interface *interface)
{
   struct my_usb *data;

   data = usb_get_intfdata(interface);
   cancel_work_sync(&data->work);

   usb_set_intfdata(interface, NULL);

   //deref the count
   usb_put_dev(data->udev);

   kfree(data); //deallocate, allocated by kzmalloc()

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
   printk(KERN_INFO "usb driver is unloaded");
   usb_deregister(&my_usb_driver);
}

module_init(_usb_init);
module_exit(_usb_exit);
