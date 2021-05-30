#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("Dual BSD/GPL");

atomic_t count = ATOMIC_INIT(0);

static int my_atomic_init(void)
{
    atomic_inc(&count);
    int n = atomic_read(&count);

    if (atomic_dec_and_test(&count)) {
        printk("count is 0\n");
    } else {
        printk("count is not 0\n");
    }

    printk(KERN_ALERT "driver loaded\n");
    return 0;
}

static void my_atomic_exit(void)
{
    printk(KERN_ALERT "driver unloaded\n");
}

module_init(my_atomic_init);
module_exit(my_atomic_exit);
