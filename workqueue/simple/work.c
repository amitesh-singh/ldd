#include <linux/init.h> // macros used to markup functions e.g. __init, __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/kernel.h> // Contains types, macros, functions for the kernel
#include <linux/workqueue.h> // for work, INIT_WORK, schedhule_work

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("A work example");
MODULE_VERSION("0.1");

static void
mywork_function(struct work_struct *work)
{
   printk("mywork function executed.\n");
}

static struct work_struct mywork;

static int __init
_work_init(void)
{
   printk("initialize work struct\n");
   INIT_WORK(&mywork, mywork_function);

   schedule_work(&mywork);

   return 0;
}

static void
_work_exit(void)
{
   //we can use flush_work(&mywork) instead to cancel out all scheduled work
   // To flush the kernel-global work queue, call flush_scheduled_work().
   if (work_pending(&mywork))
     cancel_work_sync(&mywork);
   printk("module de-init, work sync cancelled\n");
}

module_init(_work_init);
module_exit(_work_exit);

