#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/completion.h>

MODULE_LICENSE("Dual BSD/GPL");

static struct completion comp;
static struct task_struct *ktask = NULL;

static int just_sleep_thread(void *num)
{
    printk("%s just called\n", __func__);

    for(;;) {
        msleep_interruptible(5*1000);
        if (kthread_should_stop()) {
            break;
        }
    }

    printk("%s leaved\n", __func__);

    complete_and_exit(&comp, 0);

    return 0;
}

static int my_completion_init(void)
{
    printk(KERN_ALERT "driver loaded\n");

    init_completion(&comp);

    ktask = kthread_create(just_sleep_thread, NULL, "just_sleep_thread");
    if (IS_ERR(ktask)) {
        return PTR_ERR(ktask);
    }
    wake_up_process(ktask);
    printk("kernel thread started\n");

    return 0;
}

static void my_completion_exit(void)
{
    printk(KERN_ALERT "driver unloaded\n");

    printk("stop kernl thread and wait....");
    kthread_stop(ktask);

    wait_for_completion(&comp);
    printk("done\n");
}

module_init(my_completion_init);
module_exit(my_completion_exit);
