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
   uint8_t val = 0;
   // its important to keep checking if someone requested to stop the thread.
   // kthread_stop or killall -9 <pid> actually requests to stop the kernel.
   // if you don't hv this check .. you will get kernel panic! :/
   // although this thread still is invincible,
   // killall -9 <thread> won't kill it..
   // you need to all signal which it can obey. check allow_signal example
   while (!kthread_should_stop())
     {
        val ^= 0x01;
        printk(KERN_INFO "thread is running and doing the job required: %d", val);
        ssleep(5);
     }
   printk(KERN_INFO "Stopping the thread function.. exiting..");

   return 0;
}

static int __init
kernel_init(void)
{
   printk(KERN_INFO "create kthread example");

   task = kthread_run(&thread_function, NULL, "_ami_");

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
