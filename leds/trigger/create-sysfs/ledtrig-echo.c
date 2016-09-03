#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/leds.h>

static int8_t val = 0;

static ssize_t
_attr_show(struct device *dev,
           struct device_attribute *attr,
           char *buf)
{
   return sprintf(buf, "%d\n", val);
}

static ssize_t
_attr_store(struct device *dev,
            struct device_attribute *attr,
            const char *buf, size_t size)
{

   int ret = 0;
   printk(KERN_ALERT "buffer: %s\n", buf);
   //it gives warning, long can be used
   ret = kstrtol(buf, 10, &val);
   if (ret)
     return ret;
   return 1;
}

//you could also use
// static DEVICE_ATTR(fun, 0666, _attr_show, _attr_store);
// creates dev_attr_fun
static struct device_attribute dev_attr_fun = {
     .attr = {.name = "eko",
          .mode = 0666}, //so that non-root user could modify, echo 2 > eko
     .show = _attr_show,
     .store = _attr_store
};

static void
mytrig_crfs_activate(struct led_classdev *ldev)
{
   printk(KERN_ALERT "file named eko created in sys/class/leds/<device>/");
   //create the sysfs attribute named "eko"
   // in /sys/class/leds/<>/
   device_create_file(ldev->dev,
                      &dev_attr_fun);
}

static void
mytrig_crfs_deactivate(struct led_classdev *ldev)
{
   printk(KERN_ALERT "file named: eko deleted from /sys/class/leds/<device>/");
   //remove 'eko' file from /sys/class/leds/<>/
   device_remove_file(ldev->dev,
                      &dev_attr_fun);
}

static struct led_trigger mytrig_crfs = {
     .name = "create-eko",
     .activate = mytrig_crfs_activate,
     .deactivate = mytrig_crfs_deactivate
};

static int __init
mytrig_init(void)
{
   printk(KERN_INFO "module loaded");
   return led_trigger_register(&mytrig_crfs);
}

static void __exit
mytrig_exit(void)
{
   printk(KERN_INFO "module unloaded");
   led_trigger_unregister(&mytrig_crfs);
}
module_init(mytrig_init);
module_exit(mytrig_exit);

MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("Create an echo file");
MODULE_LICENSE("GPL v2");
