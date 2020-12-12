
#include "message_slot.h"
#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/string.h>
#include <errno.h>
#include <malloc.h>
#include "sys/types.h"
#include "linux/list.h"
#include "linux/slab.h"
#include "linux/uaccess.h"


typedef struct {
    struct list_head list;
    char msg_value[msg_len];
    short len;
    unsigned int minor;

} msg;

static struct msg **minor_arr;

static unsigned int get_minor(const struct file *file) {
    unsigned int minor = iminor(file);
    return minor;
}

static int device_open(struct inode *inode, struct file *file) {
    minor_arr = kcalloc(sizeof(*minor_arr), channel_num, GFP_KERNEL);
    unsigned int minor = get_minor(file);
    if (minor_arr[minor] == NULL) {
        minor_arr[minor] = kcalloc(sizeof(minor_arr), 1, GFP_KERNEL);

    }

}

static bool no_channel(const struct file *file) { return file->private_data == NULL; }

static msg *get_entry_by_minor(const char *buffer, unsigned int minor) {
    struct list_head *pos;
    msg *msg_list = (minor_arr[minor]);

    list_for_each(pos, &msg_list->list) {

        msg *entry = list_entry((pos), msg, list);
        if (entry->minor == minor) {
            return entry;


        }
    }
    return NULL;
}

static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset) {
    if (no_channel(file))
        return -EINVAL;
    unsigned int minor = file->private_data->channel_id;
    msg *entry = get_entry_by_minor(buffer, minor);
    if (entry == NULL)
        return -EWOULDBLOCK;
    if (entry->len > length)
        return -ENOSPC;
    for (short i = 0; i < entry->len; i++)
        put_user(&entry->msg_value[i], &buffer[i]);
    kfree(entry); //might it be free?
}


//invariant: old versions always come after the most updated
static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset) {
    int i;
    if (no_channel(file))
        return -EINVAL;
    if (buffer == NULL)
        return -EINVAL;
    const int minor = file->private_data->channel_id;
    if (length == 0 || length > msg_len)
        return -EMSGSIZE;
    msg *new_msg = kcalloc(sizeof(new_msg), 1, GFP_KERNEL);
    char *priv_buffer = kmalloc(sizeof(char), msg_len);
    for (i = 0; i < msg_len; i++)
        get_user(priv_buffer[i], &buffer[i]);
    list_add(&minor_arr[minor], &new_msg->list);
    return i;
}

static long invalid_ioctl() {
    return -EINVAL;
}

static long device_ioctl(struct file *file, unsigned int ioctl_command_id, unsigned long channel) {
    unsigned int minor = iminor(file);

    if (ioctl_command_id != MSG_SLOT_CHANNEL || channel == 0) {
        return invalid_ioctl();
    }


    file->private_data->channel_id = minor;

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

    msg *minor_list = &minor_list[get_minor(file)];
    struct list_head list2send = minor_list->list;
    release_list(list2send);


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

