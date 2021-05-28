#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>

MODULE_LICENSE("Dual BSD/GPL");

// we want to share irq num of uart
#define UART_IRQ_NUM 39

static irqreturn_t interrupt_cb(int irq, void *dev_id)
{
    printk("irq: %d, dev_id: %p\n", irq, dev_id);
    return IRQ_NONE;
}

static int myirq_init(void)
{
    if (request_irq(UART_IRQ_NUM, interrupt_cb, IRQF_SHARED,
                "myirq", (void *)interrupt_cb)) {
        printk("fail to request irq %d\n", UART_IRQ_NUM);
    }

    printk(KERN_ALERT "driver loaded\n");
    return 0;
}

static void myirq_exit(void)
{
    free_irq(UART_IRQ_NUM, (void *)interrupt_cb);
    printk(KERN_ALERT "driver unloaded\n");
}

module_init(myirq_init);
module_exit(myirq_exit);
