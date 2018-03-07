#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/errno.h>   /* error codes */
#include <linux/sched.h>
 
 
MODULE_LICENSE("Dual BSD/GPL");
 
/* Declaration of functions */
void device_exit(void);
int device_init(void);
   
/* Declaration of the init and exit routines */
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
    struct task_struct *task = current; // getting global current pointer 
   
    print_info(task);

    do
    {
        task = task->parent;
        print_info(task);

    } while (task->pid != 0);

    return 0;
}
 
void device_exit(void) {
  printk(KERN_NOTICE "assignment: exiting module");
}
