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
/*
In Mutex concept, when thread is trying to lock or acquire the Mutex which is not available then
 that thread will go to sleep until that Mutex is available. Whereas in Spinlock it is different. 
 The spinlock is a very simple single-holder lock. If a process attempts to acquire a spinlock and 
 it is unavailable, the process will keep trying (spinning) until it can acquire the lock.
  This simplicity creates a small and fast lock.

Like Mutex, there are two possible states in Spinlock: Locked or Unlocked.
*/
static int __init
hello_init(void)
{
   printk(KERN_INFO "spin lock init");
   spin_lock_init(&mylock);

   /*
    * If you share data with user context (between Kernel Threads),
    * then you can use this approach.

Lock: spin_lock(spinlock_t *lock)
*/
   printk(KERN_INFO "do spin_lock");
   /*
    * This will take the lock if it is free, otherwise it’ll spin until
    * that lock is free (Keep trying).

Try Lock: spin_trylock(spinlock_t *lock)
    */
   //this returns non-zero if acquired the lock else it returns zero.
   if (!spin_trylock(&mylock))
     {
        printk("Failed to acquire the lock");
        return -1;
     }

   printk(KERN_INFO "spin lock example!");

   if (spin_is_locked(&mylock) != 0)
     printk("lock is acquired");
   spin_unlock(&mylock);
   printk(KERN_INFO "do spin_unlock");
   //checking lock, spin_is_lock(spinlock_t *)
   // returns non-zero if lock is acquired. here it will return 0 since lock is not required.
   if (spin_is_locked(&mylock) == 0)
      printk("lock is not acquired");
      
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
