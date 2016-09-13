#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/input.h>
#include <linux/platform_device.h>

MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("Based on \"essential linux device drivers\" book example");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

static struct input_dev *idev;
static struct platform_device *pdev;

static ssize_t get_value(struct device *dev,
                         struct device_attribute *attr, char *buf)
{
   return -EINVAL; // do not support reading. Invalid argument error will come out on read attempt
}

static ssize_t set_value(struct device *dev,
                         struct device_attribute *attr,
                         const char *buf, size_t count)
{
   int x, y;

   sscanf(buf, "%d%d", &x, &y);

   input_report_rel(idev, REL_X, x);
   input_report_rel(idev, REL_Y, y);

   // This will trigger mouse right most key but
   // i have seen mouse click won't work afterwards.
   // TODO: check the cause behind it?

   // input_report_key(idev, BTN_RIGHT, 1);
   input_sync(idev);

   return count;
}

static DEVICE_ATTR(coordinates, 0660, get_value,
                   set_value);

static int __init
_vms_init(void)
{
   pdev = platform_device_register_simple("vmd", -1,
                                          NULL, 0);
   printk(KERN_ALERT "vmd init");
   if (IS_ERR(pdev))
     {
        printk(KERN_ALERT "Failed to create platform_device");
        return PTR_ERR(pdev);
     }

   idev = input_allocate_device();
   if (!idev)
     {
        printk(KERN_ALERT "failed to allocate device:");
        return -ENOMEM;
     }

   idev->name = "virtual mouse";  // name of the device
   idev->phys = "vmd/input0"; // physical path to the device in the system hierarchy
   idev->id.bustype = BUS_VIRTUAL; // bustype
   idev->id.vendor = 0x00; // vendor
   idev->id.product = 0x00; // product
   idev->id.version = 0x00; // version
   /*
      struct input_id {
      __u16 bustype;
      __u16 vendor;
      __u16 product;
      __u16 version;
      };
    */

   // bitmap of types of events supported by the device (EV_KEY, * EV_REL, etc.)
   // @keybit: bitmap of keys/buttons this device has
   // @relbit: bitmap of relative axes for the device
   idev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
   idev->keybit[BIT_WORD(BTN_MOUSE)] = BIT_MASK(BTN_LEFT) | BIT_MASK(BTN_RIGHT) | BIT_MASK(BTN_MIDDLE);
   idev->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y);
   idev->keybit[BIT_WORD(BTN_MOUSE)] |= BIT_MASK(BTN_SIDE) | BIT_MASK(BTN_EXTRA);
   idev->relbit[0] |= BIT_MASK(REL_WHEEL);

   input_register_device(idev);
   device_create_file(&pdev->dev, &dev_attr_coordinates);

   return 0;
}

static void __exit
_vms_exit(void)
{
   device_remove_file(&pdev->dev, &dev_attr_coordinates);
   input_unregister_device(idev);
   platform_device_unregister(pdev);
   printk(KERN_ALERT "vmd exit");
}

module_init(_vms_init);
module_exit(_vms_exit);
