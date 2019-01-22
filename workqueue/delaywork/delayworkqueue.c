#include <linux/init.h> // macros used to markup functions e.g. __init, __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/kernel.h> // Contains types, macros, functions for the kernel
#include <linux/workqueue.h> // for work, INIT_WORK, schedhule_work


static struct delayed_work mywork;

static void _work_func(struct work_struct *work)
{
    printk("work is done.");
}

static int __init
_workqueue_init(void)
{
    INIT_DELAYED_WORK(&mywork, _work_func);
    schedule_delayed_work(&mywork, 2 *HZ);
    return 0;
}

static void __exit
_workqueue_exit(void)
{
    cancel_delayed_work(&mywork);
}

module_init(_workqueue_init);
module_exit(_workqueue_exit);