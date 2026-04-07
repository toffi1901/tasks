#include <stdio.h>
#include <stdlib.h>


int *find_sequence(int *arr, int len, int *max_len, int *start){
    *max_len = 1;
    *start = 0;
    int sum = 0;
    int max_sum = 0;
    int cur_len = 1;
    for (int i = 1; i < len; ++i){
        if (arr[i] > arr[i-1]){
            cur_len++;
            sum += arr[i];
            if (cur_len > *max_len || (cur_len == *max_len && sum > max_sum)){
                *max_len = cur_len;
                max_sum = sum;
                *start =  i - *max_len + 1;
            }
        }
        else{
            cur_len = 1;
            sum = 0;
        }
         
    }
    int k = 0;
    int *res = malloc(*max_len * sizeof(int));
    for (int i = *start; i < *max_len + *start; ++i) {
        res[k++] = arr[i];
    }
    return res;
}

int main(){
    int *input = NULL;
    int len = 0;
    int num;
    int cap = 0;
    while (scanf("%d", &num) == 1){
        if (len >= cap){
            cap = (cap == 0) ? 2 : cap * 2;
            input = realloc(input, cap * sizeof(int));
        }
        input[len++] = num;
        char c = getchar();
        if (c == '\n' || c == EOF){
            break;
        }
    }
    int max_len = 0, start = 0;
    int *result = find_sequence(input, len, &max_len, &start);
    for (int i = 0; i < max_len; ++i){
        printf("%d ", result[i]);
    }
    free(input);
    free(result);
    return 0;
}