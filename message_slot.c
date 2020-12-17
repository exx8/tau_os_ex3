
#include "message_slot.h"
#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/string.h>
#include "linux/list.h"
#include "linux/slab.h"
#include "linux/uaccess.h"
#include <linux/spinlock.h>
MODULE_LICENSE("GPL");
typedef struct {
    int channel_id;
    unsigned int minor;
} private_data_type;
typedef struct {
    char msg_value[msg_len];
    short len;
    unsigned int channel_id;

} msg;
typedef struct
{
    int len;
    msg * array_of_msg;
} array_list;
static  array_list **minor_arr;
static void debug(char *const fmt) {
    printk(KERN_ERR "%s", fmt); }
    static void debug_pointer(void * pointer)
    {
    printk(KERN_ERR "%p",pointer);
    }
int  add2list(array_list ** local_minor_arr, int minor, msg  new_msg)
{
    void *newPlace;
    local_minor_arr[minor]->len++;
    printk(" new length %d,total size %ld",local_minor_arr[minor]->len,local_minor_arr[minor]->len*sizeof(msg ** ));
    debug_pointer(local_minor_arr[minor]->array_of_msg);
    newPlace= krealloc(local_minor_arr[minor]->array_of_msg, local_minor_arr[minor]->len*sizeof(msg **), GFP_KERNEL);
    printk("resize complete");
    if(newPlace==NULL)
        return 0;

    minor_arr[minor]->array_of_msg= newPlace;
    debug(minor_arr[minor]->array_of_msg[0].msg_value);
    memcpy(&(minor_arr[minor]->array_of_msg[local_minor_arr[minor]->len-1]),&new_msg,sizeof(new_msg));
    //debug(minor_arr[minor]->array_of_msg[list->len-1].msg_value);
    return 1;

}
static unsigned int get_minor(const struct inode *inode) {
    unsigned int minor = iminor(inode);
    return minor;
}

static int device_open(struct inode *inode, struct file *file) {
    unsigned int minor;
    private_data_type  * private_data=kcalloc(sizeof(private_data),1,GFP_KERNEL);
    if(minor_arr==NULL)
    minor_arr = kcalloc(sizeof(*minor_arr), channel_num, GFP_KERNEL);
    minor = get_minor(inode);
    printk("open minor %d \n",minor);
    private_data->minor=minor;
    private_data->channel_id=NO_CHANNEL;
    file->private_data=private_data;

    if (minor_arr[minor] == NULL) {
        minor_arr[minor] = kcalloc(sizeof(array_list), 1, GFP_KERNEL); //WRONG?

        

    }
    return OK;
}

static bool no_channel(const struct file *file) {
    debug("in no channel");

    return ((private_data_type*)file->private_data)->channel_id==NO_CHANNEL; }

static msg *get_entry_by_channel_id(const char *buffer, unsigned int channel_id,unsigned minor) {
    int i=0;
    array_list * current_list;
    current_list=minor_arr[minor];

    debug("before list entry");
    printk(" get entry minor %d",minor);
    printk("len reported by get_entry: %d",current_list->len);

    for(i=0;i<current_list->len;i++)
    {
    msg entry = current_list->array_of_msg[i];
        printk("pointer number: %d",i);
    printk(KERN_ERR "premortum");
    printk("%d",entry.channel_id);
    if (entry.channel_id == channel_id) {
            return &current_list->array_of_msg[i];


        }
    }
    printk(KERN_ERR "postmortum");
    debug_pointer(current_list);
    return NULL;
}

static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset) {
    unsigned int minor;
    short i;
    short returned;
    msg *entry;
    int channel_id;

    debug("before no channel read");
    if (no_channel(file)){
        debug("found no channel");
        return -EINVAL;
        }
    debug("before reading channel_id");
    channel_id = ((private_data_type*)file->private_data)->channel_id;
    minor=((private_data_type*)file->private_data)->minor;
    printk("read minor: %d",minor);
    entry = get_entry_by_channel_id(buffer, channel_id,minor);

    if (entry == NULL) {
        debug("in EWOULDBLOCK");
        return -EWOULDBLOCK; //resource temproray unavailable
    }

    if (entry->len > length)
        return -ENOSPC;
    debug("before for of msg read");
    for (i = 0; i < entry->len; i++)
        put_user(entry->msg_value[i], &buffer[i]);
    returned = entry->len;
    return returned;
}


//invariant: old versions always come after the most updated
static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset) {
    int i;
    msg new_msg;
    char * priv_buffer;
    int minor;
    int channel_id;
    int status;
    minor = ((private_data_type*)file->private_data)->minor;
    printk("shouldn't be 0: %d",((private_data_type*)file->private_data)->minor);
    if (no_channel(file))
        return -EINVAL;
    if (buffer == NULL)
        return -EINVAL;
    debug("device write");
    channel_id = ((private_data_type*)file->private_data)->channel_id;
    debug("device write minor");

    if (length == 0 || length > msg_len)
        return -EMSGSIZE;

    new_msg.channel_id=channel_id;//something is wrong here, can't set right channel_id

    priv_buffer = new_msg.msg_value;
    debug("device write before for");
    printk("%zu",length);
    for (i = 0; i < length; i++)
        get_user(priv_buffer[i], &buffer[i]);
    status=add2list(minor_arr,minor,new_msg);
    printk("viewed length is :%d,status is :%d",minor_arr[minor]->len,status);
    return i;
}

static long einvalid_ioctl(void) {
    return -EINVAL;
}



static long device_ioctl(struct file *file, unsigned int ioctl_command_id, unsigned long channel) {
    unsigned int channel_id = channel;
    debug("iocntrl\n");
    if (ioctl_command_id != MSG_SLOT_CHANNEL || channel == 0) {
        return einvalid_ioctl();
    }

    ((private_data_type*)file->private_data)->channel_id = channel_id; //wasn't init

    return OK;
}



static int device_release(struct inode *inode, struct file *file) {
    return OK;
    //todo match for arrays


}

struct file_operations Fops =
        {
                .owner      = THIS_MODULE, // Required for correct count of module usage. This prevents the module from being removed while used.
                .read           = device_read,
                .write          = device_write,
                .open           = device_open,
                .release        = device_release,
                .unlocked_ioctl=device_ioctl

        };

static int __init simple_init(void) {

    int major;
    int status;
    // Register driver capabilities. Obtain major num
    printk("%d",MAJOR_NUM);
    status=register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &Fops);
    major=MAJOR_NUM;
    // Negative values signify an error
    if (status < 0) {
        printk(KERN_ALERT "%s registraion failed for  %d\n",
               DEVICE_FILE_NAME, major);
        return status;
    }

    printk("Registeration is successful. "
           "The major device number is %d.\n", major);
    printk("If you want to talk to the device driver,\n");
    printk("you have to create a device file:\n");
    printk("mknod /dev/%s c %d 0\n", DEVICE_FILE_NAME, major);
    printk("You can echo/cat to/from the device file.\n");
    printk("Dont forget to rm the device file and "
           "rmmod when you're done\n");

    return 0;
}

static void __exit simple_cleanup(void) {
    // Unregister the device
    // Should always succeed
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

module_init(simple_init);

module_exit(simple_cleanup);