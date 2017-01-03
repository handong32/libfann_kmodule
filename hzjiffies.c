#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>

static struct proc_dir_entry *root_irq_dir;
static char Buf[PAGE_SIZE];      /* The buffer to store last message */
static char *Buf_Ptr;           /* A pointer to the buffer */
static int Buf_Char = 0;       /* The number of characters stored */
static int Bytes_Read = 0;      /* Number of bytes read since open */

static int
jiffies_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "%llu\n",
	       (unsigned long long) get_jiffies_64());
    return 0;
}

static int
jiffies_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, jiffies_proc_show, NULL);
}

static int
hz_show(struct seq_file *m, void *v)
{
    seq_printf(m, "%d\n", HZ);
    return 0;
}

static int
hz_open(struct inode *inode, struct file *file)
{
    return single_open(file, hz_show, NULL);
}

static const struct file_operations hz_fops = {
    .owner      = THIS_MODULE,
    .open       = hz_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static const struct file_operations jiffies_proc_fops = {
    .owner      = THIS_MODULE,
    .open       = jiffies_proc_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int __init
nn_proc_init(void)
{
    printk("Created nn module\n");
    /* create /proc/nn */
    root_irq_dir = proc_mkdir("nn", NULL);
    if (!root_irq_dir)
	return -1;

    proc_create("jiffies", 0, root_irq_dir, &jiffies_proc_fops);
    proc_create("hz", 0, root_irq_dir, &hz_fops);

    //printk("Created nn module\n");

    return 0;
}

static void __exit
nn_proc_exit(void)
{
    remove_proc_entry("jiffies", root_irq_dir);
    remove_proc_entry("hz", root_irq_dir);
    remove_proc_entry("nn", NULL);
    printk("Unloading nn module\n");
}

module_init(nn_proc_init);
module_exit(nn_proc_exit);

MODULE_LICENSE("GPL");
