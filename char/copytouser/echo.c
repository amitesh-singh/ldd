#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h> //for copy_to/from_user
#include <linux/slab.h> //for kmalloc/kfree
#include <linux/device.h>

#include <linux/fs.h>
#include <linux/spinlock.h>

#define DEVICE_NAME "echo"
#define DEVICE_CLASS "echo"
//Refer https://github.com/torvalds/linux/blob/master/drivers/char/mem.c

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("mynull module");
MODULE_VERSION("0.1"); // --> module version of kernel

struct dup_data {
     int end;
     char buf[100];
};
static spinlock_t mylock;

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


//TODO: check if there is any api to set the data of a char device
static struct dup_data *dd = 0;

static int __init
mynull_init(void)
{
   //try to get the major name from kernel instead of hardcoding
   major_num = register_chrdev(0, DEVICE_NAME, &fops);

   if (major_num < 0)
     {

        printk(KERN_ALERT "failed to load driver");
        return major_num;
     }
   nullClass = class_create(THIS_MODULE, DEVICE_CLASS);
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

   spin_lock_init(&mylock);

   dd = kmalloc(sizeof(struct dup_data),
                GFP_KERNEL);
   if (dd == NULL)
     {
        printk(KERN_ALERT "Failed to allocate mem");
        return -ENOMEM;
     }

  return 0;
}

static void __exit
mynull_exit(void)
{
   kfree(dd);
   device_destroy(nullClass, MKDEV(major_num, 0));
   class_unregister(nullClass);
   class_destroy(nullClass);
   unregister_chrdev(major_num, DEVICE_NAME);
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
   spin_lock(&mylock);
   if (copy_to_user(buf, dd->buf, len) != 0)
     {
        printk(KERN_ALERT "Copy to user error!");
        spin_unlock(&mylock);
        return  -ENOMEM;
     }
   spin_unlock(&mylock);
   return len;
}

//userspace writes to device
static ssize_t
dev_write(struct file *f, const char *buf, size_t len, loff_t *offset)
{
   printk(KERN_INFO "This all goes to black hole now.\n");

   spin_lock(&mylock);
   if (copy_from_user(dd->buf, buf, len) != 0)
     {
        printk(KERN_ALERT "copy from user error");
        spin_unlock(&mylock);
        return -ENOMEM;
     }

   spin_unlock(&mylock);
   return len;
}

module_init(mynull_init);
module_exit(mynull_exit);
