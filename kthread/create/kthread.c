#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/kthread.h>
#include<linux/sched.h>
#include<linux/delay.h> //for ssleep()

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("example for kthread creation");
MODULE_VERSION("0.1");

static struct task_struct *task;
static int ret;

static int
thread_function(void *data)
{
   // its important to keep checking if someone requested to stop the thread.
   // kthread_stop or killall -9 <pid> actually requests to stop the kernel.
   // if you don't hv this check .. you will get kernel panic!
   while (!kthread_should_stop())
     {
        printk(KERN_INFO "thread is running and doing the job required");
        ssleep(5);
     }
   printk(KERN_INFO "Stopping the thread function.. exiting..");

   return 0;
}

static int __init
kernel_init(void)
{
   printk(KERN_INFO "create kthread example");

   // you could call kthread_run(&thread_function, NULL, "_ami_")
   // this will create and run thread also.. so no need to call wake_up_process()
   task = kthread_create(&thread_function, NULL, "_ami_");
   if (task) wake_up_process(task);

   printk(KERN_INFO"Kernel Thread name : %s\n", task->comm);

   return 0;
}

static void __exit
kernel_exit(void)
{
   //stop the kthead on unloading the module
   ret = kthread_stop(task);
   printk(KERN_ALERT "Unloading the module: %d", ret);
}

module_init(kernel_init);
module_exit(kernel_exit);
