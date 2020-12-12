//
// Created by eran on 12/12/2020.
//

#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "message_slot.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

void error_handler(int status) {
    if (status < 0) {
        fprintf(stderr, "%s \n", strerror(errno));
        exit(1);
    }

}