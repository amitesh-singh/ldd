#include <linux/init.h> // macros used to markup functions e.g. __init, __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/kernel.h> // Contains types, macros, functions for the kernel
#include <linux/spinlock.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("spin lock example");
MODULE_VERSION("0.1"); // --> module version of kernel

// it seems SPIN_LOCK_UNLOCKED had been discontinued.
// use spin_lock_init();
static spinlock_t mylock; // = SPIN_LOCK_UNLOCKED;

static int __init
hello_init(void)
{
   printk(KERN_INFO "spin lock init");
   spin_lock_init(&mylock);

   printk(KERN_INFO "do spin_lock");
   spin_lock(&mylock);

   printk(KERN_INFO "spin lock example!");

   spin_unlock(&mylock);
   printk(KERN_INFO "do spin_unlock");

   return 0;
}

//  The __exit macro notifies that if this
//  code is used for a built-in driver (not a LKM) that this function is not required.
static void __exit
hello_exit(void)
{
   printk(KERN_INFO "unloading spin-lock example module");
}

module_init(hello_init);
module_exit(hello_exit);
