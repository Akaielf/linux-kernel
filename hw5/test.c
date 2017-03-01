#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>		/* for the macros reference */
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/memcontrol.h>
#include "slab.h"

/* slabinfo - version: 2.1
# name            <active_objs> <num_objs> <objsize> <objperslab> <pagesperslab> : tunables <limit> <batchcount> <sharedfactor> : slabdata <active_slabs> <num_slabs> <sharedavail> 
kmalloc-4194304        0          0         4194304        1         1024 : tunables    1    1    0 : slabdata      0      0      0
kmalloc-2097152        0          0         2097152        1          512 : tunables    1    1    0 : slabdata      0      0      0
kmalloc-1048576        0          0         1048576        1          256 : tunables    1    1    0 : slabdata      0      0      0
kmalloc-524288         0          0         524288         1          128 : tunables    1    1    0 : slabdata      0      0      0
kmalloc-262144         1          1         262144         1           64 : tunables    1    1    0 : slabdata      1      1      0
kmalloc-131072         4          4         131072         1           32 : tunables    8    4    0 : slabdata      4      4      0
kmalloc-65536          2          2         65536          1           16 : tunables    8    4    0 : slabdata      2      2      0
kmalloc-32768          0          0         32768          1            8 : tunables    8    4    0 : slabdata      0      0      0
kmalloc-16384          3          3         16384          1            4 : tunables    8    4    0 : slabdata      3      3      0
kmalloc-8192          19         19         8192           1            2 : tunables    8    4    0 : slabdata     19     19      0
kmalloc-4096          44         44         4096           1            1 : tunables   24   12    0 : slabdata     44     44      0
kmalloc-2048         190        190         2048           2            1 : tunables   24   12    0 : slabdata     95     95      0
kmalloc-1024         453        456         1024           4            1 : tunables   54   27    0 : slabdata    114    114      0
kmalloc-512          282        352         512            8            1 : tunables   54   27    0 : slabdata     44     44      0
kmalloc-256          223        224         256           16            1 : tunables  120   60    0 : slabdata     14     14      0
kmalloc-192          707        756         192           21            1 : tunables  120   60    0 : slabdata     36     36      0
kmalloc-96           487        527         128           31            1 : tunables  120   60    0 : slabdata     17     17      0
kmalloc-64          1776       1890         64            63            1 : tunables  120   60    0 : slabdata     30     30      0
kmalloc-128          689        744         128           31            1 : tunables  120   60    0 : slabdata     24     24      0
kmalloc-32          5424       5456         32           124            1 : tunables  120   60    0 : slabdata     44     44      0 */

/* struct slabinfo {
        unsigned long active_objs;
        unsigned long num_objs;
        unsigned long active_slabs;
        unsigned long num_slabs;
        unsigned long shared_avail;
        unsigned int limit;
        unsigned int batchcount;
        unsigned int shared;
        unsigned int objects_per_slab;
        unsigned int cache_order;
}; */

/*
 * struct array_cache
 *
 * Purpose:
 * - LIFO ordering, to hand out cache-warm objects from _alloc
 * - reduce the number of linked list operations
 * - reduce spinlock operations
 *
 * The limit is stored in the per-cpu structure to reduce the data cache
 * footprint.
 *
 */
struct array_cache {
        unsigned int avail;
        unsigned int limit;
        unsigned int batchcount;
        unsigned int touched;
        spinlock_t lock;
        void *entry[];  /*
                         * Must have this definition in here for the proper
                         * alignment of array_cache. Also simplifies accessing
                         * the entries.
                         *
                         * Entries should not be directly dereferenced as
                         * entries belonging to slabs marked pfmemalloc will
                         * have the lower bits set SLAB_OBJ_PFMEMALLOC
                         */
};

static void check_irq_on(void)
{
        BUG_ON(irqs_disabled());
}

static int get_pages_per_slab(int order) {
	return 1 << order;
}

static int get_objsize(int order) {
	int ret;

	// special cases for kmem_cache index 1 and 2
	if (order == 1) {
		ret = 96;
	} else if (order == 2) {
		ret = 192;
	} else {
		ret = 1 << order;
	}

	return ret;
}

void show_slabinfo(struct kmem_cache *cachep, int index) {
	struct page *page;
        unsigned long active_objs;
        unsigned long num_objs;
        unsigned long active_slabs = 0;
        unsigned long num_slabs, free_objects = 0, shared_avail = 0;
        const char *name;
        char *error = NULL;
        int node;
        struct kmem_cache_node *n;

        active_objs = 0;
        num_slabs = 0;
	for_each_online_node(node) {
                n = cachep->node[node];
                if (!n)
                        continue;

                check_irq_on();
                spin_lock_irq(&n->list_lock);

                list_for_each_entry(page, &n->slabs_full, lru) {
                        if (page->active != cachep->num && !error)
                                error = "slabs_full accounting error";
                        active_objs += cachep->num;
                        active_slabs++;
                }
                list_for_each_entry(page, &n->slabs_partial, lru) {
                        if (page->active == cachep->num && !error)
                                error = "slabs_partial accounting error";
                        if (!page->active && !error)
                                error = "slabs_partial accounting error";
                        active_objs += page->active;
                        active_slabs++;
                }
                list_for_each_entry(page, &n->slabs_free, lru) {
                        if (page->active && !error)
                                error = "slabs_free accounting error";
                        num_slabs++;
                }
		free_objects += n->free_objects;
                if (n->shared) {
                        shared_avail += n->shared->avail;
		}

                spin_unlock_irq(&n->list_lock);
        }
        num_slabs += active_slabs;
        num_objs = num_slabs * cachep->num;
        if (num_objs - active_objs != free_objects && !error)
                error = "free_objects accounting error";

        name = cachep->name;
        if (error) {
                printk(KERN_ERR "Akai: slab: cache %s error: %s\n", name, error);
	}

	printk(KERN_INFO "Akai: cache name %s active_objs: %lu, num_objs: %lu, objsize: %u, objects_per_slab: %u, pages_per_slab: %u \n", cachep->name, active_objs, num_objs, get_objsize(index), cachep->num, get_pages_per_slab(cachep->gfporder));
	printk(KERN_INFO "Akai: tunable: <limit>: %u, <batchcount>: %u, <sharedfactor>: %u \n", cachep->limit, cachep->batchcount, cachep->shared);
	printk(KERN_INFO "Akai: slabdata: <active_slabs>: %lu, <num_slabs>: %lu, <sharedavail>: %lu \n\n", active_slabs, num_slabs, shared_avail);
	
        /*sinfo->active_objs = active_objs;
        sinfo->num_objs = num_objs;
        sinfo->active_slabs = active_slabs;
        sinfo->num_slabs = num_slabs;
        sinfo->shared_avail = shared_avail;
        sinfo->limit = cachep->limit;
        sinfo->batchcount = cachep->batchcount;
        sinfo->shared = cachep->shared;
        sinfo->objects_per_slab = cachep->num;
        sinfo->cache_order = cachep->gfporder;*/
}

static int __init test_init(void) {
	int i;
        struct kmem_cache *s;

        for (i = 0; i<= KMALLOC_SHIFT_HIGH; i++) {
		s = kmalloc_caches[i];
                // cache 0 is usually not accessible, so we skip it
                if (!s) {
                        continue;
                }
		show_slabinfo(s, i);
	}

	return 0;
}

static void __exit test_exit(void) {
	printk(KERN_INFO "Akai: Exiting test module \n");
}

module_init(test_init);
module_exit(test_exit);
