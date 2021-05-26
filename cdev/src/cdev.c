#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <asm/current.h>
#include <asm/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");

#define DRIVER_NAME "cdevice"

// 0 is dynamic allocation
static int cdevice_major = 0;
// device count
static int cdevice_count = 2;
// accept param when insmod
module_param(cdevice_major, uint, 0);
static struct cdev cdevice_cdev;

static int cdevice_open(struct inode *inode, struct file *file)
{
    printk("%s: major:%d minor:%d (pid:%d)\n", __func__,
            imajor(inode), iminor(inode), current->pid);
    // inode屬於device，所以1個device，只有1份inode存在於kernel
    // inode主要存major:minor
    inode->i_private = inode;
    // file是分個process都有一份，記錄存取device的狀態
    file->private_data = file;
    return 0;
}

static int cdevice_close(struct inode *inode, struct file *file)
{
    printk("%s: major:%d minor:%d (pid:%d)\n", __func__,
            imajor(inode), iminor(inode), current->pid);
    return 0;
}

struct file_operations cdevice_fops = {
    .open = cdevice_open,
    .release = cdevice_close,
};

static int cdevice_init(void)
{
    int ret = -1;
    dev_t dev;
    do {
        dev = MKDEV(cdevice_major, 0);
        if (alloc_chrdev_region(&dev, 0, cdevice_count, DRIVER_NAME) != 0) {
            printk("fail to alloc chrdev\n");
            break;
        }
        cdevice_major = MAJOR(dev);

        cdev_init(&cdevice_cdev, &cdevice_fops);
        cdevice_cdev.owner = THIS_MODULE;

        if (cdev_add(&cdevice_cdev, MKDEV(cdevice_major, 0), cdevice_count) != 0) {
            printk("fail to add cdevice\n");
            break;
        }
        ret = 0;
        printk(KERN_ALERT "%s driver(%d:%d) loaded\n", DRIVER_NAME, MAJOR(dev), MINOR(dev));
    } while (false);
    if (ret != 0) {
        cdev_del(&cdevice_cdev);
        unregister_chrdev_region(dev, cdevice_count);
    }
    return ret;
}

static void cdevice_exit(void)
{
    dev_t dev = MKDEV(cdevice_major, 0);

    cdev_del(&cdevice_cdev);
    unregister_chrdev_region(dev, cdevice_count);
    printk(KERN_ALERT "%s driver(%d:%d) unloaded\n", DRIVER_NAME, MAJOR(dev), MINOR(dev));
}

module_init(cdevice_init);
module_exit(cdevice_exit);
