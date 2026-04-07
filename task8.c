#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define SIZE 100
 
static volatile sig_atomic_t current_i;
static volatile sig_atomic_t current_j;
static volatile sig_atomic_t current_k;


void handle_sigint(int sig){
    char buf[64];
    int len = snprintf(buf, sizeof(buf), "\n%d %d\n%d %d\n%d %d\n", current_i + 1, current_k + 1, current_k + 1, current_j + 1, current_i + 1, current_j + 1);
    write(STDOUT_FILENO, buf, len);
    signal(SIGINT, SIG_DFL);
}

void mul_matrix(int a[][SIZE], int b[][SIZE]){
    int c[SIZE][SIZE];
    for (int i = 0; i < SIZE; ++i){
        for (int j = 0; j < SIZE; ++j){
            int sum = 0;
            for (int k = 0; k < SIZE; ++k){
                current_i = i;
                current_j = j;
                current_k = k;
                sum += a[i][k] * b[k][j];
                usleep(500000);
            }
            c[i][j] = sum;
        }
    }
}

int main(int argc, char *argv[]){
    if (argc != 2){
        return 1;
    }

    int a[SIZE][SIZE];
    int b[SIZE][SIZE];
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            a[i][j] = 1;
            b[i][j] = 1;
        }
    }

    if (strcmp(argv[1], "--signal") == 0){
        if (signal(SIGINT, handle_sigint) == SIG_ERR){
            return 1;
        }
    }
    else if (strcmp(argv[1], "--sigaction") == 0){
        struct sigaction action;
        memset(&action, 0, sizeof(action));
        action.sa_handler = handle_sigint;
        action.sa_flags = 0;
        if (sigaction(SIGINT, &action, NULL) == -1){
            return 1;
        }
    };
    mul_matrix(a, b);
    return 0;
}