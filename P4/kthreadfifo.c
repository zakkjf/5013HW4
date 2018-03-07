#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched/signal.h>
#include <linux/kfifo.h>
#include <linux/mutex.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zach Farmer");
MODULE_DESCRIPTION("Kfifo Example Across Threads");
MODULE_VERSION("0.01");

#define FIFO_SIZE 512

#define next_process(p)    list_entry((p)->tasks.next, struct task_struct, tasks)

#define prev_process(p)    list_entry((p)->tasks.prev, struct task_struct, tasks)

static struct task_struct *thread_ga;
static struct task_struct *thread_lg;

struct mutex fifo_mutex; /* shared between the threads */

struct kfifo kf;

// Function executed by kernel thread
static int thread_gather(void *unused)
{
    char str[256];
    //int foo;
    //struct { unsigned char buf[6]; } hello = { "hello" };
    // Allow the SIGKILL signal
    allow_signal(SIGKILL);
    while (!kthread_should_stop())
    {
        printk(KERN_INFO "Thread Running\n");
	//sleep for 5 seconds
        ssleep(5);

	//grab all the current/previous/next pids and vruntime and parse it into a string
	snprintf(str,256,"Current Process: %d\nVruntime: %llu\nPrevious Process: %d\nVruntime: %llu\nNext Process: %d\nVruntime: %llu\n", 
		current->pid, current->se.vruntime,
		next_process(current)->pid,
		next_process(current)->se.vruntime,
		prev_process(current)->pid,
		prev_process(current)->se.vruntime);

	printk(KERN_INFO "Creating Thread\n");

	//lock the fifo and write to it
	mutex_lock(&fifo_mutex);
	kfifo_in(&kf, str, 256);
	mutex_unlock(&fifo_mutex);

        if (signal_pending(thread_ga))
            break;
    }
    printk(KERN_INFO "Thread Stopping\n");
    do_exit(0);
    return 0;
}

// Function executed by kernel thread
static int thread_log(void *unused)
{
    char str[256];

    // Allow the SIGKILL signal
    allow_signal(SIGKILL);
    while (!kthread_should_stop())
    {
        printk(KERN_INFO "Thread Running\n");
        ssleep(5);

	//lock the fifo and read from it
	mutex_lock(&fifo_mutex);
	kfifo_out(&kf, str, 256);
	mutex_unlock(&fifo_mutex);

	//dump the fifo to the dmesg console
	printk(KERN_INFO "Log: %s", str);

        // Check if the signal is pending
        if (signal_pending(thread_lg))
            break;
    }
    printk(KERN_INFO "Thread Stopping\n");
    do_exit(0);
    return 0;
}

// Module Initialization
static int __init init_thread(void)
{
    printk(KERN_INFO "Creating Thread\n");

    //init mutex for fifo
    mutex_init(&fifo_mutex);

    //allocate kfifo
    if(kfifo_alloc(&kf, FIFO_SIZE, GFP_KERNEL))
	printk(KERN_ERR "error kfifo_alloc\n");

    //Create the kernel thread with name "gather thread"
    thread_ga = kthread_run(thread_gather, NULL, "gather thread");
    if (thread_ga)
        printk(KERN_INFO "Gather: thread created successfully\n");
    else
        printk(KERN_ERR "Gather: thread creation failed\n");

    //Create the kernel thread with name "log thread"
    thread_lg = kthread_run(thread_log, NULL, "log thread");
    if (thread_lg)
        printk(KERN_INFO "Log: thread created successfully\n");
    else
        printk(KERN_ERR "Log: thread creation failed\n");

    
    return 0;
}
// Module Exit
static void __exit cleanup_thread(void)
{
   printk(KERN_INFO "Cleaning Up\n");
   if (thread_ga)
   {
       kfifo_free(&kf);
       kthread_stop(thread_ga);
       kthread_stop(thread_lg);
       printk(KERN_INFO "All threads stopped");
   }
}

MODULE_LICENSE("GPL");
module_init(init_thread);
module_exit(cleanup_thread);
