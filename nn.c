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
#include "src/include/fann.h"

#define NNNS 50
#define NAMELEN 50
#define BSIZE 100

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
    struct fann *ann;
    struct fann_train_data *data;
    char *Buf_Ptr;
    int Buf_Char;
    int Bytes_Read;
    char Buf[PAGE_SIZE];
    char name[NAMELEN];
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

void freeNN(void)
{
    int i;
    
    for(i=0;i<NNNS;i++)
    {
	if(nns[i].valid)
	{
	    nns[i].valid = 0;
	    if(nns[i].ann != NULL)
	    {
		fann_destroy(nns[i].ann);
		nns[i].ann = NULL;
	    }
	    printk("Unloading proc -> %s\n", nns[i].name);
	    remove_proc_entry(nns[i].name, root_irq_dir);
	    memset(nns[i].name, 0, NAMELEN);
	    memset(nns[i].Buf, 0, PAGE_SIZE);
	    nns[i].Buf_Ptr = NULL;
	}
    }
}
static int
nns_proc_release(struct inode *inodep, struct file *filep){
    return 0;
}

static ssize_t
nns_proc_read(struct file *file, char *buffer, size_t len, loff_t *offset)
{
    int already_read;
    unsigned int n;
    
    n = (unsigned int)(long)PDE_DATA(file_inode(file)); //wtf?

    printk("%s %d\n", __PRETTY_FUNCTION__, n);
    
    already_read = nns[n].Bytes_Read;

    if (nns[n].Bytes_Read >= nns[n].Buf_Char)
	return 0;

    while (len && (nns[n].Bytes_Read < nns[n].Buf_Char)) 
    {
	put_user(nns[n].Buf_Ptr[nns[n].Bytes_Read], buffer+nns[n].Bytes_Read);
	
	len--;
	nns[n].Bytes_Read++;
    }
    //printk("%s %s\n", __PRETTY_FUNCTION__, nns[n].Buf);
    
    return nns[n].Bytes_Read-already_read;
}

static int
nns_proc_open(struct inode *inode, struct file *file)
{
    unsigned int n;
    n = (unsigned int)(long)PDE_DATA(inode);

    printk("%s %d\n", __PRETTY_FUNCTION__, n);

    nns[n].Buf_Ptr = nns[n].Buf;
    nns[n].Bytes_Read = 0;
    
    return 0;
}

static ssize_t 
nns_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    unsigned int n;
    n = (unsigned int)(long)PDE_DATA(file_inode(file)); //wtf?
    printk("%s %d\n", __PRETTY_FUNCTION__, n);
    
    nns[n].Buf_Char = 0;

    if (nns[n].Buf_Char >= PAGE_SIZE) {
	return 0;
    }
    
    while (count && (nns[n].Buf_Char < PAGE_SIZE)) 
    {
	get_user(nns[n].Buf_Ptr[nns[n].Buf_Char], buffer+nns[n].Buf_Char);
	
        count--;
	nns[n].Buf_Char++;
    }
    
    return nns[n].Buf_Char;
}

static const struct file_operations nns_proc_fops = {
    .owner          = THIS_MODULE,
    .open           = nns_proc_open,
    .read           = nns_proc_read,
    .write          = nns_proc_write,
    .release        = nns_proc_release,
};

static int
init_open(struct inode *inode, struct file *file)
{
    Buf_Ptr = Buf;
    Bytes_Read = 0;
    return 0;
}

static ssize_t
init_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
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

static ssize_t
init_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    int i, s, e, n;
    char tmp[BSIZE];
    char tmp2[BSIZE];
    char tmp3[BSIZE];
    char tmp4[BSIZE];
    
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
	    
	    memcpy(tmp, Buf_Ptr+s, BSIZE);
	    memset(tmp2, 0, BSIZE);
	    memset(tmp3, 0, BSIZE);
	    sscanf(tmp, "%s %s", tmp2, tmp3);
	    
	    if(strcmp(tmp2, "name") == 0)
	    {
		memcpy(nns[n].name, tmp3, BSIZE);
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

    nns[n].ann = fann_create_standard(nns[n].num_layers, nns[n].num_input, nns[n].num_neurons_hidden, nns[n].num_output);
    if(nns[n].ann == NULL)
    {
	printk("ERR cannot allocate ann\n");
	return -1;
    }

    proc_create_data(nns[n].name, 0, root_irq_dir, &nns_proc_fops, (void*)(long)n);
    
    return Buf_Char;
}

static int
init_release(struct inode *inodep, struct file *filep){
    return 0;
}

static const struct file_operations init_fops = {
    .owner      = THIS_MODULE,
    .open       = init_open,
    .read       = init_read,
    .write      = init_write,
    .release    = init_release,
};

static int __init
nn_proc_init(void)
{
    root_irq_dir = proc_mkdir("nn", NULL);
    if (!root_irq_dir)
	return -1;
    
    proc_create("init", 0, root_irq_dir, &init_fops);
    
    printk("Created nn module\n");
    return 0;
}

static void __exit
nn_proc_exit(void)
{
    freeNN();
    remove_proc_entry("init", root_irq_dir);
    remove_proc_entry("nn", NULL);
    printk("Unloading nn module\n");
}


module_init(nn_proc_init);
module_exit(nn_proc_exit);

MODULE_LICENSE("GPL");
