#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>

MODULE_LICENSE("Dual BSD/GPL");

struct my_data {
    int irq_num;
    struct work_struct workq;
};

const int IO_IRQ_NUM = 42;
struct my_data data;
void *dev_data = (void *)&data;

void workqueue_cb(struct work_struct *work)
{
    struct my_data *p = container_of(work, struct my_data, workq);
    printk("%s: irq:%d (%ld, %ld, %ld)\n",
            __func__, p->irq_num, in_irq(), in_softirq(), in_interrupt());
}

irqreturn_t workqueue_isr(int irq, void *dev_id)
{
    if (printk_ratelimit()) {
        printk("%s: irq:%d (%ld, %ld, %ld)\n",
                __func__, irq, in_irq(), in_softirq(), in_interrupt());
            schedule_work(&data.workq);
    }
    return IRQ_NONE;
}

static int my_workqueue_init(void)
{
    int ret = 0;

    data.irq_num = IO_IRQ_NUM;
    INIT_WORK(&data.workq, workqueue_cb);

    ret = request_irq(data.irq_num, workqueue_isr, IRQF_SHARED, "myworkqueue", dev_data);
    if (ret != 0) {
        printk("fail to request irq: %d\n", data.irq_num);
        goto out;
    }

    printk(KERN_ALERT "driver loaded\n");
out:
    return ret;
}

static void my_workqueue_exit(void)
{
    flush_scheduled_work();
    free_irq(data.irq_num, dev_data);

    printk(KERN_ALERT "driver unloaded\n");
}

module_init(my_workqueue_init);
module_exit(my_workqueue_exit);
