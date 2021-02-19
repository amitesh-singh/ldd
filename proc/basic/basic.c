#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>
#include <linux/slab.h> //kmalloc

#define DATA_SIZE 1024

char *msg = NULL;
struct proc_dir_entry *proc;
int len;


ssize_t my_proc_read(struct file *filp, char *buf, size_t count, loff_t *offp)
{
   int err;

   char *data = PDE_DATA(file_inode(filp));
   if ((int)(*offp) > len)
     return 0;

   if (!data)
     {
        printk(KERN_ALERT "NULL DATA");
        return 0;
     }
   if (count == 0)
     {
        printk(KERN_ALERT "Do nothing");
        return count;
     }

   count = len + 1;
   err = copy_to_user(buf, data, count);
   *offp = count;

   if (err)
     {
        printk(KERN_INFO "Error in copying data");
     }

   return count;

}

ssize_t my_proc_write(struct file *filp, const char __user *buffer, size_t count, loff_t *pos)
{
   char *data = PDE_DATA(file_inode(filp));
   if (count > DATA_SIZE)
     return -EFAULT;

   printk(KERN_ALERT "Writing to proc");
   if (copy_from_user(data, buffer, count))
     {
        return -EFAULT;
     }

   data[count - 1] = '\0';
   printk(KERN_INFO "msg has been set to %s", msg);

   *pos = (int) count;
   len = count - 1;

   return count;
}

const struct proc_ops proc_fops = {
     .proc_read = my_proc_read,
     .proc_write = my_proc_write,
};


int proc_init(void)
{
   char *DATA = "Hello World!";

   msg = kmalloc((size_t) DATA_SIZE, GFP_KERNEL);
   if (msg != NULL)
     {
        printk(KERN_INFO "Allocated memory for msg");
     }
   else return -1;

   len = strlen(DATA);
   strncpy(msg, DATA, len + 1);

   proc = proc_create_data("my_proc", 0666, NULL, &proc_fops, msg);

   return 0;
}

void proc_exit(void)
{
   remove_proc_entry("my_proc", NULL);
}

MODULE_LICENSE("GPL");

module_init(proc_init);
module_exit(proc_exit);
