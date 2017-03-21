#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>		/* for the macros reference */

#include <linux/types.h>
#include <linux/sched.h>	/* for struct task_struct */
#include <linux/smpboot.h>	/* for smp_hotplug_thread */
#include <asm-generic/bug.h>	/* for BUG_ON macro */

static void Akai_run_hotplug(unsigned int cpu);
static void Akai_thread_create_1(void);

DEFINE_PER_CPU(struct task_struct *, ksoftirqd);
static struct smp_hotplug_thread akai_softirq_threads = {
        .store                  = &ksoftirqd,
        //.thread_should_run      = ksoftirqd_should_run,
        .thread_fn              = Akai_run_hotplug,
        .thread_comm            = "ksoftirqd/%u",
};

static void Akai_run_hotplug(unsigned int cpu) {
	printk(KERN_NOTICE "Akai: running Akai hotplug %d \n", cpu);
}

static __init int spawn_my_hotplug_threads(void){
         BUG_ON(smpboot_register_percpu_thread(&akai_softirq_threads));
         return 0;
}

pid_t kernel_thread(int (*fn)(void *), void *arg, unsigned long flags)
{
	//return do_fork(flags|CLONE_VM|CLONE_UNTRACED, (unsigned long)fn, (unsigned long)arg, NULL, NULL);
	return 1;
}

static int Akai_kthread_1(void* unused) {
	struct task_struct *cur_task = current;
	// comment / name of the process
	strcpy(cur_task->comm, "Akai: Akai_kthread_1");
	set_task_state(cur_task, TASK_RUNNING);
	printk(KERN_NOTICE "Akai: Akai_kthread_1 is about to be scheduled \n");
	schedule();
	printk(KERN_NOTICE "Akai: Akai_kthread_1 is now scheduled \n");

	return 0;
}

static void Akai_thread_create_1(void) {
	int my_pid;
	printk(KERN_NOTICE "Akai: calling kernel_thread(Akai_kthread_1) \n");
	my_pid = kernel_thread(Akai_kthread_1, NULL, CLONE_FS);
	printk(KERN_NOTICE "Akai: Akai_kthread_1 = %d \n", my_pid);
}

static int __init test_init(void) {
	struct task_struct *p;
	extern struct pid_namespace init_pid_ns;

	printk(KERN_INFO "Akai: Loading test module \n");
	p = current;
	for_each_process(p) {
		printk(KERN_INFO "Akai: pid = %d \n", task_pid_nr(p));
		printk(KERN_INFO "Akai: stime = %lu \n", p->stime);
		//printk(KERN_INFO "Akai: ppid = %d \n", task_ppid_nr(p));
	}
	Akai_thread_create_1();
	spawn_my_hotplug_threads();

	return 0;
}

static void __exit test_exit(void) {
	printk(KERN_INFO "Akai: Exiting test module \n");
}

module_init(test_init);
module_exit(test_exit);
