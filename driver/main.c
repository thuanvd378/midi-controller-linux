#include "midi_core.h"
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>

static dev_t midi_dev_num;
static struct cdev midi_cdev;
static struct class *midi_class;
static struct device *midi_device;

static int __init midi_driver_init(void)
{
	int ret;

	pr_info(DEVICE_NAME ": initialising custom modular driver...\n");

	/* 1. Allocate a dynamic Major number */
	ret = alloc_chrdev_region(&midi_dev_num, 0, 1, DEVICE_NAME);
	if (ret < 0) {
		pr_err(DEVICE_NAME ": alloc_chrdev_region failed\n");
		return ret;
	}

	/* 2. Create device class */
	midi_class = class_create(CLASS_NAME);
	if (IS_ERR(midi_class)) {
		ret = PTR_ERR(midi_class);
		goto fail_class;
	}

	/* 3. Create device node /dev/custom_midi */
	midi_device = device_create(midi_class, NULL, midi_dev_num, NULL, DEVICE_NAME);
	if (IS_ERR(midi_device)) {
		ret = PTR_ERR(midi_device);
		goto fail_device;
	}

	/* 4. Init cdev structure with file operations from fops.c */
	cdev_init(&midi_cdev, &midi_fops);
	midi_cdev.owner = THIS_MODULE;

	ret = cdev_add(&midi_cdev, midi_dev_num, 1);
	if (ret < 0) goto fail_cdev;

	/* 5. Initialize Hardware simulation & Buffers from hw_sim.c */
	ret = hw_sim_init();
	if (ret < 0) goto fail_hw;

	pr_info(DEVICE_NAME ": module loaded successfully (Major=%d)\n", MAJOR(midi_dev_num));
	return 0;

	/* Error Unwind Ladder */
fail_hw:
	cdev_del(&midi_cdev);
fail_cdev:
	device_destroy(midi_class, midi_dev_num);
fail_device:
	class_destroy(midi_class);
fail_class:
	unregister_chrdev_region(midi_dev_num, 1);
	return ret;
}

static void __exit midi_driver_exit(void)
{
	/* Reverse order teardown */
	hw_sim_exit();
	cdev_del(&midi_cdev);
	device_destroy(midi_class, midi_dev_num);
	class_destroy(midi_class);
	unregister_chrdev_region(midi_dev_num, 1);
	
	pr_info(DEVICE_NAME ": module unloaded\n");
}

module_init(midi_driver_init);
module_exit(midi_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BTL Project Team");
MODULE_DESCRIPTION("Modular Custom MIDI Character Driver");
MODULE_VERSION("2.0.0");
