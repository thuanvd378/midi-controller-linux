#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x9f222e1e, "alloc_chrdev_region" },
	{ 0xa61fd7aa, "__check_object_size" },
	{ 0x092a35a2, "_copy_from_user" },
	{ 0xd710adbf, "__kmalloc_noprof" },
	{ 0x562e3aaa, "__kfifo_in" },
	{ 0xc87f4bab, "finish_wait" },
	{ 0xfbc10eaa, "class_destroy" },
	{ 0xcb8b6ec6, "kfree" },
	{ 0x0db8d68d, "prepare_to_wait_event" },
	{ 0x2352b148, "timer_delete_sync" },
	{ 0x16ab4215, "__wake_up" },
	{ 0x283bce08, "__kfifo_free" },
	{ 0xe1e1f979, "_raw_spin_lock_irqsave" },
	{ 0xd272d446, "__fentry__" },
	{ 0xe8213e80, "_printk" },
	{ 0xd272d446, "schedule" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0x90a48d82, "__ubsan_handle_out_of_bounds" },
	{ 0x7db91808, "cdev_add" },
	{ 0x7a5ffe84, "init_wait_entry" },
	{ 0x32feeafc, "mod_timer" },
	{ 0xa2e1228b, "device_create" },
	{ 0x3d568d84, "class_create" },
	{ 0x357aaab3, "mutex_lock_interruptible" },
	{ 0x81a1a811, "_raw_spin_unlock_irqrestore" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x092a35a2, "_copy_to_user" },
	{ 0x058c185a, "jiffies" },
	{ 0x0bc5fb0d, "unregister_chrdev_region" },
	{ 0xf46d5bf3, "mutex_unlock" },
	{ 0x02f9bbf0, "init_timer_key" },
	{ 0x88b4fdc1, "device_destroy" },
	{ 0xb3d1d601, "__kfifo_alloc" },
	{ 0xd1ea1c88, "__kfifo_out" },
	{ 0xb06a91bd, "cdev_init" },
	{ 0x7851be11, "__SCT__might_resched" },
	{ 0x6ce3748e, "cdev_del" },
	{ 0x70eca2ca, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x9f222e1e,
	0xa61fd7aa,
	0x092a35a2,
	0xd710adbf,
	0x562e3aaa,
	0xc87f4bab,
	0xfbc10eaa,
	0xcb8b6ec6,
	0x0db8d68d,
	0x2352b148,
	0x16ab4215,
	0x283bce08,
	0xe1e1f979,
	0xd272d446,
	0xe8213e80,
	0xd272d446,
	0xd272d446,
	0x90a48d82,
	0x7db91808,
	0x7a5ffe84,
	0x32feeafc,
	0xa2e1228b,
	0x3d568d84,
	0x357aaab3,
	0x81a1a811,
	0xd272d446,
	0x092a35a2,
	0x058c185a,
	0x0bc5fb0d,
	0xf46d5bf3,
	0x02f9bbf0,
	0x88b4fdc1,
	0xb3d1d601,
	0xd1ea1c88,
	0xb06a91bd,
	0x7851be11,
	0x6ce3748e,
	0x70eca2ca,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"alloc_chrdev_region\0"
	"__check_object_size\0"
	"_copy_from_user\0"
	"__kmalloc_noprof\0"
	"__kfifo_in\0"
	"finish_wait\0"
	"class_destroy\0"
	"kfree\0"
	"prepare_to_wait_event\0"
	"timer_delete_sync\0"
	"__wake_up\0"
	"__kfifo_free\0"
	"_raw_spin_lock_irqsave\0"
	"__fentry__\0"
	"_printk\0"
	"schedule\0"
	"__stack_chk_fail\0"
	"__ubsan_handle_out_of_bounds\0"
	"cdev_add\0"
	"init_wait_entry\0"
	"mod_timer\0"
	"device_create\0"
	"class_create\0"
	"mutex_lock_interruptible\0"
	"_raw_spin_unlock_irqrestore\0"
	"__x86_return_thunk\0"
	"_copy_to_user\0"
	"jiffies\0"
	"unregister_chrdev_region\0"
	"mutex_unlock\0"
	"init_timer_key\0"
	"device_destroy\0"
	"__kfifo_alloc\0"
	"__kfifo_out\0"
	"cdev_init\0"
	"__SCT__might_resched\0"
	"cdev_del\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "E0A19B17C19EB57233793CB");
