#include "midi_core.h"
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/kernel.h>

/* Global state for the Core (Declared in midi_core.h) */
struct kfifo midi_fifo;
DEFINE_SPINLOCK(fifo_lock);
DEFINE_MUTEX(midi_mutex);
DECLARE_WAIT_QUEUE_HEAD(midi_wq);

/* Internal Hardware Simulation State */
static struct timer_list midi_timer;
#define TIMER_INTERVAL_MS 500

/* Simulated MIDI stream (Yamaha Organ Note On/Off/CC) */
static const u8 simulated_midi_data[] = {
	0x90, 0x3C, 0x7F,
	0x80, 0x3C, 0x00,
	0xB0, 0x0B, 0x64,
	0xC0, 0x12,
	0xFE
};
static unsigned int sim_index;

static void midi_timer_callback(struct timer_list *t)
{
	unsigned long flags;
	int bytes_to_push = (jiffies % 4) + 1; /* Randomize burst size */
	int i;

	spin_lock_irqsave(&fifo_lock, flags);
	for (i = 0; i < bytes_to_push; i++) {
		if (kfifo_avail(&midi_fifo) == 0) {
			pr_warn(DEVICE_NAME ": FIFO full – dropping byte\n");
		} else {
			kfifo_in(&midi_fifo, &simulated_midi_data[sim_index], 1);
		}
		sim_index = (sim_index + 1) % ARRAY_SIZE(simulated_midi_data);
	}
	spin_unlock_irqrestore(&fifo_lock, flags);

	/* Wake up waiting processes */
	wake_up_interruptible(&midi_wq);

	/* Reschedule timer */
	mod_timer(&midi_timer, jiffies + msecs_to_jiffies(TIMER_INTERVAL_MS));
}

int hw_sim_init(void)
{
	int ret;
	
	/* Dynamically allocate FIFO buffer */
	ret = kfifo_alloc(&midi_fifo, FIFO_SIZE, GFP_KERNEL);
	if (ret) {
		pr_err(DEVICE_NAME ": failed to allocate FIFO\n");
		return ret;
	}

	/* Initialize Timer ISR */
	sim_index = 0;
	timer_setup(&midi_timer, midi_timer_callback, 0);
	mod_timer(&midi_timer, jiffies + msecs_to_jiffies(TIMER_INTERVAL_MS));

	pr_info(DEVICE_NAME ": hardware simulation initialized\n");
	return 0;
}

void hw_sim_exit(void)
{
	/* Stop timer before freeing memory */
	del_timer_sync(&midi_timer);
	kfifo_free(&midi_fifo);
	pr_info(DEVICE_NAME ": hardware simulation stopped\n");
}
