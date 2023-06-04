#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <drm/drm_drv.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");

static struct drm_driver my_drm_driver = {
     .driver_features = DRIVER_MODSET | DRIVER_GEM,
     .name = "ami",
     .desc = "amitesh singh",
     .date = "20230604",
     .major = 1,
     .minor = 0,
     .patchlevel = 0,
};

static int __init
driver_init(void)
{
   int ret;

   ret = drm_driver_register(&my_drm_driver);
   if (ret) {
        pr_err("Failed to register drm driver: %d\n", ret);
        return ret;
   }

   return 0;
}

static void __exit
driver_exit(void)
{
}
