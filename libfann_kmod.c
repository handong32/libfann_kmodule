#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/time.h>

#include <asm/page.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/string.h>
#include <asm/i387.h>

#include "math.h"
#include "src/include/fann.h"
#include "twister.h"

#define  DEVICE_NAME "NN"
#define  CLASS_NAME  "mnt"

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static int    numberOpens = 0;              ///< Counts the number of times the device is opened
static struct class*  nnClass  = NULL; ///< The device-driver class struct pointer
static struct device* nnDevice = NULL; ///< The device-driver device struct pointer

// IOCTL calls
#define MAJOR_NUM 101
#define IOCTL_TEST _IO(MAJOR_NUM, 0)
/*#define IOCTL_SET_FILESIZE _IOR(MAJOR_NUM, 0, xlen_t *)
#define IOCTL_SET_NN _IOR(MAJOR_NUM, 1, xlen_t *)
#define IOCTL_SHOW_ANT _IO(MAJOR_NUM, 2)
#define IOCTL_PHYS_ADDR _IO(MAJOR_NUM, 3)
#define IOCTL_TRANS_PHYS_ADDR _IOR(MAJOR_NUM, 4, xlen_t*)*/
//#define IOCTL_TEST _IO(MAJOR_NUM, 5)

static char Buf[PAGE_SIZE];      /* The buffer to store last message */
static char *Buf_Ptr;           /* A pointer to the buffer */
static int Buf_Char = 0;       /* The number of characters stored */
static int Bytes_Read = 0;      /* Number of bytes read since open */

static struct fann *ann = NULL;
static struct fann_train_data *data = NULL;

static unsigned int lfnum_layers = 0;
static unsigned int lfnum_input = 0;
static unsigned int lfnum_neurons_hidden = 0;
static unsigned int lfnum_output = 0;
static unsigned int lfnum_data = 0;

module_param(lfnum_layers, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(lfnum_input, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(lfnum_neurons_hidden, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(lfnum_output, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(lfnum_data, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

void printData(void)
{
    int i, j;
    
    for(i=0; i < lfnum_data; i++)
    {
	printk("i=%d\nInput:\n", i);
	
	for(j=0; j<lfnum_input; j++)
	{
	    printk("\t %d ", (int)(data->input[i][j])); 
	}
	printk("\nOutput:\n");
	
	for(j=0; j<lfnum_output; j++)
	{
	    printk("\t %d ", (int)(data->output[i][j])); 
	}
	printk("\n\n");
    }
}

long nn_ioctl(struct file* file,
		  unsigned int ioctl_num,
		  unsigned long ioctl_param)
{
    double test;
    char tbuf[100];
/*    struct timeval time;
    unsigned long local_time;*/
/*switch(ioctl_num) 
  {
    case IOCTL_TEST:
	printk(KERN_INFO "IOCTL_TEST\n");
	break;
    default:
	printk(KERN_INFO "UNKNOWN IOCTL\n");
	break;
	}*/

    /* fann_type *calc_out; */
    /* const unsigned int num_input = 2; */
    /* const unsigned int num_output = 1; */
    /* const unsigned int num_layers = 3; */
    /* const unsigned int num_neurons_hidden = 3; */
    /* const float desired_error = (const float) 0; */
    /* const unsigned int max_epochs = 10; */
    /* const unsigned int epochs_between_reports = 1; */
    /* struct fann *ann; */
    /* struct fann_train_data *data; */
    
    /* unsigned int i = 0; */
    /* unsigned int decimal_point; */

    /* printk(KERN_INFO "Creating network.\n"); */
    /* ann = fann_create_standard(num_layers, num_input, num_neurons_hidden, num_output); */
    /* fann_destroy(ann); */

    
    /* do_gettimeofday(&time); */
    /* local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60)); */

    /* printk(KERN_INFO "time = %lu\n", local_time); */
    
    /* seedMT(4357U); */
    /* printk(KERN_INFO "rand = %lu\n", (unsigned long) randomMT()); */

    mexp(5.0, &test);
    mftoa(test, &(tbuf[0]), 5);
    printk("mexp = %s\n", tbuf);

    mlog(5.5, &test);
    mftoa(test, &(tbuf[0]), 5);
    printk("mlog = %s\n", tbuf);

    mpow(7.0, 3.0, &test);
    mftoa(test, &(tbuf[0]), 5);
    printk("7 ^ 3 = %s\n", tbuf);

    mpow(4.73, 12, &test);
    mftoa(test, &(tbuf[0]), 5);
    printk("4.73 ^ 12 = %s\n", tbuf);

    mpow(32.01, 1.54, &test);
    mftoa(test, &(tbuf[0]), 5);
    printk("32.91 ^ 1.54 = %s\n", tbuf);

    msin(45.0 * (3.14159265/180.0), &test);
    mftoa(test, &(tbuf[0]), 5);
    printk("sine = %s\n", tbuf);
    
    mcos(60.0 * (3.14159265/180.0), &test);
    mftoa(test, &(tbuf[0]), 5);
    printk("cosine = %s\n", tbuf);
	
    return 0;

}

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
   .unlocked_ioctl = nn_ioctl,
};

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init nndev_init(void){
    printk(KERN_INFO "%s Initializing the NN LKM\n", __PRETTY_FUNCTION__);
 
   // Try to dynamically allocate a major number for the device -- more difficult but worth it
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
       printk(KERN_ALERT "%s failed to register a major number\n", __PRETTY_FUNCTION__);
      return majorNumber;
   }
   printk(KERN_INFO "%s registered correctly with major number %d\n", __PRETTY_FUNCTION__, majorNumber);
 
   // Register the device class
   nnClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(nnClass)){                // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "%s Failed to register device class\n", __PRETTY_FUNCTION__);
      return PTR_ERR(nnClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "%s device class registered correctly\n", __PRETTY_FUNCTION__);
 
   // Register the device driver
   nnDevice = device_create(nnClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(nnDevice)){               // Clean up if there is an error
      class_destroy(nnClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "%s Failed to create the device\n", __PRETTY_FUNCTION__);
      return PTR_ERR(nnDevice);
   }

   printk("Num Layers %d\n", lfnum_layers);
   printk("Num Input %d\n", lfnum_input);
   printk("Num Neurons Hidden %d\n", lfnum_neurons_hidden);
   printk("Num Output %d\n", lfnum_output);
   printk("Num Data %d\n", lfnum_data);
   
   if(ann != NULL || data != NULL)
   {
       fann_destroy_train(data);
       fann_destroy(ann);
       ann = NULL;
       data = NULL;
   }

   printk("Creating network.\n");
   ann = fann_create_standard(lfnum_layers, lfnum_input, lfnum_neurons_hidden, lfnum_output);

   data = fann_create_train(lfnum_data, lfnum_input, lfnum_output);
   if(data == NULL || ann == NULL)
   {
       printk("ERR cannot allocate ann or data\n");
   }
   
   //printData();
   printk(KERN_INFO "%s device class created correctly\n", __PRETTY_FUNCTION__);
   return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit nndev_exit(void){
   device_destroy(nnClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(nnClass);                          // unregister the device class
   class_destroy(nnClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number

   fann_destroy_train(data);
   fann_destroy(ann);
   data = NULL;
   ann = NULL;
   printk(KERN_INFO "%s Goodbye from the LKM!\n", __PRETTY_FUNCTION__);
}

static int dev_open(struct inode *inodep, struct file *filep){
   numberOpens++;
   Buf_Ptr = Buf;
   Bytes_Read = 0;
   
   printk(KERN_INFO "%s Device has been opened %d time(s)\n", __PRETTY_FUNCTION__, numberOpens);
   return 0;
}

/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
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

/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the sprintf() function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
    int i, s, e;
    char tmp[20];
    char *tok, *end;
    long ret;
    int nd, ni, no;
    int cio;

    const float desired_error = (const float) 0;
    const unsigned int max_epochs = 100;
    const unsigned int epochs_between_reports = 1;

    nd = ni = no = 0;
    cio = 1;
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
    
    printk(KERN_INFO "%d\n", Buf_Char);

    s = e = 0;
    for(i=0;i<Buf_Char;i++)
    {
	if(Buf_Ptr[i] == '\n')
	{
	    Buf_Ptr[i] = '\0';
	    
	    memcpy(tmp, Buf_Ptr+s, 20);
	    tok = tmp;
	    end = tmp;

	    if(cio) //input
	    {
		cio = 0;
		//printk("input: ");
		while(tok != NULL)
		{
		    strsep(&end, " ");
		    ret = simple_strtol(tok, NULL, 10);
		    //printk("%d %d: %d ", nd, ni,(int)ret);
		    data->input[nd][ni] = ret;
		    ni ++;
		    tok = end;
		}
		ni = 0;
		//printk("\n");
	    }
	    else //output
	    {
		cio = 1;

		//printk("output: ");
		while(tok != NULL)
		{
		    strsep(&end, " ");
		    ret = simple_strtol(tok, NULL, 10);
		    //printk("%d %d: %d ", nd, no,(int)ret);
		    data->output[nd][no] = ret;
		    no ++;
		    tok = end;
		}
		no = 0;
		nd ++;
		//printk("\n");
	    }
	    //printk("%s\n", tmp);
	    s = i+1;
	    //printk("%s\n", Buf_Ptr+s);
	    //printk(KERN_INFO "START %d %s END \n\n", i, tmp);
	    //s = i;
	}
    }

    printData();

    fann_set_activation_steepness_hidden(ann, 1);
    fann_set_activation_steepness_output(ann, 1);
    
    fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
    fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);
    
    fann_set_train_stop_function(ann, FANN_STOPFUNC_BIT);
    fann_set_bit_fail_limit(ann, 0.01f);
     
    fann_set_training_algorithm(ann, FANN_TRAIN_RPROP);
    fann_init_weights(ann, data);

    printk("Training network.\n");
    fann_train_on_data(ann, data, max_epochs, epochs_between_reports, desired_error);
    
    return Buf_Char;
}

static int dev_release(struct inode *inodep, struct file *filep){
    printk(KERN_INFO "%s Device successfully closed\n", __PRETTY_FUNCTION__);
    return 0;
}

module_init(nndev_init);
module_exit(nndev_exit);
