#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include "cdev_ioctl.h"

MODULE_LICENSE("Dual BSD/GPL");

#define DRIVER_NAME "cdevice"
// device count
#define DEVICE_NUMS 4
#define TIMEOUT_SEC 5

// 0 is dynamic allocation
static int cdevice_major = 0;
// accept param when insmod
module_param(cdevice_major, uint, 0);
// global device status
static struct cdev cdevice_cdev;
// class for udev
static struct class *cdevice_class = NULL;

struct cdevice_data {
    unsigned char val;
    // timer
    struct timer_list timer;
    // lock for timer
    spinlock_t lock;
    // event queue
    wait_queue_head_t wait_queue;
    // just a flag to know data is ready
    int timeout_done;
    // critical section for read/write
    struct semaphore sem;
};

static void cdevice_timeout_cb(struct timer_list *t)
{
    struct cdevice_data *p = from_timer(p, t, timer);
    unsigned long flags;

    printk("timer callback\n");

    spin_lock_irqsave(&p->lock, flags);

    p->timeout_done = 1;
    // notify
    wake_up_interruptible(&p->wait_queue);

    spin_unlock_irqrestore(&p->lock, flags);

    // restart timer
    mod_timer(&p->timer, jiffies + TIMEOUT_SEC*HZ);
}

// at user space, no matter select or poll, both are calling .poll system call
unsigned int cdevice_poll(struct file *file, poll_table *wait)
{
    struct cdevice_data *p = file->private_data;
    // it is writable
    unsigned int mask = POLLOUT | POLLWRNORM;

    if (!p) {
        return -EBADFD;
    }

    down(&p->sem);
    // block and wait notify
    poll_wait(file, &p->wait_queue, wait);
    // someone notified
    if (p->timeout_done == 1) {
        // read is ready
        mask |= POLLIN | POLLRDNORM;
    }
    up(&p->sem);
    return mask;
}

static int cdevice_open(struct inode *inode, struct file *file)
{
    int ret = 0;
    printk("%s: major:%d minor:%d (pid:%d)\n", __func__,
            imajor(inode), iminor(inode), current->pid);

    struct cdevice_data *p = kmalloc(sizeof(struct cdevice_data), GFP_KERNEL);
    if (!p) {
        printk("%s: no memory\n", __func__);
        ret = -ENOMEM;
        goto out;
    }

    p->val = 0xff - iminor(inode);
    file->private_data = p;
    spin_lock_init(&p->lock);
    init_waitqueue_head(&p->wait_queue);
    sema_init(&p->sem, 1);

    timer_setup(&p->timer, cdevice_timeout_cb, 0);
    // start timer
    p->timeout_done = 0;
    mod_timer(&p->timer, jiffies + TIMEOUT_SEC*HZ);
out:
    return ret;
}

static int cdevice_close(struct inode *inode, struct file *file)
{
    printk("%s: major:%d minor:%d (pid:%d)\n", __func__,
            imajor(inode), iminor(inode), current->pid);
    struct cdevice_data *p = file->private_data;
    if (p) {
        del_timer_sync(&p->timer);
        kfree(p);
    }
    return 0;
}

ssize_t cdevice_write(struct file *file, const char __user *buf,
        size_t count, loff_t *f_pos)
{
    struct cdevice_data *p = file->private_data;
    unsigned char val;
    ssize_t write_count;

    printk("%s: count %lu pos %lld\n", __func__, count, *f_pos);

    if (down_interruptible(&p->sem)) {
        return -ERESTARTSYS;
    }
    if (count >= 1) {
        if (copy_from_user(&val, buf, 1)) {
            write_count = -EFAULT;
            goto out;
        }
        p->val = val;
    }
    write_count = count;
out:
    up(&p->sem);
    return write_count;
}

ssize_t cdevice_read(struct file *file, char __user *buf,
        size_t count, loff_t *f_pos)
{
    struct cdevice_data *p = file->private_data;
    unsigned char val;
    ssize_t read_count;
    int i = 0;
    int ret = 0;

    if (down_interruptible(&p->sem)) {
        return -ERESTARTSYS;
    }

    // read is not ready
    if (p->timeout_done == 0) {
        up(&p->sem);
        // non-blocking mode
        if (file->f_flags & O_NONBLOCK) {
            return -EAGAIN;
        }
        // blocking mode
        do {
             ret = wait_event_interruptible_timeout(
                     p->wait_queue, p->timeout_done == 1, 1*HZ);
             if (ret == -ERESTARTSYS) {
                 return -ERESTARTSYS;
             }
        } while (ret == 0); // if timeout, do it again
        // read is ready, lock again
        if (down_interruptible(&p->sem)) {
            return -ERESTARTSYS;
        }
    }

    printk("%s: count %lu pos %lld\n", __func__, count, *f_pos);
    val = p->val;
    for (i = 0; i < count; ++i) {
        if (copy_to_user(&buf[i], &val, 1)) {
            read_count = -EFAULT;
            goto out;
        }
    }
    read_count = count;
out:
    p->timeout_done = 0;
    up(&p->sem);
    return read_count;
}

long cdevice_ioctl(struct file *file,
        unsigned int cmd, unsigned long arg)
{
    struct cdevice_data *p = file->private_data;
    int ret = 0;
    struct ioctl_cmd data;

    memset(&data, 0x00, sizeof(data));

    if (down_interruptible(&p->sem)) {
        return -ERESTARTSYS;
    }
    switch (cmd) {
        case IOCTL_SET_VAL:
            if (!capable(CAP_SYS_ADMIN)) {
                printk("permission deny\n");
                ret = -EPERM;
                break;
            }
            if (!access_ok((void __user *)arg, _IOC_SIZE(cmd))) {
                printk("memory is not access\n");
                ret = -EFAULT;
                break;
            }
            if (copy_from_user(&data, (int __user *)arg, sizeof(data))) {
                printk("fail to copy data from user\n");
                ret = -EFAULT;
                break;
            }
            printk("ioctl: set val: 0x%x\n", data.val);
            p->val = data.val;
            break;
        case IOCTL_GET_VAL:
            if (!access_ok((void __user *)arg, _IOC_SIZE(cmd))) {
                printk("memory is not access\n");
                ret = -EFAULT;
                break;
            }
            data.val = p->val;
            if (copy_to_user((int __user *)arg, &data, sizeof(data))) {
                printk("fail to copy data to user\n");
                ret = -EFAULT;
                break;
            }
            break;
        default:
            ret = -ENOTTY;
            break;
    }
    up(&p->sem);
    return ret;
}

struct file_operations cdevice_fops = {
    .open = cdevice_open,
    .release = cdevice_close,
    .write = cdevice_write,
    .read = cdevice_read,
    .unlocked_ioctl = cdevice_ioctl,
    .poll = cdevice_poll,
};

static int cdevice_init(void)
{
    int ret = 0;
    int i = 0;
    // just create dev_t
    dev_t dev = MKDEV(cdevice_major, 0);

    // ask system to alloc 1 cdev into dev variable
    if (alloc_chrdev_region(&dev, 0, DEVICE_NUMS, DRIVER_NAME) != 0) {
        printk("fail to alloc chrdev\n");
        ret = -EFAULT;
        goto out;
    }

    // we get ready major which assigned from kernel
    cdevice_major = MAJOR(dev);
    cdev_init(&cdevice_cdev, &cdevice_fops);
    cdevice_cdev.owner = THIS_MODULE;
    cdevice_cdev.ops = &cdevice_fops;

    // add 4 cdev to cdevice_cdev
    if (cdev_add(&cdevice_cdev, dev, DEVICE_NUMS) != 0) {
        printk("fail to add cdevice\n");
        ret = -EFAULT;
        goto unreg_cdev;
    }

    // register class for udev
    cdevice_class = class_create(THIS_MODULE, DRIVER_NAME);
    if (IS_ERR(cdevice_class)) {
        printk("fail to create class\n");
        goto del_cdev;
    }
    for (i = 0; i < DEVICE_NUMS; i++) {
        // save dev_t from major
        dev_t devone = MKDEV(cdevice_major, i);
        device_create(cdevice_class, NULL, devone,
                NULL, DRIVER_NAME"%d", i);
        printk(KERN_ALERT "%s driver(%d:%d) loaded\n", DRIVER_NAME, MAJOR(devone), MINOR(devone));
    }
    return ret;

del_cdev:
    cdev_del(&cdevice_cdev);
unreg_cdev:
    unregister_chrdev_region(dev, DEVICE_NUMS);
out:
    return ret;
}

static void cdevice_exit(void)
{
    int i = 0;
    dev_t dev;

    for (i = 0; i < DEVICE_NUMS; ++i) {
        dev = MKDEV(cdevice_major, i);
        // unregister class
        device_destroy(cdevice_class, dev);
    }
    class_destroy(cdevice_class);

    dev = MKDEV(cdevice_major, 0);
    cdev_del(&cdevice_cdev);
    unregister_chrdev_region(dev, DEVICE_NUMS);
    printk(KERN_ALERT "%s driver(%d) unloaded\n", DRIVER_NAME, MAJOR(dev));
}

module_init(cdevice_init);
module_exit(cdevice_exit);
