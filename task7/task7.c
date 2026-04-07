#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>

int read_data(FILE *fd, char *out_fname){    
    FILE *fd_out = fopen(out_fname, "w");
    if (!fd_out){
        return 1;
    }
    char buf[256];
    size_t len;
    while ((len = fread(buf, 1, sizeof(buf), fd)) > 0){
        fwrite(buf, 1, len ,fd_out);
    }
    fclose(fd_out);
    return 0;
}

void print_file(char *fname){
    FILE *fd = fopen(fname, "r");
    if (!fd){
        return;
    }
    printf("%s:\n", fname);
    char c;
    while ((c = fgetc(fd)) != EOF) {
        putchar(c);
    }
    printf("\n");
    fclose(fd);
}


int main(int argc, char *argv[]){
    if (argc < 2){
        return 1;
    }
    pid_t second = fork();
    if (second == -1) {
        perror("fork");
        return 1;
    }

    int result = 0;
//оба процесса читают файл параллеьно, но родительский процесс ждет завершения дочернего
    if (second == 0){
        FILE *fd = fopen(argv[1], "r");
        if (!fd){
            return 1;
        } 
        result = read_data(fd, "child_copy");
        fclose(fd);
        exit(0);
    }
    else{
        FILE *fd = fopen(argv[1], "r");
        if (!fd){
            return 1;
        } 
        result = read_data(fd, "parent_copy");
        wait(NULL);
        fclose(fd);
    }
    if (result != 0){
        printf("Error\n");
        return 1;
    }
    print_file("child_copy");
    print_file("parent_copy");
    return 0;
}
