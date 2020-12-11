#include <stdio.h>
#include <string.h>
#include <errno.h>

void error_handler(int status) {
    if (status < 0) {
        perror(strerror(errno));
    }
}


int main(int argc, char *argv[]) {

}