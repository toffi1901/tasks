#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>


#define BUF_SIZE 256

int open_fifo(const char *fname) {
    if (access(fname, F_OK) != 0) {
        if (mkfifo(fname, 0666) == -1) {
            return -1;
        }
    }
    int fd = open(fname, O_RDWR);
    if (fd == -1) {
        return -1;
    }
    return fd;
}

void chat_read(int fd) {
    char buf[BUF_SIZE];
    while (1) {
        ssize_t bytes_read = read(fd, buf, BUF_SIZE - 1);
        if (bytes_read <= 0){
            break;
        }
        buf[bytes_read] = '\0';
        printf("%s", buf);
        fflush(stdout);

        if (fgets(buf, BUF_SIZE, stdin) == NULL){
            break;
        }
        if (strcmp(buf, "exit\n") == 0){
            break;
        }
        write(fd, buf, strlen(buf));
    }
}

void chat_write(int fd) {
    char buf[BUF_SIZE];
    while (1) {
        if (fgets(buf, BUF_SIZE, stdin) == NULL){
            break;
        }
        if (strcmp(buf, "exit\n") == 0){
            break;
        }
        write(fd, buf, strlen(buf));

        ssize_t bytes_read = read(fd, buf, BUF_SIZE - 1);
        if (bytes_read <= 0){
            break;
        }
        buf[bytes_read] = '\0';
        printf("%s", buf);
        fflush(stdout);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) return 1;

    int fd = open_fifo(argv[1]);
    if (fd == -1){
        printf("Error of opening fifo\n");
        return 1;
    }

    fd_set fds;
    struct timeval tv = {0, 0};
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    int flag = select(fd + 1, &fds, NULL, NULL, &tv); //проверяем есть ли доступные для чтения данные, если нет, то записываем

    if (flag > 0) {
        chat_read(fd);
    } else if (flag == 0) {
        chat_write(fd);
    } else {
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}
