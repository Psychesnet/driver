#include <linux/module.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

MODULE_LICENSE("Dual BSD/GPL");

#define PROC_FILE "driver/procfs"

static int flag = 0;

ssize_t procfs_read(struct file *filp, char *buf, size_t count, loff_t *offp)
{
    ssize_t len = count;

    if (*offp > 0) {
        // return 0 mean we do not have more data to output
        // first time, *offp is 0, next time is len, 2*len, 3*len...
        len = 0;
    } else {
        len = sprintf(buf, "%d\n", flag);
    }

    return len;
}

ssize_t procfs_write(struct file *file, const char *buf, size_t count, loff_t *offp)
{
    char tmp[16] = {0};
    ssize_t len = count;

    if (len >= sizeof(tmp)) {
        len = sizeof(tmp) - 1;
    }

    if (copy_from_user(tmp, buf, len)) {
        printk("fail to copy data from user\n");
        return -EFAULT;
    }
    tmp[len] = '\0';

    int n = simple_strtol(tmp, NULL, 10);
    if (n == 0) {
        flag = 0;
    } else {
        flag = 1;
    }

    return len;
}

static const struct file_operations procfs_fops = {
    .owner = THIS_MODULE,
    .read = procfs_read,
    .write = procfs_write,
};

static int procfs_init(void)
{
    struct proc_dir_entry *file_entry;

    // add proc
    file_entry = proc_create(PROC_FILE, 0666, NULL, &procfs_fops);
    if (!file_entry) {
        printk("unable to create /proc entry\n");
        return -ENOMEM;
    }

    printk(KERN_ALERT "driver loaded\n");
    return 0;
}

static void procfs_exit(void)
{
    remove_proc_entry(PROC_FILE, NULL);

    printk(KERN_ALERT "driver unloaded\n");
}

module_init(procfs_init);
module_exit(procfs_exit);
