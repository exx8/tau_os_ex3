#include "message_slot.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void error_handler(int status) {
    if (status < 0) {
        perror("error:");
        exit(1);
    }

}
void validate_num_of_argc(int argc) {
    if (argc != 4) {
        fprintf(stderr, "incorrect num of arguments\n");
        exit(1);
    }
}

int open_file_for_sender(char *const *argv) {
    int file_status = open(argv[1], O_WRONLY);
    error_handler(file_status);
    return file_status;
}

int get_channel_id(char *const *argv) {
    int channel_id = atoi(argv[2]);
    error_handler(channel_id);
    return channel_id;
}

void ioctl_call(int ioctl_status, int channel_id, int file_status) {
    ioctl_status = ioctl(file_status, MSG_SLOT_CHANNEL, channel_id);
    error_handler(ioctl_status);
}

int main(int argc, char *argv[]) {
    int ioctl_status=0;
    int channel_id;
    validate_num_of_argc(argc);

    int file_status = open_file_for_sender(argv);
    channel_id = get_channel_id(argv);

    ioctl_call(ioctl_status, channel_id, file_status);

    size_t length;
    length = strlen(argv[3]);
    ioctl_status = write(file_status, argv[3], length);
    if (ioctl_status != length) {
        perror("error:");
        exit(1);
    }

    int close_status = close(file_status);

    error_handler(close_status);

    return 0;
}