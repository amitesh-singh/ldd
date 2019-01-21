#include <linux/types.h>
#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/module.h>

//check user.c file for accessing this driver.
// in this example, we just connect this driver to /dev/ttyACM0
// from stm32.

static int n_tty_open(struct tty_struct *tty)
{
   printk(KERN_ALERT "open");
   return 0;
}

static void n_tty_close(struct tty_struct *tty)
{
   printk(KERN_ALERT "close");
}

static ssize_t n_tty_read(struct tty_struct *tty, struct file *file,
                           unsigned char __user * buf, size_t nr)
{
   printk(KERN_ALERT "READ ");
   //return -EOPNOTSUPP;
   return 1;
}

static ssize_t n_tty_write(struct tty_struct *tty, struct file *file,
                            const unsigned char *buf, size_t nr)
{
   printk(KERN_ALERT "WRITE ");
   //return -EOPNOTSUPP;
   return 10;
}

static void n_tty_receivebuf(struct tty_struct *tty,
                              const unsigned char *cp, char *fp,
                              int cnt)
{
   printk(KERN_ALERT "got something");
}

static struct tty_ldisc_ops tty_ldisc = {
     .owner      =  THIS_MODULE,
     .magic      =  TTY_LDISC_MAGIC,
     .name    =  "n_tty",
     .open    =  n_tty_open,
     .close      =  n_tty_close,
     .read    =  n_tty_read,
     .write      =  n_tty_write,
     .receive_buf   =  n_tty_receivebuf
};

static int __init stm32_tty_init(void)
{
   printk(KERN_ALERT "tty init");
   BUG_ON(tty_register_ldisc(N_TTY, &tty_ldisc));
   return 0;
}

static void __exit stm32_tty_exit(void)
{
   printk(KERN_ALERT "tty de-init");
   tty_unregister_ldisc(N_TTY);
}

module_init(stm32_tty_init);
module_exit(stm32_tty_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_ALIAS_LDISC(N_TTY);
MODULE_DESCRIPTION("ldisc driver");
