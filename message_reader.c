#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "message_slot.h"
#include "error_handler.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>


void check_num_of_args(int argc) {
    printf("%d",argc);
    if (argc != 2) {
        fprintf(stderr, "incorrect number of argument\n");
        exit(1);
    }
}

void open_file(char *const *argv, int *status) {
    *status = open(argv[1], O_RDONLY);
    error_handler(*status);
}

void set_channel(char *const *argv, int file_status, unsigned long channel_id) {
    channel_id = atoi(argv[2]);
    int ioctl_status;
    ioctl_status = ioctl(file_status, MSG_SLOT_CHANNEL, channel_id);

    error_handler(ioctl_status);
}

int read_msg(int file_status, char *buffer) {
    int read_status;

    read_status = read(file_status, buffer, msg_len);
    error_handler(read_status);
    return read_status;
}

void write2console(const char *buffer, int read_status) {
    int write_status = write(STDOUT_FILENO, buffer, read_status) == -1;
    error_handler(write_status);
}

void close_file(int file_status) {
    int close_status = close(file_status);
    error_handler(close_status);
}

int main(int argc, char *argv[]) {
    int file_status;
    //change from here.
    char buffer[msg_len];
    unsigned long channel_id=0;

    check_num_of_args(argc);

    open_file(argv, &file_status);

    set_channel(argv, file_status, channel_id);

    int read_status = read_msg(file_status, buffer);

    write2console(buffer, read_status);

    close_file(file_status);
    return 0;

}