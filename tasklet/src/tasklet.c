#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>

MODULE_LICENSE("Dual BSD/GPL");

const int IO_IRQ_NUM = 42;
void *dev_data = (void *)&IO_IRQ_NUM;
struct tasklet_struct tasklet;

void tasklet_cb(unsigned long data)
{
    int *p = (int *)data;
    printk("%s: irq:%d (%ld, %ld, %ld)\n",
            __func__, *p, in_irq(), in_softirq(), in_interrupt());
}

irqreturn_t tasklet_isr(int irq, void *dev_id)
{
    if (printk_ratelimit()) {
        printk("%s: irq:%d (%ld, %ld, %ld)\n",
                __func__, irq, in_irq(), in_softirq(), in_interrupt());
            tasklet_schedule(&tasklet);
    }
    return IRQ_NONE;
}

static int my_tasklet_init(void)
{
    int ret = 0;

    tasklet_init(&tasklet, tasklet_cb, dev_data);

    ret = request_irq(IO_IRQ_NUM, tasklet_isr, IRQF_SHARED, "mytasklet", dev_data);
    if (ret != 0) {
        printk("fail to request irq: %d\n", IO_IRQ_NUM);
        tasklet_kill(&tasklet);
        goto out;
    }

    printk(KERN_ALERT "driver loaded\n");
out:
    return ret;
}

static void my_tasklet_exit(void)
{
    tasklet_kill(&tasklet);
    free_irq(IO_IRQ_NUM, dev_data);

    printk(KERN_ALERT "driver unloaded\n");
}

module_init(my_tasklet_init);
module_exit(my_tasklet_exit);
