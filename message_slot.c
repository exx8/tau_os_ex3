
#include "message_slot.h"
#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/string.h>
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

static int size_of_lists[channel_num];
static msg **minor_arr;

static void debug(char *const fmt) {
    printk(KERN_ERR "%s", fmt);
}

static void debug_pointer(void *pointer) {
    printk(KERN_ERR "%p", pointer);
}

/**
 *
 * @param minor
 * @param new_msg
 * @return 0 for failure 1 for success
 */
int add2list(int minor, msg new_msg) {
    int k;
    void *newPlace;
    size_of_lists[minor]++;
    newPlace = krealloc(minor_arr, size_of_lists[minor] * sizeof(msg **), GFP_KERNEL);
    printk("resize complete");
    if (newPlace == NULL)
        return 0;

    minor_arr[minor] = newPlace;
    for (k = 0; k < new_msg.len; k++) {
        minor_arr[minor][size_of_lists[minor] - 1].msg_value[k] = new_msg.msg_value[k];
    }
    printk("new_msg is %s, and arr is %s length of msg is %d", new_msg.msg_value,
           minor_arr[minor][size_of_lists[minor] - 1].msg_value, new_msg.len);
    minor_arr[minor][size_of_lists[minor] - 1].channel_id = new_msg.channel_id;

    minor_arr[minor][size_of_lists[minor] - 1].len = new_msg.len;
    return 1;

}

static unsigned int get_minor(const struct inode *inode) {
    unsigned int minor = iminor(inode);
    return minor;
}

static int device_open(struct inode *inode, struct file *file) {
    unsigned int minor;
    private_data_type *private_data = kcalloc(sizeof(private_data), 1, GFP_KERNEL);
    if (minor_arr == NULL)
        minor_arr = kcalloc(sizeof(*minor_arr), channel_num, GFP_KERNEL);
    minor = get_minor(inode);
    printk("open minor %d \n", minor);
    private_data->minor = minor;
    private_data->channel_id = NO_CHANNEL;
    file->private_data = private_data;


    return OK;
}

static bool no_channel(const struct file *file) {
    debug("in no channel");

    return ((private_data_type *) file->private_data)->channel_id == NO_CHANNEL;
}

static msg *get_entry_by_channel_id(const char *buffer, unsigned int channel_id, unsigned minor) {
    int i = 0;
    printk("size as viewed by get_entry_by_channel_id :%d", size_of_lists[minor]);
    debug_pointer(minor_arr[minor]);
    for (i = size_of_lists[minor] - 1; i >= 0; i--) {
        msg entry = (minor_arr[minor])[i];
        printk("pointer number: %d", i);
        printk(KERN_ERR "premortum");
        printk("channel id as seen by get_entry_by_channel_id: %d", entry.channel_id);
        if (entry.channel_id == channel_id) {
            debug(entry.msg_value);

            return &minor_arr[minor][i];


        }
    }
    printk(KERN_ERR "postmortum");
    debug_pointer(size_of_lists);//to eliminate warnings
    return NULL;
}

static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset) {
    unsigned int minor;
    short i;
    short returned;
    msg *entry;
    int channel_id;

    debug("before no channel read");
    if (no_channel(file)) {
        debug("found no channel");
        return -EINVAL;
    }
    debug("before reading channel_id");
    channel_id = ((private_data_type *) file->private_data)->channel_id;
    minor = ((private_data_type *) file->private_data)->minor;
    printk("read minor: %d", minor);
    entry = get_entry_by_channel_id(buffer, channel_id, minor);

    if (entry == NULL) {
        debug("in EWOULDBLOCK");
        return -EWOULDBLOCK; //resource temproray unavailable
    }

    if (entry->len > length)
        return -ENOSPC;
    for (i = 0; i < entry->len; i++) {
        int put_user_status = put_user(entry->msg_value[i], &buffer[i]);
        if (put_user_status != 0)
            return -EINVAL;
    }
    returned = entry->len;
    return returned;
}


//invariant: old versions always come after the most updated
static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset) {
    int i;
    msg new_msg;
    char *priv_buffer;
    int minor;
    int channel_id;
    int status;
    minor = ((private_data_type *) file->private_data)->minor;
    printk("shouldn't be 0: %d", ((private_data_type *) file->private_data)->minor);
    if (no_channel(file))
        return -EINVAL;
    printk("is buffer NULL %d", buffer == NULL);
    if (buffer == NULL)
        return -EINVAL;
    debug("device write");
    channel_id = ((private_data_type *) file->private_data)->channel_id;
    debug("device write minor");
    if (length == 0 || length > msg_len)
        return -EMSGSIZE;

    new_msg.channel_id = channel_id;
    new_msg.len = length;
    priv_buffer = new_msg.msg_value;
    debug("device write before for");
    printk("%zu", length);
    for (i = 0; i < length; i++) {
        int status_get_user = get_user(priv_buffer[i], &buffer[i]);
        if (status_get_user != 0)
            return -EINVAL;
    }
    status = add2list(minor, new_msg);
    if (!status)
        return -ENOMEM;
    printk("viewed length is :%d,status is :%d", size_of_lists[minor], status);
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

    ((private_data_type *) file->private_data)->channel_id = channel_id;

    return OK;
}


static int device_release(struct inode *inode, struct file *file) {
    void *private_data = file->private_data;
    kfree(private_data);

    return OK;

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
    printk("%d", MAJOR_NUM);
    status = register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &Fops);
    major = MAJOR_NUM;
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
    int i;
    for (i = 0; i < channel_num; i++)
        kfree(minor_arr[i]);
    kfree(minor_arr);
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

module_init(simple_init);

module_exit(simple_cleanup);