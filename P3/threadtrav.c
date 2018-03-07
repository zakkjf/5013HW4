#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/sched.h>
 
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zach Farmer");
MODULE_DESCRIPTION("Traverse Thread Tree");
MODULE_VERSION("0.01");

void device_exit(void);
int device_init(void);

module_init(device_init);
module_exit(device_exit);
 
int print_info(struct task_struct *task)
{
	int childcount = 0;
	struct list_head *list;

	list_for_each(list, &task->children) 
	{
	
		childcount++;
	
	}

	printk(KERN_NOTICE "assignment: current process: %s, PID: %d, State: %ld, Child count: %d, Nice: %d", task->comm, task->pid, task->state, childcount, task_nice(task));

	return 0;
}
int device_init(void)
{
    printk(KERN_NOTICE "Starting Traverse Module");
    struct task_struct *task = current; // grab current task
   
    print_info(task);

    do
    {
        task = task->parent;
        print_info(task);

    } while (task->pid != 0);

    return 0;
}
 
void device_exit(void) {
  printk(KERN_NOTICE "Exiting Traverse Module");
}
