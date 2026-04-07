#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

//В данной задаче используется параллельная сортировка PSRS(Paralel sorting by regular sampling)


typedef struct{
    int *data; //кусочек данных для данного потока
    int size; //размер данных
    int p; //всего потоков
    int thread_num; //номер потока
    int *help_array; //указатель на область в массиве all_samples(размером p^2)
    int *separators; //массив разделителей
    pthread_mutex_t *mutex;
    int *ready_th; //флаг для того, чтобы опредедлить готовность всех потоков
    int *sorted; //флаг, что поток отсортировал свой кусочек данных
    pthread_cond_t *cond;

    //для обмена и слияния, шаги 7-8 PSRS
    int *first_phase; //счетчик для определения готовности всех потоков при сборе размеров батчей
    int *second_phase; //счетчик для определения готовности всех потоков при копировании в общий буфер 
    int *input; //исходные данные
    int *buf; //буфер для формирования раезультата, куда каждый поток будет вписывать данные на нужные места, указанные в матрице
    int **offsets; //матрица[i][j],  i поток отдает count элементов потоку j
    int *current_pos; 
    int *bias;
} thread_data;

int comp(const void *a, const void *b){
    return (*(int *)a - *(int *)b);
}

//формируем массив разделителей из элементо вспомогательно массива, под индексами p [p/2] - 1, 2p [p/2] - 1... 
void sep_array(int p, int *separators, int *samples) {
    int offset = p/2 - 1;
    for (int i = 1; i < p; i++) {
        separators[i-1] = samples[p*i + offset];
    }
}

//делим данные в каждом потоке на интервалы, используя разделители
void split_batches(thread_data *th, int ***batches, int **b_sizes){
    int p = th->p;
    *b_sizes = calloc(p, sizeof(int));

    for (int i = 0; i < th->size; ++i){
        int batch = 0;
        int val = th->data[i];
        while (batch < p - 1 && val > (th->separators)[batch]){
            batch++;
        }
        (*b_sizes)[batch]++;
    }
    *batches = malloc(p * sizeof(int*));
    for (int i = 0; i < p; ++i) {
        (*batches)[i] = malloc((*b_sizes)[i] * sizeof(int));
    }
    int *groups = calloc(p, sizeof(int));
    for (int i = 0; i < th->size; ++i){
        int batch = 0;
        int val = th->data[i];
        while (batch < p - 1 && val > (th->separators)[batch]){
            batch++;
        }
        (*batches)[batch][groups[batch]++] = val;
    }
    free(groups);
}

//сливаем группы элементов в массивы, создаем матрицу для каждого потока, чтобы определить 
//какие батчи в какие процессы попадают
void merge(thread_data *th, int **batches, int *b_sizes){
    int tnum = th->thread_num;
    int p = th->p;
    int **offsets = th->offsets;
    int *buf = th->buf;
    int *local_offset = th->current_pos;
    int *input = th->input;
    int *bias = th->bias;
    pthread_mutex_t *mutex = th->mutex;
    pthread_cond_t *cond = th->cond;
    int *first_phase = th->first_phase;
    int *second_phase = th->second_phase;

    //в матрицу записываем размеры корзин
    for (int i = 0; i < p; ++i){
        offsets[tnum][i] = b_sizes[i]; //поток записывает размеры своих батчей,
    }
//ждем, пока все потоки запишут данные о батчах
    pthread_mutex_lock(mutex);
    (*first_phase)++;
    if (*first_phase == p) {
        *first_phase = 0;              
        pthread_cond_broadcast(cond);
    } else {
        while (*first_phase != 0) {
            pthread_cond_wait(cond, mutex);
        }
    }
    pthread_mutex_unlock(mutex);

  //поток определяет, сколько элементов он получит от остальных
    int *counts = calloc(p, sizeof(int)); //сколько элментов будет у потока j, массив кол-ва элементов
    for (int j = 0; j < p; j++) {
        for (int i = 0; i < p; i++) {
            counts[j] += offsets[i][j];
        }
    }
//смещения, чтобы в buf поток записывал данные на свои места
    int *biases = malloc(p * sizeof(int));
    biases[0] = 0;
    for (int j = 1; j < p; j++) {
        biases[j] = biases[j-1] + counts[j-1];//то есть j поток пишет с biases[j] индекса, то есть это следущий индекс от начала записи предыдущего потока + кол-во элементов предыдущего потока
    }

    pthread_mutex_lock(mutex); //мьтекс испольуется, чтобы защитить общие перменные от одновременного обращения(например счетчик ready_th)
    for (int j = 0; j < p; j++) {
        int cur = b_sizes[j]; //кол-во элементов в батче
        if (cur > 0) {
            int pos = biases[j] + local_offset[j];
            memcpy(buf + pos, batches[j], cur * sizeof(int));
            local_offset[j] += cur; //текущая позиция в buf, необхоима, чтобы потоки писали в свою свободную облатсь данных
        }
    }
    (*second_phase)++; //синхронизация копирования
    if (*second_phase == p) {
        *second_phase = 0;
        memset(local_offset, 0, p * sizeof(int));
        pthread_cond_broadcast(cond);
    } else {
        while (*second_phase != 0) {
            pthread_cond_wait(cond, mutex);
        }
    }
    pthread_mutex_unlock(mutex);
//каждый поток забирает сваю часть из буфера и сорирует ее
    int tmp = counts[tnum];
    int *my_data = malloc(tmp * sizeof(int));
    if (tmp > 0) {
        memcpy(my_data, buf + biases[tnum], tmp * sizeof(int));
    }
    qsort(my_data, tmp, sizeof(int), comp);
    memcpy(input + biases[tnum], my_data, tmp * sizeof(int));

    free(my_data);
    free(counts);
    free(biases);
}

void *one_thread(void *arg) {
    thread_data *th = (thread_data*)arg;
    int tnum = th->thread_num;
    int p = th->p;
    int *data = th->data;
    int size = th->size;
//2 шаг - на каждом процессоре запускаем быструю сортировку
    qsort(data, size, sizeof(int), comp);
//формируем вспомогательный массив из элементов каждого процессора
//каждый поток вписывает в этот массив  p элментов, которые рапсредлены равномерно, это нужно, 
//чтобы дальше выбрать разделители
    for (int j = 0; j < p; j++) {
        int idx = (p == 1) ? 0 : (j * (size - 1)) / (p - 1);
        th->help_array[j] = data[idx];
    }

    //сортируем вспомогательный массив с помощью qsort
    pthread_mutex_lock(th->mutex);
    (*th->ready_th)++;
    if (*th->ready_th == p) {
        pthread_cond_broadcast(th->cond); //активирем все потоки, когда все закончили вычисления
    } else {
        while (*th->ready_th != p) {
            pthread_cond_wait(th->cond, th->mutex);
        }
    }

    if (tnum == 0) {
        qsort(th->help_array, p * p, sizeof(int), comp);
        sep_array(p, th->separators, th->help_array); //шаг 4
        *th->sorted = 1;
        pthread_cond_broadcast(th->cond);
    } else {
        while (!(*th->sorted)) {
            pthread_cond_wait(th->cond, th->mutex);
        }
    }
    pthread_mutex_unlock(th->mutex);

    int *b_sizes = NULL;
    int **batches = NULL;
    split_batches(th, &batches, &b_sizes);   

    merge(th, batches, b_sizes);

    for (int i = 0; i < p; i++) free(batches[i]);
    free(batches);
    free(b_sizes);

    return NULL;
}

int main(int argc, char *argv[]){
    if (argc < 2){
        printf("Error: Not all arguments was inputed");
        return 1;
    }
    int p = atoi(argv[1]);
    if (p <= 0) p = 1;

    int *input = NULL;
    int n = 0;
    int capacity = 0;
    int num;
    while (scanf("%d", &num) == 1) {
        if (n == capacity) {
            capacity = (capacity == 0) ? 8 : capacity * 2;
            input = realloc(input, capacity * sizeof(int));
        }
        input[n++] = num;
    }

    if (n == 0) {
        printf("\n");
        free(input);
        return 0;
    }

    if (p > n){
        p = n;
    }
    int ready_th = 0;
    int sorted = 0;
    int first_phase = 0;
    int second_phase = 0;

    int whole = n/p; //исходный массив из n элементов будем делить поровну между p процессорами
    int remain = n % p;
    int *sizes = malloc(p * sizeof(int));
    int *bias = malloc(p * sizeof(int));
    for (int i = 0; i < p; ++i){
        sizes[i] = whole + (i < remain ? 1 : 0); //если нацело не делится, то добавляем элементы из остатка 
        bias[i] = (i == 0) ? 0 : sizes[i - 1] + bias[i - 1];
    }

    int *all_samples = malloc(p * p * sizeof(int));
    int *sep_arr = (p > 1) ? malloc((p-1) * sizeof(int)) : NULL;

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

    pthread_t *threads = malloc(p * sizeof(pthread_t));
    thread_data *th = malloc(p * sizeof(thread_data));

    int *buf = malloc(n * sizeof(int));
    int **offsets = malloc(p * sizeof(int*));
    for (int i = 0; i < p; i++) {
        offsets[i] = calloc(p, sizeof(int));
    }
    int *local_offsets = calloc(p, sizeof(int));


    for (int i = 0; i < p; ++i){
        th[i].data = input + bias[i];
        th[i].size = sizes[i];
        th[i].p = p;
        th[i].thread_num = i;
        th[i].help_array = all_samples + i * p;
        th[i].separators = sep_arr;
        th[i].mutex = &mutex;
        th[i].cond = &cond;
        th[i].ready_th = &ready_th;
        th[i].sorted = &sorted;
        th[i].first_phase = &first_phase;
        th[i].second_phase = &second_phase;
        th[i].input = input;
        th[i].buf = buf;
        th[i].offsets = offsets;
        th[i].current_pos = local_offsets;
        th[i].bias = bias;
        pthread_create(&threads[i], NULL, one_thread, &th[i]);
    }

    for (int i = 0; i < p; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("Sorted array:\n");
    for (int i = 0; i < n; i++) {
        printf("%d ", input[i]);
    }
    printf("\n");

    free(input);
    free(sizes); free(bias);
    free(all_samples);
    if (sep_arr){
        free(sep_arr);
    }
    free(buf);
    for (int i = 0; i < p; i++){
        free(offsets[i]);
    }
    free(offsets);
    free(local_offsets);
    free(threads);
    free(th);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

}
