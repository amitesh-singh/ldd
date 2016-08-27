#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/kthread.h>
#include<linux/sched.h>
#include<linux/delay.h> //for ssleep

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("example for kthread creation");
MODULE_VERSION("0.1");

static struct task_struct *task;

static int cpu;

static int
thread_function(void *data)
{
   while (!kthread_should_stop())
     {
        printk(KERN_INFO "thread is running..: at cpu: %d", get_cpu());
        ssleep(5);
     }
   printk(KERN_INFO "exiting from thread kernel");
   do_exit(0);

   return 0;
}

static int __init
kernel_init(void)
{
   cpu = get_cpu();
   put_cpu();
   printk(KERN_INFO "create kthread example");
   printk(KERN_INFO "main thread cpu: %d", cpu);
   //you could use kthread_run() which creates the thread and run it
   task = kthread_create(&thread_function, NULL,
                         "_ami_");
   printk(KERN_INFO "binding this new thread to cpu: %d",
          cpu + 1);
   kthread_bind(task, cpu + 1); //run the thread on processor 2
   printk(KERN_INFO"Kernel Thread name : %s\n",
          task->comm);
   if (task) wake_up_process(task);

   return 0;
}

static void __exit
kernel_exit(void)
{
   //stop the kthead on unloading the module
   kthread_stop(task);
   printk("kthread cpu bin module is unloaded");
}

module_init(kernel_init);
module_exit(kernel_exit);
