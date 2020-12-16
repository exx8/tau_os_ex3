
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
    int minor;
} private_data_type;
typedef struct {
    struct list_head list;
    char msg_value[msg_len];
    short len;
    unsigned int channel_id;

} msg;

static msg **minor_arr;
static void debug(char *const fmt) {
    printk(fmt); }
    static void debug_pointer(void * pointer)
    {
    printk("%p",pointer);
    }

static unsigned int get_minor(const struct inode *inode) {
    unsigned int minor = iminor(inode);
    return minor;
}

static int device_open(struct inode *inode, struct file *file) {
    unsigned int minor;
    private_data_type  * private_data=kcalloc(sizeof(private_data),1,GFP_KERNEL);
    minor_arr = kcalloc(sizeof(*minor_arr), channel_num, GFP_KERNEL);
    minor = get_minor(inode);
    printk("open minor %d \n",minor);
    private_data->minor=minor;
    private_data->channel_id=0;
    file->private_data=private_data;

    if (minor_arr[minor] == NULL) {
        minor_arr[minor] = kcalloc(sizeof(minor_arr), 1, GFP_KERNEL);

    }
    return OK;
}

static bool no_channel(const struct file *file) { return file->private_data == NULL; }

static msg *get_entry_by_channel_id(const char *buffer, unsigned int channel_id) {
    struct list_head *pos;
    msg *msg_list = (minor_arr[channel_id]);

    list_for_each(pos, &msg_list->list) {

        msg *entry = list_entry((pos), msg, list);
        if (entry->channel_id == channel_id) {
            return entry;


        }
    }
    return NULL;
}

static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset) {
    unsigned int minor;
    short i;
    short returned;
    msg *entry;
    int channel_id;
    if (no_channel(file))
        return -EINVAL;
    channel_id = ((private_data_type*)file->private_data)->channel_id;
    entry = get_entry_by_channel_id(buffer, minor);
    if (entry == NULL)
        return -EWOULDBLOCK;
    if (entry->len > length)
        return -ENOSPC;
    for (i = 0; i < entry->len; i++)
        put_user(entry->msg_value[i], &buffer[i]);
    returned = entry->len;
    kfree(entry); //might it be free?
    return returned;
}


//invariant: old versions always come after the most updated
static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset) {
    int i;
    msg *new_msg;
    char *priv_buffer;
    int minor;
    int channel_id;
    if (no_channel(file))
        return -EINVAL;
    if (buffer == NULL)
        return -EINVAL;
    debug("device write");
    channel_id = ((private_data_type*)file->private_data)->channel_id; // a problem
    debug("device write minor");

    if (length == 0 || length > msg_len)
        return -EMSGSIZE;

    new_msg = kcalloc(sizeof(new_msg), 1, GFP_KERNEL);
    new_msg->channel_id=channel_id;
    priv_buffer = kmalloc(sizeof(char), msg_len);
    debug("device write before for");

    for (i = 0; i < msg_len; i++)
        get_user(priv_buffer[i], &buffer[i]);
    debug("device write before list");
    debug_pointer(minor_arr[minor]);
    debug_pointer(&minor_arr[minor]->list);
    printk("check for minor %d",minor);
    list_add(&minor_arr[minor]->list, &new_msg->list);
    debug("device write list");

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

    file->private_data=kcalloc(sizeof(private_data_type),1,GFP_KERNEL);
    ((private_data_type*)file->private_data)->channel_id = channel_id; //wasn't init

    return OK;
}

static void release_list(struct list_head list) {

    struct list_head *pos, *q;

    list_for_each_safe(pos, q, &list) {
        msg *entry = list_entry(pos, msg, list);
        list_del(pos);
        kfree(entry); //might it be free?
    }
}

static int device_release(struct inode *inode, struct file *file) {

    msg *minor_list_ele = minor_arr[get_minor(inode)];
    struct list_head list2send = minor_list_ele->list;
    release_list(list2send);
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
    // Register driver capabilities. Obtain major num
    major = register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &Fops);

    // Negative values signify an error
    if (major < 0) {
        printk(KERN_ALERT "%s registraion failed for  %d\n",
               DEVICE_FILE_NAME, major);
        return major;
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