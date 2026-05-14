#ifndef MIDI_CORE_H
#define MIDI_CORE_H

#include <linux/fs.h>
#include <linux/kfifo.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>

/* Module Constants */
#define DEVICE_NAME "custom_midi"
#define CLASS_NAME  "midi_class"
#define FIFO_SIZE   1024

/*
 * Person 3 (Hardware/Concurrency) defines these globally in hw_sim.c
 * so that Person 2 (FOPS) can use them.
 */
extern struct kfifo midi_fifo;
extern spinlock_t fifo_lock;
extern struct mutex midi_mutex;
extern wait_queue_head_t midi_wq;

/*
 * Person 2 (FOPS) exports the file operations structure here.
 */
extern const struct file_operations midi_fops;

/*
 * Person 3 exports the setup/teardown functions for the hardware communication.
 */
int usb_midi_init(void);
void usb_midi_exit(void);

#endif /* MIDI_CORE_H */
