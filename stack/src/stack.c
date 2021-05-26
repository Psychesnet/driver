#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>

MODULE_LICENSE("Dual BSD/GPL");

struct stack_data {
    spinlock_t lock;
    struct file *file;
    struct list_head list;
    wait_queue_head_t wait;
    int no;
};

struct stack_data head;

void free_stack(void)
{
    struct list_head *listptr;
    struct stack_data *entry;
    /* foreach link list */
    list_for_each(listptr, &head.list) {
        /* use list to look for entire struct */
        entry = list_entry(listptr, struct stack_data, list);
        printk("Free: no=%d(list %p, next %p, prev %p)\n",
                entry->no, &entry->list, entry->list.next, entry->list.prev);
        kfree(entry);
    }
}

void show_stack_data(void)
{
    struct list_head *listptr;
    struct stack_data *entry;
    printk("--------------------------------------\n");
    printk("Stack: no=%d(list %p, next %p, prev %p)\n",
            head.no, &head.list, head.list.next, head.list.prev);

    /* foreach link list */
    list_for_each(listptr, &head.list) {
        /* use list to look for entire struct */
        entry = list_entry(listptr, struct stack_data, list);
        printk("Stack: no=%d(list %p, next %p, prev %p)\n",
                entry->no, &entry->list, entry->list.next, entry->list.prev);
    }
}

void remove_stack(struct stack_data *entry)
{
    printk("Remove: no=%d(list %p, next %p, prev %p)\n",
            entry->no, &entry->list, entry->list.next, entry->list.prev);
    list_del(&entry->list);
    kfree(entry);
}

struct stack_data *add_stack(int no)
{
    struct stack_data *ptr = NULL;
    do {
        ptr = kmalloc(sizeof(struct stack_data), GFP_KERNEL);
        if (!ptr) {
            printk("fail to alloc memory for new data\n");
            break;
        }
        ptr->no = no;
        list_add(&ptr->list, &head.list);
    } while (false);
    return ptr;
}

static int stack_init(void)
{
    //clean stack
    memset(&head, 0x00, sizeof(head));
    INIT_LIST_HEAD(&head.list);
    show_stack_data();

    add_stack(1);
    show_stack_data();

    struct stack_data *p = add_stack(2);
    add_stack(3);
    show_stack_data();

    if (p) {
        remove_stack(p);
        show_stack_data();
    }

    printk(KERN_ALERT "driver loaded\n");

    return 0;
}

static void stack_exit(void)
{
    free_stack();
    printk(KERN_ALERT "driver unloaded\n");
}

module_init(stack_init);
module_exit(stack_exit);
