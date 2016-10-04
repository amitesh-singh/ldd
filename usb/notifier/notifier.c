//
//  To register and unregister with the already existing notifier chain
//  for hot-plugging of USB devices, use the exported functions:
//
//     register a notifier callback whenever a usb change happens
//     void usb_register_notify   (struct notifier_block *nb);
//
//     unregister a notifier callback
//     void usb_unregister_notify (struct notifier_block *nb);
//

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/init.h>
#include <linux/notifier.h>

static int
my_notifier_call(struct notifier_block *b,unsigned long event,
                 void *data)
{
   pr_info("\nUSB event: %ld\n", event);

   switch(event)
   {
      case USB_DEVICE_ADD:
         pr_info("Adding a usb device, event = USB_DEVICE_ADD\n");
         break;
      case USB_DEVICE_REMOVE:
         pr_info("Removing a usb device, event = USB_DEVICE_REMOVE\n");
         break;
      case USB_BUS_ADD:
         pr_info("Adding a usb bus, event = USB_BUS_ADD\n");
         break;
      case USB_BUS_REMOVE:
         pr_info("Removing a usb bus, event = USB_BUS_REMOVE\n");
         break;

      default:
         break; //do nothing
   }

   return NOTIFY_OK;
}

static struct notifier_block notifier_blck = {
   .notifier_call = my_notifier_call,
   .priority = 0,
};

static int __init
_usb_notify_init(void)
{
   usb_register_notify(&notifier_blck);
   pr_info("USB notifier module init\n");

   return 0;
}

static void __exit
_usb_notify_exit(void)
{
   usb_unregister_notify(&notifier_blck);
   pr_info("USB notifier module exit\n");
}

module_init(_usb_notify_init);
module_exit(_usb_notify_exit);

MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("simple notifier usb driver");
MODULE_LICENSE("GPL");
