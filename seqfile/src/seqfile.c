#include <linux/module.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <linux/sched/signal.h>

MODULE_LICENSE("Dual BSD/GPL");

#define PROC_FILE "driver/seqfile"

static int seqfile_show(struct seq_file *m, void *v)
{
    struct task_struct *tp = NULL;
    for_each_process(tp) {
        seq_printf(m, "[%s] pid=%d\n", tp->comm, tp->pid);
        seq_printf(m, "     tgid=%d\n", tp->tgid);
        seq_printf(m, "     state=%ld\n", tp->state);
        seq_printf(m, "     mm=0x%p\n", tp->mm);
        seq_printf(m, "     utime=%llu\n", tp->utime);
        seq_printf(m, "     stime=%llu\n", tp->stime);
        seq_puts(m, "\n");
    }

    return 0;
}

int seqfile_open(struct inode *inode, struct file *file)
{
    // if you want to have private data for each process
    // you can kmalloc here and assign to file->private_data
    // and pass file->private_data to single_open as third parameter
    return single_open(file, seqfile_show, inode->i_private);
}

static const struct file_operations seqfile_fops = {
    .owner = THIS_MODULE,
    .open  = seqfile_open,
    .read = seq_read,
    // seq_file only support read
    //.write = seqfile_write,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int seqfile_init(void)
{
    struct proc_dir_entry *file_entry;

    // add proc
    file_entry = proc_create(PROC_FILE, 0666, NULL, &seqfile_fops);
    if (!file_entry) {
        printk("unable to create /proc entry\n");
        return -ENOMEM;
    }

    printk(KERN_ALERT "driver loaded\n");
    return 0;
}

static void seqfile_exit(void)
{
    remove_proc_entry(PROC_FILE, NULL);

    printk(KERN_ALERT "driver unloaded\n");
}

module_init(seqfile_init);
module_exit(seqfile_exit);
