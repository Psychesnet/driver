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

MODULE_LICENSE("Dual BSD/GPL");

#define DRIVER_NAME "cdevice"
// device count
#define DEVICE_NUMS 4

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
    rwlock_t lock;
};

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
    rwlock_init(&p->lock);
    file->private_data = p;
out:
    return ret;
}

static int cdevice_close(struct inode *inode, struct file *file)
{
    printk("%s: major:%d minor:%d (pid:%d)\n", __func__,
            imajor(inode), iminor(inode), current->pid);
    if (file->private_data) {
        kfree(file->private_data);
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

    if (count >= 1) {
        if (copy_from_user(&val, buf, 1)) {
            write_count = -EFAULT;
            goto out;
        }
        write_lock(&p->lock);
        p->val = val;
        write_unlock(&p->lock);
    }
    write_count = count;
out:
    return write_count;
}

ssize_t cdevice_read(struct file *file, char __user *buf,
        size_t count, loff_t *f_pos)
{
    struct cdevice_data *p = file->private_data;
    unsigned char val;
    ssize_t read_count;
    int i = 0;

    read_lock(&p->lock);
    val = p->val;
    read_unlock(&p->lock);

    printk("%s: count %lu pos %lld\n", __func__, count, *f_pos);

    for (i = 0; i < count; ++i) {
        if (copy_to_user(&buf[i], &val, 1)) {
            read_count = -EFAULT;
            goto out;
        }
    }
    read_count = count;
out:
    return read_count;
}

struct file_operations cdevice_fops = {
    .open = cdevice_open,
    .release = cdevice_close,
    .write = cdevice_write,
    .read = cdevice_read,
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
