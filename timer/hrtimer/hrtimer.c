#include <linux/init.h> // macros used to markup functions e.g. __init, __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/kernel.h> // Contains types, macros, functions for the kernel
			  //
#include <linux/hrtimer.h>
#include <linux/ktime.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amitesh Singh");
MODULE_DESCRIPTION("A simple hrtimer example");
MODULE_VERSION("0.1");

//jiffies is the current time.

static struct hrtimer hr_timer;
static u64 sampling_period_ms = 100; //100ms
static enum hrtimer_restart callback(struct hrtimer *timer)
{
	pr_info("timer++");
	hrtimer_forward_now(&hr_timer, ms_to_ktime(sampling_period_ms));
	//or HRTIMER_NORESTART;//timer is no restarted
	return HRTIMER_RESTART; //timer is restarted
}

static int __init
_timer_init(void)
{
	pr_info("hr timer module init");
	/*
	 *
	 *
	 *  HRTIMER_MODE_ABS		- Time value is absolute
 * HRTIMER_MODE_REL		- Time value is relative to now
 * HRTIMER_MODE_PINNED		- Timer is bound to CPU (is only considered
 *				  when starting the timer)
 * HRTIMER_MODE_SOFT		- Timer callback function will be executed in
 *				  soft irq context
 * HRTIMER_MODE_HARD		- Timer callback function will be executed in
 *				  hard irq context even on PREEMPT_RT.
 *				  */

	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hr_timer.function = &callback;
	// or ms_to_ktime = ktime_set(0, nanoseconds);
	hrtimer_start(&hr_timer, ms_to_ktime(sampling_period_ms), HRTIMER_MODE_REL);

	return 0;
}

static void
_timer_exit(void)
{
	pr_info("hr timer module exit");

	hrtimer_cancel(&hr_timer);
}

module_init(_timer_init);
module_exit(_timer_exit);

