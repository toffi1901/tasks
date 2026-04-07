#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

void free_array(char **arr, int cap){
    if (arr){
        for (int i = 0; i < cap; ++i){
            if (arr[i] != NULL){
                free(arr[i]);
            }   
        }
        free(arr);
    }
}
//функция для генераци результата, в которую передаются все опции, разбитые на группы
char **create_result(char **long_opt, char **short_opt, char **non_opt, int short_count, int long_count, int non_count){
    char **result = malloc(4 * sizeof(char*));   
    if (!result){
        return NULL;
    }
    char *short_line = NULL;
    char *long_line = NULL;
    char *non_line = NULL;
    int len = strlen("Short options: ");
    for (int i = 0; i < short_count; ++i) {
        len += strlen(short_opt[i]) + 2;
        if (i < short_count - 1){
            len += 1;
        }
    }
    
    short_line = calloc(len + 1, sizeof(char));
    strcpy(short_line, "Short options: ");
    for (int i = 0; i < short_count; ++i){
        strcat(short_line, "'");
        strcat(short_line, short_opt[i]);
        strcat(short_line, "'");
        if (i < short_count - 1){
            strcat(short_line, " ");
        }
    }
    len = strlen("Long options: ");
    for (int i = 0; i < long_count; ++i){
        len += strlen(long_opt[i]) + 2;
        if (i < long_count - 1){
            len += 1;
        }
    }

    long_line = calloc(len + 1, sizeof(char));
    strcpy(long_line, "Long options: ");
    for (int i = 0; i < long_count; ++i){
        strcat(long_line, "'");
        strcat(long_line, long_opt[i]);
        strcat(long_line, "'");
        if (i < long_count - 1){
            strcat(long_line, " ");
        }
    }

    len = strlen("Non options: ");
    for (int i = 0; i < non_count; ++i){
        len += strlen(non_opt[i]) + 2;
        if (i < non_count - 1){
            len += 1;
        }
    }
    non_line = calloc(len + 1, sizeof(char));
    strcpy(non_line, "Non options: ");
    for (int i = 0; i < non_count; ++i){
        strcat(non_line, "'");
        strcat(non_line, non_opt[i]);
        strcat(non_line, "'");
        if (i < non_count - 1){
            strcat(non_line, " ");
        }
    }

    result[0] = short_line;
    result[1] = long_line;
    result[2] = non_line;
    result[3] = NULL;

    return result;
}


char **sys_call(int argc, char **argv){
    char **short_opts = NULL;  
    int short_count = 0, short_cap = 0;
    
    char **long_opts = NULL;   
    int long_count = 0, long_cap = 0;
    
    char **non_opts = NULL;    
    int non_count = 0, non_cap = 0;
    
    char *error = NULL;  
    
    static struct option long_options[] = {
        {"elbrus", required_argument, NULL, 'e'},
        {0, 0, 0, 0}
    };
    
    int command;
    int long_index = 0;
    while((command = getopt_long(argc, argv, "mcst", long_options, &long_index)) != -1){
        if (error){
            break;
        }
        switch(command){
            case 'm': case 'c': case 's': case 't':
                if (short_count >= short_cap){
                    short_cap = short_cap ? short_cap * 2 : 4;
                    short_opts = realloc(short_opts, short_cap * sizeof(char *));
                }
                short_opts[short_count] = malloc(2 * sizeof(char));
                sprintf(short_opts[short_count++], "%c", command);
                break;
            case 'e':
                if (strcmp(optarg, "16c") == 0 || strcmp(optarg, "2c3") == 0 || strcmp(optarg, "1c+") == 0 || strcmp(optarg, "2c+") == 0 || strcmp(optarg, "4c") == 0 || strcmp(optarg, "8c") == 0) {
                    if (long_count >= long_cap){
                    long_cap = long_cap ? long_cap * 2 : 4;
                    long_opts = realloc(long_opts, long_cap * sizeof(char *));
                }
                char *opt = calloc(strlen("elbrus=") + strlen(optarg) + 1, sizeof(char));
                    sprintf(opt, "elbrus=%s", optarg);
                    long_opts[long_count++] = opt;
                }
                else{
                    error = calloc(strlen("elbrus=") + strlen(optarg) + 3, sizeof(char));
                    sprintf(error, "'elbrus=%s'", optarg);
                }
                break;

            case '?':
                char *bad = argv[optind - 1];
                if (bad[0] == '-' && bad[1] == '-') {
                    error = calloc(strlen(bad) + 1, sizeof(char));
                    strcat(error, "'");
                    error = strcat(error, bad + 2);  
                    strcat(error, "'");
                } else {
                    error = calloc(4, sizeof(char));
                    sprintf(error, "'%c'", optopt);
                }
                break;
            default:
            break;

        }
    }

    if (error){
        free_array(short_opts, short_count);
        free_array(long_opts, long_count);
        free_array(non_opts, non_count);
        int len = strlen("Incorrect option: ");
        char **result = malloc(2 * sizeof(char*));   
        if (!result){
            return NULL;
        }
        result[0] = calloc(len + strlen(error) + 1, sizeof(char));
        strcat(result[0], "Incorrect option: "); 
        strcat(result[0], error);
        free(error);
        result[1] = NULL;
        return result;
    }

    for (int i = optind; i < argc; ++i){
        if (non_count >= non_cap){
            non_cap = non_cap ? non_cap * 2 : 4;
            non_opts = realloc(non_opts, non_cap * sizeof(char *));
        }
        non_opts[non_count] = calloc(strlen(argv[i]) + 1, sizeof(char));
        strcpy(non_opts[non_count++], argv[i]);
    }

    char **result = create_result(long_opts, short_opts, non_opts, short_count, long_count, non_count);
    free_array(short_opts, short_count);
    free_array(long_opts, long_count);
    free_array(non_opts, non_count);
    return result;
}


int main(int argc, char *argv[]){
    char **result = NULL;
    result = sys_call(argc, argv);
    if (result[1] == NULL) {
        printf("%s\n", result[0]);
        free_array(result, 2);
    } else {
        printf("%s\n", result[0]);
        printf("%s\n", result[1]);
        printf("%s\n", result[2]);
        free_array(result, 4);
    }
    return 0;
}
