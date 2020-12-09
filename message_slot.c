
#include "message_slot.h"
#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/string.h>
#include <errno.h>
#include "sys/types.h"
#include "linux/list.h"
#include "linux/slab.h"

typedef struct {
    struct list_head list;

} msg_lst;

static msg_lst *channel_list;

static int device_open(struct inode *inode, struct file *file) {
    msg_lst **channel_list = kcalloc(sizeof(msg_lst*), 256, GFP_KERNEL);

    unsigned int minor = iminor(inode);
    if (channel_list[minor] == NULL) {

    }


}

static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset) {

}

static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset) {

}

static long invalid_ioctl() {
    errno = EINVAL;
    return -1;
}

static long device_ioctl(struct file *file, unsigned int ioctl_command_id, unsigned long channel) {

    if (ioctl_command_id != MSG_SLOT_CHANNEL || channel == 0) {
        return invalid_ioctl();
    }


}

struct file_operations Fops =
        {
                .owner      = THIS_MODULE, // Required for correct count of module usage. This prevents the module from being removed while used.
                .read           = device_read,
                .write          = device_write,
                .open           = device_open,

        };

