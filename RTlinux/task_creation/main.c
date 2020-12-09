#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/kthread.h>

static struct task_struct *thread_st;
// Function executed by kernel thread
static int thread_fn(void *unused)
{
    while (!kthread_should_stop())
    {
        printk(KERN_ALERT "Thread Running\n");
        ssleep(5);
    }
    printk(KERN_ALERT "Thread Stopping\n");
    //do_exit(0);
    return 0;
}
// Module Initialization
static int init_thread(void)
{
    printk(KERN_ALERT "Creating Thread\n");
    //Create the kernel thread with name 'mythread'
    thread_st = kthread_create(thread_fn, NULL, "mythread");
    if (thread_st)
    {
        printk(KERN_ALERT "Thread Created successfully\n");
        wake_up_process(thread_st);
    }
    else
        printk(KERN_ALERT "Thread creation failed\n");
    return 0;
}
// Module Exit
static void cleanup_thread(void)
{       
    kthread_stop(thread_st);
    printk("Cleaning Up\n");
}

module_init(init_thread);
module_exit(cleanup_thread);
//https://sysplay.in/blog/linux-kernel-internals/2015/04/kernel-threads/
