#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/of_platform.h>

#include <drm/drm_drv.h>
#include <drm/drm_module.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");

static struct drm_driver my_drm_driver = {
     .driver_features = DRIVER_MODESET | DRIVER_GEM,
     .name = "ami",
     .desc = "amitesh singh",
     .date = "20230604",
     .major = 1,
     .minor = 0,
     .patchlevel = 0,
};

struct my_dev {
	struct drm_device drm;
};



static int _probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct drm_device *ddev;
	int ret;

	ddev = drm_dev_alloc(&my_drm_driver, dev);
	if (!ddev) {
		pr_err("unable to allocate drm_device");
		return -ENOMEM;
	}

	ret = drm_dev_register(ddev, 0);
//	drm_fbdev_generic_setup(ddev, 16);


	platform_set_drvdata(pdev, ddev);
	

	return 0;
}

static int _remove(struct platform_device *pdev)
{
	struct drm_device *ddev = platform_get_drvdata(pdev);

	drm_dev_unregister(ddev);
	drm_dev_put(ddev);
	return 0;
}

static const struct of_device_id x_drv_dts_ids[] = {
	{ .compatible = "x,xx"},
	{ /* end node */ },
};
MODULE_DEVICE_TABLE(of, x_drv_dts_ids);


static struct platform_driver x_drm_platform_driver = {
	.probe = _probe,
	.remove = _remove,
	.driver = {
		.name = "x,display",
		.of_match_table = x_drv_dts_ids,
		//.pm = //TODO: read about .pm
	},
};

drm_module_platform_driver(x_drm_platform_driver);

