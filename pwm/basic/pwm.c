#include <linux/init.h>// macros used to markup functions e.g. __init, __exit
#include <linux/module.h>// Core header for loading LKMs into the kernel
#include <linux/kernel.h>// Contains types, macros, functions for the kernel
#include <linux/device.h>// Header to support the kernel Driver Model

#include <linux/usb.h> //for usb stuffs
#include <linux/slab.h> //for

#include <linux/workqueue.h> //for work_struct
#include <linux/platform_device.h>

#include <linux/pwm.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("pwm driver"); 
MODULE_VERSION("0.1"); //

struct xboard_pwm_chip {
  struct pwm_chip chip;
};

/*
static struct pwm_lookup xboard_pwm_lookup[] = {
     PWM_LOOKUP("xboard-pwm", 0, "pwm-xboard", NULL,
                36296, PWM_POLARITY_NORMAL),
};
*/

static int xboard_pwm_config(struct pwm_chip *chip, struct pwm_device *pwm,
                             int duty_ns, int period_ns)
{

  return 0;
}

static int xboard_pwm_enable(struct pwm_chip *chip, struct pwm_device *pwm)
{

  return 0;
}

static void xboard_pwm_disable(struct pwm_chip *chip, struct pwm_device *pwm)
{
}

static const struct pwm_ops xboard_pwm_ops = {
  .config = xboard_pwm_config,
  .enable = xboard_pwm_enable,
  .disable = xboard_pwm_disable,
  .owner = THIS_MODULE,

};
static int _xboard_pwm_probe(struct platform_device *pdev)
{
  struct xboard_pwm_chip *xpwm;
  int err;

  //this is not exposed to be used in kernel module,
  // only work with static compilation :/
  //pwm_add_table(xboard_pwm_lookup, ARRAY_SIZE(xboard_pwm_lookup));

/*
devm_kzalloc() is resource-managed kzalloc(). The memory allocated with resource-managed functions
 is associated with the device. When the device is detached from the system or the driver for the 
 device is unloaded, that memory is freed automatically. It is possible to free the memory with
  devm_kfree() if it's no longer needed.
*/
  xpwm = devm_kzalloc(&pdev->dev, sizeof(*xpwm), GFP_KERNEL);
  if (!xpwm) return -ENOMEM;

  xpwm->chip.dev = &pdev->dev;
  xpwm->chip.ops = &xboard_pwm_ops;
  xpwm->chip.base = pdev->id;
  xpwm->chip.npwm = 1; // we got how many pwms on board?4? or 6

  err = pwmchip_add(&xpwm->chip); //add pwm chip 
  if (err < 0) return err;

  platform_set_drvdata(pdev, xpwm);

  printk(KERN_INFO "pwm driver probed");

  return 0;
}

static int _xboard_pwm_remove(struct platform_device *pdev)
{
  struct xboard_pwm_chip *xpwm = platform_get_drvdata(pdev);
  int err;

  if (!xpwm) return 0;
  err = pwmchip_remove(&xpwm->chip);
  if (err < 0) return err;

  printk(KERN_INFO "pwm driver removed");
  /*
  devm_kzalloc() is resource-managed kzalloc(). The memory allocated with resource-managed functions
   is associated with the device. When the device is detached from the system or the driver
    for the device is unloaded, that memory is freed automatically. It is possible to free the
     memory with devm_kfree() if it's no longer needed.
  */
  //devm_kfree(&pdev->dev, xpwm);
  return 0;
}

static struct platform_driver xboard_pwm_driver = {
  .driver = {
    .name = "xboard-pwm",
  },
  .probe = _xboard_pwm_probe,
  .remove = _xboard_pwm_remove,
};

module_platform_driver(xboard_pwm_driver);
