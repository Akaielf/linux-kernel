#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>		/* for the macros reference */

static int hello_2_data __initdata = 2;

static int __init hello_2_init(void) {
	printk(KERN_INFO "My Name: Loading Hello2 module - Hello World %d \n", hello_2_data);
	return 0;
}

static void __exit hello_2_exit(void) {
	printk(KERN_INFO "My Name: Exiting Hello2 module - Goodbye World 2 \n");
}

module_init(hello_2_init);
module_exit(hello_2_exit);
