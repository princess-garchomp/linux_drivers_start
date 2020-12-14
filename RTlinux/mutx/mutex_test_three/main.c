#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/timekeeping.h>
//#include <linux/time.h>

int counter=1;
struct task_struct *thread_st;
struct task_struct *thread_st_2;

struct timespec64 timetime;
//ktime_get_real_ts64(struct timespec64 *tv);
time64_t enter_one;
time64_t new_one;


// Function executed by kernel thread ONE
static int thread_fn(void *unused)
{
    ktime_get_real_ts64(&timetime);
    enter_one=timetime.tv_sec;
    //stay in this while loop for 3 seconds
    while ((new_one-enter_one)<3)
    {
	ktime_get_real_ts64(&timetime);
	printk(KERN_ALERT "Thread ONE Running. counter: %d. time: %lld\n",counter,timetime.tv_sec);
	new_one=timetime.tv_sec;
	if(counter!=0)
	{
        counter++;
	}
	else if(counter ==4000000)
	{
	printk(KERN_ALERT "form thread one, counter reached 4000000. counter: %d. time: %lld\n",counter,timetime.tv_sec);
	counter = 0;
	}
	else
	{
	}
//	ssleep(1);
    }
    while (!kthread_should_stop())
    {
    	    printk(KERN_ALERT "Thread ONE sleeping\n");
	    ssleep(5);
    }
    printk(KERN_ALERT "Thread ONE Stopping\n");
    return 0;
}


// Function executed by kernel thread TWO
static int thread_fn_2(void *unused)
{
    while (!kthread_should_stop())
    {
	ktime_get_real_ts64(&timetime);
        //printk(KERN_ALERT "Thread TWO Running. counter: %d\n",counter);
	printk(KERN_ALERT "Thread TWO Running. counter: %d. time: %lld\n",counter,timetime.tv_sec);
	ssleep(1);
	counter=0;
    }
    printk(KERN_ALERT "Thread two Stopping\n");
    return 0;
}



// Module Initialization
static int init_thread(void)
{
    printk(KERN_ALERT "Creating Thread\n");
    //Create the kernel thread with name 'mythread'
    thread_st = kthread_create(thread_fn, NULL, "mythread 1");
    if (thread_st)
    {
        printk(KERN_ALERT "Thread ONE Created successfully\n");
//	thread_st->rt_priority=0;
	thread_st->prio=0;
//	thread_st->static_prio=0;

        wake_up_process(thread_st);
    }
    else
    {
        printk(KERN_ALERT "Thread ONE creation failed\n");
    }

    thread_st_2 = kthread_create(thread_fn_2, NULL, "mythread 2");
    if (thread_st_2)
    {
        printk(KERN_ALERT "Thread TWO Created successfully\n");
//        thread_st_2->rt_priority=99;
	thread_st->prio=99;
//	thread_st->static_prio=99;

        wake_up_process(thread_st_2);
    }
    else
    {
        printk(KERN_ALERT "Thread TWO creation failed\n");
    }
    return 0;
}


// Module Exit
static void cleanup_thread(void)
{
    kthread_stop(thread_st);
    kthread_stop(thread_st_2);
    printk("Cleaning Up\n");
}

module_init(init_thread);
module_exit(cleanup_thread);
//https://sysplay.in/blog/linux-kernel-internals/2015/04/kernel-threads/

