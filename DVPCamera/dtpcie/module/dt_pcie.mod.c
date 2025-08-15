#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
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

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x2c635209, "module_layout" },
	{ 0x2f2c95c4, "flush_work" },
	{ 0xf435dd31, "kernel_write" },
	{ 0x37ce6741, "cdev_del" },
	{ 0x30a93ed, "kmalloc_caches" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0x51b1c11d, "cdev_init" },
	{ 0xc4f0da12, "ktime_get_with_offset" },
	{ 0x2caa7bd4, "pci_free_irq_vectors" },
	{ 0x754d539c, "strlen" },
	{ 0x54b1fac6, "__ubsan_handle_load_invalid_value" },
	{ 0xd14f89a0, "remap_vmalloc_range" },
	{ 0x42d723fb, "dma_set_mask" },
	{ 0x69449ab2, "pci_disable_device" },
	{ 0x56470118, "__warn_printk" },
	{ 0xdff7669f, "device_destroy" },
	{ 0xf8c7264e, "filp_close" },
	{ 0xa0bcb084, "pci_release_regions" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x999e8297, "vfree" },
	{ 0xa648e561, "__ubsan_handle_shift_out_of_bounds" },
	{ 0x97651e6c, "vmemmap_base" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x71038ac7, "pv_ops" },
	{ 0x1e0252cf, "dma_set_coherent_mask" },
	{ 0x851c15cf, "kthread_create_on_node" },
	{ 0x7b75a9f1, "__pskb_pull_tail" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x9591a9f, "kernel_read" },
	{ 0xaff2dd5d, "pci_set_master" },
	{ 0xf9c0b663, "strlcat" },
	{ 0xf7242b1e, "pci_alloc_irq_vectors_affinity" },
	{ 0xfb578fc5, "memset" },
	{ 0xd35cce70, "_raw_spin_unlock_irqrestore" },
	{ 0x4c9f47a5, "current_task" },
	{ 0xcefb0c9f, "__mutex_init" },
	{ 0xa0aa95d4, "kthread_stop" },
	{ 0x449ad0a7, "memcmp" },
	{ 0x9ec6ca96, "ktime_get_real_ts64" },
	{ 0xde80cd09, "ioremap" },
	{ 0x4c9d28b0, "phys_base" },
	{ 0xe4c2c66c, "rtc_ktime_to_tm" },
	{ 0x9166fada, "strncpy" },
	{ 0x4dfa8d4b, "mutex_lock" },
	{ 0x8c03d20c, "destroy_workqueue" },
	{ 0xcf2a881c, "device_create" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0xabac4112, "cdev_add" },
	{ 0x800473f, "__cond_resched" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x87a21cb3, "__ubsan_handle_out_of_bounds" },
	{ 0x5635a60a, "vmalloc_user" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x8ddd8aad, "schedule_timeout" },
	{ 0x92997ed8, "_printk" },
	{ 0x3f68deca, "dma_map_page_attrs" },
	{ 0x14d83f6a, "dev_driver_string" },
	{ 0x9ce84760, "wake_up_process" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0x1c937e3f, "pci_unregister_driver" },
	{ 0xaf88e69b, "kmem_cache_alloc_trace" },
	{ 0xba8fbd64, "_raw_spin_lock" },
	{ 0x34db050b, "_raw_spin_lock_irqsave" },
	{ 0x148653, "vsnprintf" },
	{ 0x640663dd, "pci_irq_vector" },
	{ 0x3eeb2322, "__wake_up" },
	{ 0xb3f7646e, "kthread_should_stop" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0x37a0cba, "kfree" },
	{ 0x7394b519, "remap_pfn_range" },
	{ 0x69acdf38, "memcpy" },
	{ 0xe3f4c62c, "pci_request_regions" },
	{ 0xedc03953, "iounmap" },
	{ 0x2554c13b, "__pci_register_driver" },
	{ 0x3ee2b36d, "class_destroy" },
	{ 0xbcaa1d60, "dma_unmap_page_attrs" },
	{ 0x92540fbf, "finish_wait" },
	{ 0xc5b6f236, "queue_work_on" },
	{ 0x656e4a6e, "snprintf" },
	{ 0xb0e602eb, "memmove" },
	{ 0xa74ccb9b, "pci_enable_device" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x9b0fb107, "__class_create" },
	{ 0x49cd25ed, "alloc_workqueue" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xc31db0ce, "is_vmalloc_addr" },
	{ 0xc1514a3b, "free_irq" },
	{ 0xe914e41e, "strcpy" },
	{ 0x98303217, "filp_open" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("pci:v000010EEd00008888sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00001901sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00001902sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00001903sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00001920sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00001921sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00001922sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00001923sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00000400sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00000800sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00000120sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00000140sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00000160sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00000180sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00000220sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00000240sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00000260sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00000280sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00000320sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00000340sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00000360sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00000380sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00000242sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "F28E2F6D6ED0444CC87D45F");
