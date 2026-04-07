#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

//компараторы, которые используются в функциях сортировки для указания направления сортировки
int comp_plain(const void *a, const void *b){
    return strcmp(*(const char **)a, *(const char **)b);
}

int rcomp_plain(const void *a, const void *b){
    return strcmp(*(const char **)b, *(const char **)a);
}

int comp_lex(const void *a, const void *b){
    return strcasecmp(*(const char **)a, *(const char **)b);
}

int rcomp_lex(const void *a, const void *b){
    return strcasecmp(*(const char **)b, *(const char **)a);
}

int not_useful_char(const char *str){
    int len = strlen(str);
    for (int i = 0; i < len; ++i){
        if (!isspace(str[i])){
            return 1;
        }
    }
    return 0;
}

char **read_file(char *in_fname, int *buf_cap, int *lines){
    FILE *fd = fopen(in_fname, "r");
    if (!fd){
        return NULL;
    }
    char **buf = malloc((*buf_cap) * sizeof(char *));
    if (!buf){
        fclose(fd);
        return NULL;
    }

    buf[0] = NULL;
    int len = 0;
    size_t size = 0;

    while ((len = getline(&(buf[*lines]), &size, fd)) != -1){
        if (len > 0 && buf[*lines][len - 1] == '\n') {
            buf[*lines][len - 1] = '\0';
        }
        if (!not_useful_char(buf[*lines])){
            free(buf[*lines]);
            buf[*lines] = NULL;
            continue;
        }
        (*lines)++;
        if (*lines >= *buf_cap){
            *buf_cap *= 2;
            buf = realloc(buf, *buf_cap * sizeof(char *));
        }
        buf[*lines] = NULL;
    }
    fclose(fd);
    return buf;
}

//функция выбора типа сортировки
int sort_str(char **buf, int lines, char *mode){
    if (strcmp(mode, "plain") == 0){
        qsort(buf, lines, sizeof(char *), comp_plain);
    }
    else if (strcmp(mode, "rplain") == 0){
        qsort(buf, lines, sizeof(char *), rcomp_plain);
    }
    else if (strcmp(mode, "lex") == 0){
        qsort(buf, lines, sizeof(char *), comp_lex);
    }
    else if (strcmp(mode, "rlex") == 0){
        qsort(buf, lines, sizeof(char *), rcomp_lex);
    }
    else{
        return 1;
    }
    return 0;
}

int write_into_file(char *fname, char **buf, int lines){
    FILE *fout = fopen(fname, "w");
    if (!fout) {
        return 1;
    }
    for (int i = 0; i < lines; i++) {
        fprintf(fout, "%s\n", buf[i]);
    }
    fclose(fout);
    return 0;
}


int main(int argc, char *argv[]){
    if (argc != 4){
        printf("Error, too many arguments");
        return 1;
    }
    char *in_fname = argv[1];
    char *out_fname = argv[2];
    char *mode = argv[3];
    
    int lines = 0;
    int buf_cap = 2;
    char **buf = NULL;
    buf = read_file(in_fname, &buf_cap, &lines);
    if (!buf){
        printf("Error, file wasn't opened");
        return 1;
    }
    int res = sort_str(buf, lines, mode);
    if (res == 1){
        printf("Error, incorrect type of sort");
        for (int i = 0; i <= lines; i++) {
            free(buf[i]);
        }
        free(buf);
        return 1;
    }
    res = write_into_file(out_fname, buf, lines);
    if (res == 1){
        printf("Error, file wasn't opened");
        for (int i = 0; i <= lines; i++) {
            free(buf[i]);
        }
        free(buf);
        return 1;
    }
    for (int i = 0; i <= lines; i++) {
        free(buf[i]);
    }
    free(buf);
    return 0;
}
