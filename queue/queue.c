#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("Dual BSD/GPL");

struct queue_data {
    spinlock_t lock;
    struct file *file;
    struct list_head list;
    wait_queue_head_t wait;
    int no;
};

struct queue_data head;

void free_queue(void)
{
    struct list_head *listptr;
    struct queue_data *entry;
    /* foreach link list */
    list_for_each(listptr, &head.list) {
        /* use list to look for entire struct */
        entry = list_entry(listptr, struct queue_data, list);
        printk("Free: no=%d(list %p, next %p, prev %p)\n",
                entry->no, &entry->list, entry->list.next, entry->list.prev);
        kfree(entry);
    }
}

void show_queue(void)
{
    struct list_head *listptr;
    struct queue_data *entry;
    printk("--------------------------------------\n");
    printk("queue: no=%d(list %p, next %p, prev %p)\n",
            head.no, &head.list, head.list.next, head.list.prev);

    /* foreach link list */
    list_for_each(listptr, &head.list) {
        /* use list to look for entire struct */
        entry = list_entry(listptr, struct queue_data, list);
        printk("queue: no=%d(list %p, next %p, prev %p)\n",
                entry->no, &entry->list, entry->list.next, entry->list.prev);
    }
}

void remove_queue(struct queue_data *entry)
{
    printk("Remove: no=%d(list %p, next %p, prev %p)\n",
            entry->no, &entry->list, entry->list.next, entry->list.prev);
    list_del(&entry->list);
    kfree(entry);
}

struct queue_data *add_queue(int no)
{
    struct queue_data *ptr = NULL;
    do {
        ptr = kmalloc(sizeof(struct queue_data), GFP_KERNEL);
        if (!ptr) {
            printk("fail to alloc memory for new data\n");
            break;
        }
        ptr->no = no;
        list_add_tail(&ptr->list, &head.list);
    } while (false);
    return ptr;
}

static int queue_init(void)
{
    //clean queue
    memset(&head, 0x00, sizeof(head));
    INIT_LIST(&head.list);
    show_queue();

    add_queue(1);
    show_queue();

    struct show_queue *p = add_queue(2);
    add_queue(3);
    show_queue();

    if (p) {
        remove_queue(p);
        show_queue();
    }

    printk(KERNEL_ALERT "driver loaded\n");

    return 0;
}

static void queue_exit(void)
{
    free_queue();
    printk(KERNEL_ALERT "driver unloaded\n");
}

module_init(queue_init);
module_exit(queue_exit);
