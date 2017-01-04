#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/page.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/string.h>

#include "math.h"

#define NNNS 50

static struct proc_dir_entry *root_irq_dir;

static char Buf[PAGE_SIZE];      /* The buffer to store last message */
static char *Buf_Ptr;           /* A pointer to the buffer */
static int Buf_Char = 0;       /* The number of characters stored */
static int Bytes_Read = 0;      /* Number of bytes read since open */

struct nn
{
    float desired_error;
    unsigned int valid;
    unsigned int num_layers;
    unsigned int num_input;
    unsigned int num_neurons_hidden;
    unsigned int num_output;
    unsigned int max_epochs;
    unsigned int epochs_between_reports;
    char name[50];
};

static struct nn nns[NNNS];

int getNN(void)
{
    int i;
    
    for(i=0;i<NNNS;i++)
    {
	if(!nns[i].valid)
	{
	    return i;
	}
    }
    return -1;
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

static int
nn_open(struct inode *inode, struct file *file)
{
    Buf_Ptr = Buf;
    Bytes_Read = 0;
    return 0;
}

static ssize_t
nn_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int already_read = Bytes_Read;

    if (Bytes_Read >= Buf_Char)
	return 0;

    while (len && (Bytes_Read < Buf_Char)) 
    {
	put_user(Buf_Ptr[Bytes_Read], buffer+Bytes_Read);
	
	len--;
	Bytes_Read++;
    }
    
    return Bytes_Read-already_read;
}

static const struct file_operations hz_fops = {
    .owner      = THIS_MODULE,
    .open       = hz_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static ssize_t
nn_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    int i, s, e, n;
    char tmp[100];
    char tmp2[50];
    char tmp3[50];
    char tmp4[20];
    
    Buf_Char = 0;

    if (Buf_Char >= PAGE_SIZE) {
	return 0;
    }
    
    while (len && (Buf_Char < PAGE_SIZE)) 
    {
	get_user(Buf_Ptr[Buf_Char], buffer+Buf_Char);
	
	len--;
	Buf_Char++;
    }

    s = e = 0;
    n = getNN();
    nns[n].valid = 1;
    
    printk("n -> %d\n", n);
    for(i=0;i<Buf_Char;i++)
    {
	if(Buf_Ptr[i] == '\n')
	{
	    Buf_Ptr[i] = '\0';
	    
	    memcpy(tmp, Buf_Ptr+s, 100);
	    memset(tmp2, 0, 50);
	    memset(tmp3, 0, 50);
	    sscanf(tmp, "%s %s", tmp2, tmp3);
	    
	    if(strcmp(tmp2, "name") == 0)
	    {
		memcpy(nns[n].name, tmp3, 50);
		printk("%s -> %s\n", tmp2, nns[n].name);
	    }
	    else if(strcmp(tmp2, "desired_error") == 0)
	    {
                stof(tmp3, &(nns[n].desired_error));
		mftoa(nns[n].desired_error, &(tmp4[0]), 4);
		printk("desired_error -> %s\n", tmp4);
	    }
	    else if(strcmp(tmp2, "num_layers") == 0)
	    {
		nns[n].num_layers = matoi(tmp3);
		printk("num_layers -> %d\n", nns[n].num_layers);
	    } 
	    else if(strcmp(tmp2, "num_input") == 0)
	    {
		nns[n].num_input = matoi(tmp3);
		printk("num_input -> %d\n", nns[n].num_input);
	    }
	    else if(strcmp(tmp2, "num_neurons_hidden") == 0)
	    {
	        nns[n].num_neurons_hidden = matoi(tmp3);
		printk("num_neurons_hidden -> %d\n", nns[n].num_neurons_hidden);
	    }
	    else if(strcmp(tmp2, "num_output") == 0)
	    {
		nns[n].num_output = matoi(tmp3);
		printk("num_output -> %d\n", nns[n].num_output);
	    }
	    else if(strcmp(tmp2, "max_epochs") == 0)
	    {
	        nns[n].max_epochs = matoi(tmp3);
		printk("max_epochs -> %d\n", nns[n].max_epochs);
	    }
	    else if(strcmp(tmp2, "epochs_between_reports") == 0)
	    {
	        nns[n].epochs_between_reports = matoi(tmp3);
		printk("epochs_between_reports -> %d\n", nns[n].epochs_between_reports);
	    }
	    else
	    {
		printk("Unknown input %s, error\n", tmp2);
		return -1;
	    }
	    
	    s = i+1;
	}
    }

    return Buf_Char;
}

static int
nn_release(struct inode *inodep, struct file *filep){
    //printk(KERN_INFO "%s Device successfully closed\n", __PRETTY_FUNCTION__);
    return 0;
}

static const struct file_operations nn_fops = {
    .owner      = THIS_MODULE,
    .open       = nn_open,
    .read       = nn_read,
    .write      = nn_write,
    .release = nn_release,
};

static int __init
nn_proc_init(void)
{
    root_irq_dir = proc_mkdir("nn", NULL);
    if (!root_irq_dir)
	return -1;
    
    proc_create("init", 0, root_irq_dir, &nn_fops);
    proc_create("hz", 0, root_irq_dir, &hz_fops);

    printk("Created nn module\n");
    return 0;
}

static void __exit
nn_proc_exit(void)
{
    remove_proc_entry("init", root_irq_dir);
    remove_proc_entry("hz", root_irq_dir);
    remove_proc_entry("nn", NULL);
    printk("Unloading nn module\n");
}


module_init(nn_proc_init);
module_exit(nn_proc_exit);

MODULE_LICENSE("GPL");
