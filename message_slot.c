
#include "message_slot.h"
#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/string.h>
#include <errno.h>
#include "sys/types.h"
#include "linux/list.h"
#include "linux/slab.h"
#include "linux/uaccess.h"

#define msg_len 128
typedef struct {
    struct list_head list;
    char msg[msg_len];
    short len;
    unsigned int minor;

} msg;

static struct msg **channel_list;

static int device_open(struct inode *inode, struct file *file) {
    //  msg **channel_list = kcalloc(sizeof(msg*), 256, GFP_KERNEL);
    channel_list = kcalloc(sizeof(*channel_list), 256, GFP_KERNEL);


}

static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset) {
    unsigned int minor = iminor(file);
    struct list_head *pos;
    msg * msg_list = (channel_list[minor]);

    list_for_each(pos, &msg_list->list) {

        msg * entry = list_entry((pos), msg, list);
        if (entry->minor == minor) {
            for (short i = 0; i < entry->len; i++)
                put_user(&entry->msg[i], &buffer[i]);

        }
    }
}

static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset) {
    int i;
    if (buffer == NULL)
        return -EINVAL;
    const int channel_id = file->private_data->channel_id;
    if (length == 0 || length > msg_len)
        return -EMSGSIZE;
    msg *new_msg = kcalloc(sizeof(new_msg), 1, GFP_KERNEL);
    char *priv_buffer = kmalloc(sizeof(char), msg_len);
    for (i = 0; i < msg_len; i++)
        get_user(priv_buffer[i], &buffer[i]);
    list_add(&channel_list[channel_id], &new_msg->list);
    return i;
}

static long invalid_ioctl() {
    return -EINVAL;
}

static long device_ioctl(struct file *file, unsigned int ioctl_command_id, unsigned long channel) {

    if (ioctl_command_id != MSG_SLOT_CHANNEL || channel == 0) {
        return invalid_ioctl();
    }
    unsigned int minor = iminor(file);
    if (channel_list[minor] == NULL) {
        channel_list[minor] = kcalloc(sizeof(channel_list), 1, GFP_KERNEL);

    }

    file->private_data->channel_id = minor;


}

static int device_release(struct inode *, struct file *) {

}

struct file_operations Fops =
        {
                .owner      = THIS_MODULE, // Required for correct count of module usage. This prevents the module from being removed while used.
                .read           = device_read,
                .write          = device_write,
                .open           = device_open,
                .release        = device_release

        };

