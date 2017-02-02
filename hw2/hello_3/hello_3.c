#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>		/* for the macros reference */
#include <linux/module.h>	/* for module_param macros */

/* according to the Linux community, MODULE_PARM should be replaced with module_param after kernel 2.6 */

static char* name = "My Name";
static int zip = 0;

module_param(name, charp, S_IRUSR);
module_param(zip, int, S_IRUSR);

static int __init hello_3_init(void) {
	printk(KERN_INFO "Loading Hello 3: Hello World 3 \n");
	printk(KERN_INFO "My Name: %s \n", name);
	printk(KERN_INFO "ZIP code: %d \n", zip);

	return 0;
}

static void __exit hello_3_exit(void) {
	printk(KERN_INFO "My Name: Exiting Hello3 module - Goodbye World 3 \n");
}

module_init(hello_3_init);
module_exit(hello_3_exit);
