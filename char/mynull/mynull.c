#include <linux/init.h> // macros used to markup functions e.g. __init, __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/kernel.h> // Contains types, macros, functions for the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model

#include <linux/fs.h> // Header for the Linux file system support

#define DEVICE_NAME "mynull"
#define DEVICE_CLASS "null"
//Refer https://github.com/torvalds/linux/blob/master/drivers/char/mem.c

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("mynull module");
MODULE_VERSION("0.1"); // --> module version of kernel

static char *name = "null";
module_param(name, charp, S_IRUGO);
MODULE_PARM_DESC(name, "The name to display in /var/log/kern.log");

static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static int major_num;
static struct class *nullClass = NULL;
static struct device *nullDevice = NULL;

//check linux/fs.h for full details of below struct.
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release
};

static int __init
mynull_init(void)
{
   //try to get the major name from kernel instead of hardcoding
   major_num = register_chrdev(0, DEVICE_NAME, &fops);

   if (major_num < 0)
     {

        printk(KERN_ALERT "failed to load %s driver\n", name);
        return major_num;
     }
   printk(KERN_INFO "%s device created successfully with major number: %d\n", name, major_num);

   nullClass = class_create(THIS_MODULE, DEVICE_CLASS);
   //TODO: fix the node permission while creation
   //nullClass->devnode = device_devnode; ?

   if (IS_ERR(nullClass))
     {
        class_destroy(nullClass);
        unregister_chrdev(major_num, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create class\n");
        return PTR_ERR(nullClass);
     }

   // this is doing mknod /dev/devicename c <major_num> <minor_num> for us.
   nullDevice = device_create(nullClass, NULL, MKDEV(major_num, 0),
                              NULL, DEVICE_NAME);
   if (IS_ERR(nullDevice))
     {
        device_destroy(nullClass, MKDEV(major_num, 0));
        class_destroy(nullClass);
        unregister_chrdev(major_num, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create device\n");
        return PTR_ERR(nullDevice);
     }
   printk(KERN_INFO "%s device created successfully\n", DEVICE_NAME);

  return 0;
}

static void __exit
mynull_exit(void)
{
     device_destroy(nullClass, MKDEV(major_num, 0));
     class_unregister(nullClass);
     class_destroy(nullClass);
     unregister_chrdev(major_num, DEVICE_NAME);
     printk("%s: %s device is destoryed\n", name, DEVICE_NAME);
}

static int
dev_open(struct inode *in, struct file *fl)
{
   printk(KERN_INFO "device is opened\n");

   return 0;
}

///This is called when u close the fd by close()
static int
dev_release(struct inode *in, struct file *fl)
{
   printk(KERN_INFO "device is closed, applied close(fd)?\n");

   return 0;
}

//userspace reads the device
static ssize_t
dev_read(struct file *f, char *buf, size_t len, loff_t *offset)
{
   printk(KERN_INFO "This is a black hole and i ate all your input, so i got nothing\n");
   len = sprintf(buf, "null: %d", 1);
   return len;
}

//userspace writes to device
static ssize_t
dev_write(struct file *f, const char *buf, size_t len, loff_t *offset)
{
   printk(KERN_INFO "This all goes to black hole now.\n");
   return len;
}

module_init(mynull_init);
module_exit(mynull_exit);
