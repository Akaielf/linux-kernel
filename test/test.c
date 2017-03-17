#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>		/* for the macros reference */

static int __init test_init(void) {
	printk(KERN_INFO "Akai: Loading test module \n");
	return 0;
}

static void __exit test_exit(void) {
	printk(KERN_INFO "Akai: Exiting test module \n");
}

module_init(test_init);
module_exit(test_exit);
