#include "message_slot.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error_handler.h"

void validate_num_of_argc(int argc) {
    if (argc != 4) {
        fprintf(stderr, "incorrect num of arguments\n");
        exit(1);
    }
}

int main(int argc, char *argv[]) {

    int ioctl_status;
    unsigned long channel_id;
    long length;

    validate_num_of_argc(argc);

    int file_status = open(argv[1], O_WRONLY);


    channel_id = atoi(argv[2]);
    ioctl_status = ioctl(file_status, MSG_SLOT_CHANNEL, channel_id);
    if (ioctl_status != OK) {
        perror("ioctl");
        exit(1);
    }

    length = strlen(argv[3]);
    ioctl_status = write(file_status, argv[3], length);
    if (ioctl_status != length) {
        perror("write");
        exit(1);
    }

    close(file_status);
    return 0;
}