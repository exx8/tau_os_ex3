message_sender.c
        ע
A

        B
Type
        C
Size
908 bytes
        Storage used
908 bytes
        Location
hw3
        Owner
me
        Modified
Jun 26, 2020 by me
Opened
3:19 PM by me
        Created
Jun 26, 2020 with Google Drive Web
Add a description
        Viewers can download
#include "message_slot.h"


#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int file_status;
    int ioctl_status;
    unsigned long channel_id;
    long length;

    if (argc < 4) {
        fprintf(stderr, "Please give exactly 4 arguemnts\n");
        exit(1);
    }

    file_status = open(argv[1], O_WRONLY );
    if(file_status < 0 ) {
        perror("open");
        exit(1);
    }

    channel_id = atoi(argv[2]);
    ioctl_status = ioctl(file_status, MSG_SLOT_CHANNEL, channel_id);
    if (ioctl_status != SUCCESS) {
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