#include <linux/version.h>
#include <linux/module.h>    // included for all kernel modules
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>      // included for __init and __exit macros

#include <linux/proc_fs.h>   // file operations
#include <linux/seq_file.h>  // seq_read, ...

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dr. Dyson");
MODULE_DESCRIPTION("Global Information Grid");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

static int skynet_show(struct seq_file *m, void *v);

static int skynet_open(struct inode *inode, struct  file *file);

#ifdef HAVE_PROC_OPS
static const struct proc_ops skynet_ops = {
  .proc_open = skynet_open,
  .proc_read = seq_read,
  .proc_lseek = seq_lseek,
  .proc_release = single_release,
};
#else
static const struct file_operations skynet_ops = {
  .owner = THIS_MODULE,
  .open = skynet_open,
  .read = seq_read,
  .llseek = seq_lseek,
  .release = single_release,
};
#endif


static int skynet_show(struct seq_file *m, void *v) {
 here:
  seq_printf(m, "Skynet location: 0x%lx\n", (unsigned long)&&here);
  return 0;
}

static int skynet_open(struct inode *inode, struct  file *file) {
  return single_open(file, skynet_show, NULL);
}


static int __init skynet_init(void) {
  proc_create("skynet", 0, NULL, &skynet_ops);
  printk(KERN_INFO "Skynet in control\n");

  return 0;
}

static void __exit skynet_cleanup(void) {
  remove_proc_entry("skynet", NULL);
  printk(KERN_INFO "I'll be back!\n");
}

module_init(skynet_init);
module_exit(skynet_cleanup);
