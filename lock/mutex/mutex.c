#include <linux/init.h> // macros used to markup functions e.g. __init, __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/kernel.h> // Contains types, macros, functions for the kernel
#include <linux/mutex.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("mutex lock example");
MODULE_VERSION("0.1"); // --> module version of kernel

static struct mutex mymutexlock;

static int __init
hello_init(void)
{
   printk(KERN_INFO "mutex init");
   mutex_init(&mymutexlock);

   printk(KERN_INFO "do mutex_lock");
   mutex_lock(&mymutexlock);

   printk(KERN_INFO "mutex lock example!");

   mutex_unlock(&mymutexlock);
   printk(KERN_INFO "do mutex_unlock");

   return 0;
}

//  The __exit macro notifies that if this
//  code is used for a built-in driver (not a LKM) that this function is not required.
static void __exit
hello_exit(void)
{
   printk(KERN_INFO "unloading mutex-lock example module");
}

module_init(hello_init);
module_exit(hello_exit);
