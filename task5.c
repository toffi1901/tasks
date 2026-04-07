#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>  

int main(int argc, char * argv[]) {
    int fd = open("non-existant file", O_RDONLY);
    if (fd == -1) {
        perror("Error occured while opening non-existant file");
        fprintf(stderr, "errno = %d\n", errno);
        fprintf(stderr, "strerror(errno) = %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return 0;
}