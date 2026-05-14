#include "midi_core.h"
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/atomic.h>

/* Track how many file descriptors currently have the device open. */
#define MAX_OPEN_COUNT 4
static atomic_t open_count = ATOMIC_INIT(0);

static int midi_open(struct inode *inode, struct file *filp)
{
	if (atomic_read(&open_count) >= MAX_OPEN_COUNT) {
		pr_err(DEVICE_NAME ": max open count (%d) reached\n", MAX_OPEN_COUNT);
		return -EBUSY;
	}
	atomic_inc(&open_count);
	pr_info(DEVICE_NAME ": device opened (open_count=%d)\n", atomic_read(&open_count));
	return 0;
}

static int midi_release(struct inode *inode, struct file *filp)
{
	atomic_dec(&open_count);
	pr_info(DEVICE_NAME ": device closed (open_count=%d)\n", atomic_read(&open_count));
	return 0;
}

static ssize_t midi_read(struct file *filp, char __user *buf,
			 size_t count, loff_t *ppos)
{
	int ret;
	unsigned int copied;
	unsigned long flags;
	u8 *kbuf;

	/* 1. Sleep until data is available in FIFO */
	ret = wait_event_interruptible(midi_wq, !kfifo_is_empty(&midi_fifo));
	if (ret)
		return -EINTR;

	/* 2. Allocate temporary kernel buffer */
	if (count > FIFO_SIZE) count = FIFO_SIZE;
	kbuf = kmalloc(count, GFP_KERNEL);
	if (!kbuf) return -ENOMEM;

	/* 3. Extract data from FIFO safely */
	if (mutex_lock_interruptible(&midi_mutex)) {
		kfree(kbuf);
		return -EINTR;
	}

	spin_lock_irqsave(&fifo_lock, flags);
	ret = kfifo_out(&midi_fifo, kbuf, count);
	spin_unlock_irqrestore(&fifo_lock, flags);

	if (ret == 0) {
		mutex_unlock(&midi_mutex);
		kfree(kbuf);
		return 0; /* Someone else read the data, try again */
	}

	/* 4. Copy to User Space */
	copied = ret;
	if (copy_to_user(buf, kbuf, copied)) {
		mutex_unlock(&midi_mutex);
		kfree(kbuf);
		return -EFAULT;
	}

	mutex_unlock(&midi_mutex);
	kfree(kbuf);
	pr_info(DEVICE_NAME ": read %u bytes\n", copied);
	return copied;
}

static ssize_t midi_write(struct file *filp, const char __user *buf,
			  size_t count, loff_t *ppos)
{
	unsigned int enqueued;
	unsigned long flags;
	u8 *kbuf;

	if (count == 0) return 0;
	if (count > FIFO_SIZE) count = FIFO_SIZE;

	kbuf = kmalloc(count, GFP_KERNEL);
	if (!kbuf) return -ENOMEM;

	/* 1. Copy from User Space to Kernel Space */
	if (mutex_lock_interruptible(&midi_mutex)) {
		kfree(kbuf);
		return -EINTR;
	}

	if (copy_from_user(kbuf, buf, count)) {
		mutex_unlock(&midi_mutex);
		kfree(kbuf);
		return -EFAULT;
	}

	/* 2. Push into FIFO safely */
	spin_lock_irqsave(&fifo_lock, flags);
	enqueued = kfifo_in(&midi_fifo, kbuf, count);
	spin_unlock_irqrestore(&fifo_lock, flags);

	mutex_unlock(&midi_mutex);
	kfree(kbuf);

	/* 3. Wake up any blocked readers */
	wake_up_interruptible(&midi_wq);

	pr_info(DEVICE_NAME ": wrote %u bytes\n", enqueued);
	return enqueued;
}

const struct file_operations midi_fops = {
	.owner   = THIS_MODULE,
	.open    = midi_open,
	.release = midi_release,
	.read    = midi_read,
	.write   = midi_write,
};
