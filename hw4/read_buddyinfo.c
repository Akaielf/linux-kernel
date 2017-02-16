#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>		/* for the macros reference */
#include <linux/kmod.h>		/* for call_usermodehelper() */
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <asm/segment.h>
#include <asm/uaccess.h>

#define PROCFS_MAX_SIZE		1024
#define PROCFS_NAME		"buffer1k"

struct file* file_open(const char* path, int flags, int rights) {
	struct file* filp = NULL;
	mm_segment_t oldfs;
	int err = 0;

	oldfs = get_fs();
	set_fs(get_ds());
	filp = filp_open(path, flags, rights);
	set_fs(oldfs);
	if(IS_ERR(filp)) {
		err = PTR_ERR(filp);
		return NULL;
	}
	return filp;
}

void file_close(struct file* file) {
	filp_close(file, NULL);
}

int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(get_ds());

	ret = vfs_read(file, data, size, &offset);

	set_fs(oldfs);
	return ret;
}

int file_sync(struct file* file) {
	vfs_fsync(file, 0);
	return 0;
}

/* save the content of /proc/buddyinfo into a new file */
int cat_buddyinfo(void) {
	char* argv[4];
	char* envp[4];

	argv[0] = "/bin/bash";
	argv[1] = "-c";
	argv[2] = "/bin/cat /proc/buddyinfo > /home/kh276/buddyinfo";
	argv[3] = NULL;

	envp[0] = "HOME=/";
	envp[1] = "TERM=linux";
	envp[2] = "PATH=/sbin:/usr/sbin:/bin:/usr/bin";
	envp[3] = NULL;

	call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);

	return 0;	
}

/* the original buddyinfo has already been registered, so we can not access it with proc_fs */
/* all the /proc files are not real files, so we need to cat the infomation into a new file */
/* and then read the new file */
static int __init meminfo_init(void) {
	struct file *fp;
	unsigned char buff[1024];
	int ret = 0;

	printk(KERN_INFO "Akai: Loading read_buddyinfo module \n");
	printk(KERN_INFO "Akai: Performing cat /proc/buddyinfo into /home/kh276/buddyinfo \n");
	cat_buddyinfo();

	printk(KERN_INFO "Akai: Reading new buddyinfo file \n");
	fp = file_open("/home/kh276/buddyinfo", O_RDONLY|O_CREAT, 0644);
	ret = file_read(fp, 0, buff, 1024);
	if (ret == 0) {
		printk(KERN_INFO "Akai: Failed to read /proc/buddyinfo \n");
		goto END;
	}
	printk(KERN_INFO "Akai: buddyinfo: %s \n", buff);
	printk(KERN_INFO "Akai: finished reading /proc/buddinfo \n");

	file_close(fp);

END:
	return 0;
}

static void __exit meminfo_exit(void) {
	printk(KERN_INFO "Akai: Exiting read_buddyinfo module \n");
}

module_init(meminfo_init);
module_exit(meminfo_exit);
