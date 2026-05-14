#include "midi_core.h"
#include <linux/usb.h>
#include <linux/slab.h>

/* Global state for the Core (Declared in midi_core.h) */
struct kfifo midi_fifo;
DEFINE_SPINLOCK(fifo_lock);
DEFINE_MUTEX(midi_mutex);
DECLARE_WAIT_QUEUE_HEAD(midi_wq);

#define MIDI_USB_IN_BUF_SIZE 64

struct midi_usb_device {
	struct usb_device *udev;
	struct urb *in_urb;
	u8 *in_buffer;
	dma_addr_t in_dma;
};

static struct midi_usb_device *my_midi_dev = NULL;

static void midi_usb_read_callback(struct urb *urb)
{
	struct midi_usb_device *dev = urb->context;
	unsigned long flags;
	int i;
	int ret;

	if (urb->status) {
		if (urb->status == -ENOENT || urb->status == -ECONNRESET || urb->status == -ESHUTDOWN)
			return;
		goto resubmit;
	}

	/* USB MIDI packets are 4 bytes. byte 0 is CIN/Cable. bytes 1-3 are MIDI data. */
	spin_lock_irqsave(&fifo_lock, flags);
	for (i = 0; i < urb->actual_length; i += 4) {
		u8 cin = dev->in_buffer[i] & 0x0F;
		int len = 0;
		switch (cin) {
			case 0x8: case 0x9: case 0xB: case 0xE: len = 3; break;
			case 0xC: case 0xD: len = 2; break;
			case 0xF: len = 1; break;
			default: len = 3; break; 
		}
		
		if (len > 0 && (i + 1 + len <= urb->actual_length)) {
			kfifo_in(&midi_fifo, &dev->in_buffer[i + 1], len);
		}
	}
	spin_unlock_irqrestore(&fifo_lock, flags);

	wake_up_interruptible(&midi_wq);

resubmit:
	ret = usb_submit_urb(urb, GFP_ATOMIC);
	if (ret) {
		pr_err(DEVICE_NAME ": failed to resubmit URB, err %d\n", ret);
	}
}

static int midi_usb_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(interface);
	struct usb_host_interface *iface_desc = interface->cur_altsetting;
	struct usb_endpoint_descriptor *endpoint;
	int i;
	int ret;

	pr_info(DEVICE_NAME ": USB MIDI device probed\n");

	if (my_midi_dev) {
		pr_warn(DEVICE_NAME ": Only one device supported at a time\n");
		return -EBUSY;
	}

	my_midi_dev = kzalloc(sizeof(*my_midi_dev), GFP_KERNEL);
	if (!my_midi_dev) return -ENOMEM;

	my_midi_dev->udev = usb_get_dev(udev);

	/* Find bulk/interrupt IN endpoint */
	for (i = 0; i < iface_desc->desc.bNumEndpoints; i++) {
		endpoint = &iface_desc->endpoint[i].desc;
		if (usb_endpoint_dir_in(endpoint) &&
			(usb_endpoint_xfer_bulk(endpoint) || usb_endpoint_xfer_int(endpoint))) {
			
			my_midi_dev->in_buffer = usb_alloc_coherent(udev, MIDI_USB_IN_BUF_SIZE, GFP_KERNEL, &my_midi_dev->in_dma);
			if (!my_midi_dev->in_buffer) goto error;

			my_midi_dev->in_urb = usb_alloc_urb(0, GFP_KERNEL);
			if (!my_midi_dev->in_urb) goto error;

			if (usb_endpoint_xfer_int(endpoint)) {
				usb_fill_int_urb(my_midi_dev->in_urb, udev,
						 usb_rcvintpipe(udev, endpoint->bEndpointAddress),
						 my_midi_dev->in_buffer, MIDI_USB_IN_BUF_SIZE,
						 midi_usb_read_callback, my_midi_dev,
						 endpoint->bInterval);
			} else {
				usb_fill_bulk_urb(my_midi_dev->in_urb, udev,
						  usb_rcvbulkpipe(udev, endpoint->bEndpointAddress),
						  my_midi_dev->in_buffer, MIDI_USB_IN_BUF_SIZE,
						  midi_usb_read_callback, my_midi_dev);
			}
			my_midi_dev->in_urb->transfer_dma = my_midi_dev->in_dma;
			my_midi_dev->in_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

			ret = usb_submit_urb(my_midi_dev->in_urb, GFP_KERNEL);
			if (ret) goto error;

			usb_set_intfdata(interface, my_midi_dev);
			pr_info(DEVICE_NAME ": USB MIDI IN endpoint found and URB submitted\n");
			return 0;
		}
	}

error:
	if (my_midi_dev->in_urb) usb_free_urb(my_midi_dev->in_urb);
	if (my_midi_dev->in_buffer) usb_free_coherent(udev, MIDI_USB_IN_BUF_SIZE, my_midi_dev->in_buffer, my_midi_dev->in_dma);
	usb_put_dev(my_midi_dev->udev);
	kfree(my_midi_dev);
	my_midi_dev = NULL;
	return -ENODEV;
}

static void midi_usb_disconnect(struct usb_interface *interface)
{
	struct midi_usb_device *dev = usb_get_intfdata(interface);

	if (dev) {
		usb_set_intfdata(interface, NULL);
		usb_kill_urb(dev->in_urb);
		usb_free_urb(dev->in_urb);
		usb_free_coherent(dev->udev, MIDI_USB_IN_BUF_SIZE, dev->in_buffer, dev->in_dma);
		usb_put_dev(dev->udev);
		kfree(dev);
		my_midi_dev = NULL;
		pr_info(DEVICE_NAME ": USB MIDI device disconnected\n");
	}
}

static const struct usb_device_id midi_usb_table[] = {
	{ USB_INTERFACE_INFO(USB_CLASS_AUDIO, 3, 0) }, /* Audio, MIDI Streaming */
	{ USB_DEVICE(0x0499, 0x1604) }, /* Yamaha PSR A2000 Digital Keyboard */
	{ USB_DEVICE(0x0499, 0x1038) }, /* Yamaha UX16 or similar */
	{ USB_DEVICE(0x0499, 0x1039) }, /* Yamaha */
	{ USB_DEVICE(0x0499, 0x1000) }, /* Yamaha */
	{ } /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, midi_usb_table);

static struct usb_driver midi_usb_driver = {
	.name       = "custom_usb_midi",
	.id_table   = midi_usb_table,
	.probe      = midi_usb_probe,
	.disconnect = midi_usb_disconnect,
};

int usb_midi_init(void)
{
	int ret;
	
	/* Dynamically allocate FIFO buffer */
	ret = kfifo_alloc(&midi_fifo, FIFO_SIZE, GFP_KERNEL);
	if (ret) {
		pr_err(DEVICE_NAME ": failed to allocate FIFO\n");
		return ret;
	}

	/* Register USB driver */
	ret = usb_register(&midi_usb_driver);
	if (ret) {
		pr_err(DEVICE_NAME ": usb_register failed\n");
		kfifo_free(&midi_fifo);
		return ret;
	}

	pr_info(DEVICE_NAME ": USB MIDI driver initialized\n");
	return 0;
}

void usb_midi_exit(void)
{
	usb_deregister(&midi_usb_driver);
	kfifo_free(&midi_fifo);
	pr_info(DEVICE_NAME ": USB MIDI driver stopped\n");
}
