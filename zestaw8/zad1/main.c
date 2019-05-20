#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <pthread.h>
#include <math.h>

#define log(format, ...) {printf("\033[1;31m Main: \033[0m"); printf(format, ##__VA_ARGS__);}
#define FAILURE_EXIT(code, format, ...) { log(format, ##__VA_ARGS__); exit(code);}

int **image = NULL;
int i_height;
int i_width;

double **filter = NULL;
int f_size;

int **result = NULL;

int thread_count;

int *threads;

int max(int a, int b){
    return a > b ? a : b;
}


double calculate_pixel(int x, int y){
    int i,j;
    double sum = 0;
    for(i = 0; i < f_size; i++){
        for(j = 0; j < f_size; j++){
            int h_expr = max(0, x - ceil(f_size / 2.0) + i);
            int w_expr = max(0, y - ceil(f_size / 2.0) + j);
            sum += h_expr < i_height && w_expr < i_width ? image[h_expr][w_expr] * filter[i][j] : 0;
        }
    }
    return round(sum);
}

struct timeval *thread_calculation_block(void *args){
    int *arg = (int*) args;
    int k = arg[0];
    struct timeval *start = malloc(sizeof(struct timeval));
    struct timeval *end = malloc(sizeof(struct timeval));
    struct timeval *res = malloc(sizeof(struct timeval));
    gettimeofday(start,NULL);
    int i,j;
    int x_start =ceil(k * i_width / (1.0*thread_count));
    int x_end = round((k + 1) * i_width / (1.0*thread_count)) - 1;
    for(i = x_start; i <= x_end; i++){
        for(j = 0; j < i_height; j++){
            result[j][i] = abs(calculate_pixel(j, i));
        }
    }
    gettimeofday(end,NULL);
    timersub(end,start,res);
    free(start);
    free(end);
    return res;
}

struct timeval *thread_calculation_interleaved(void *args){
    int *arg = (int*) args;
    int k = arg[0];
    struct timeval *start = malloc(sizeof(struct timeval));
    struct timeval *end = malloc(sizeof(struct timeval));
    struct timeval *res = malloc(sizeof(struct timeval));
    gettimeofday(start,NULL);
    int i,j;
    for(i = k; i < i_width; i += thread_count){
        for(j = 0; j < i_height; j++){
            result[j][i] = abs(calculate_pixel(j, i));
        }
    }
    gettimeofday(end,NULL);
    timersub(end,start,res);
    free(start);
    free(end);
    return res;
}


int load_image(FILE* file){
    int i,j;
    int max_color;
    if(getc(file) != 'P') return 1;
    else if (getc(file) != '2') return 1;
    while(getc(file) != '\n');

    while(getc(file) == '#'){
        while(getc(file) != '\n');
    }
    fseek(file, -1, SEEK_CUR);

    
    if(fscanf(file, "%d", &i_width) != 1) return 1;
    if(fscanf(file, "%d", &i_height) != 1) return 1;
    if(fscanf(file, "%d", &max_color) != 1) return 1;
    if(max_color > 255) return 1;

    image = malloc(i_height * sizeof(char*));
    result = malloc(i_height * sizeof(char*));

    for(i = 0; i < i_height; i++) {
        image[i] = malloc(i_width * sizeof(int));
        result[i] = malloc(i_width * sizeof(int));
    }

    for(i = i_height - 1; i >= 0; i--){
        for(j = 0 ; j < i_width; j++){
            if(fscanf(file, "%d", &image[i][j]) != 1) return 1; 
        }
    }
    return 0;
}

int load_filter(FILE* file){
    int i,j;
    if(fscanf(file, "%d", &f_size) != 1) return 1;

    filter = malloc(f_size*sizeof(double*));
    for(i = 0; i < f_size; i++) filter[i] = malloc(f_size*sizeof(double));

    for(i = 0; i < f_size; i++){
        for(j = 0; j < f_size; j++){
            if(fscanf(file, "%lf", &filter[i][j]) != 1) return 1;
        }
    }
    return 0;
}

int save_result(FILE* file){
    int i,j;
    fprintf(file,"P2\n%d %d\n%d\n",i_width,i_height,255);
    for(i = i_height - 1; i >= 0; i--){
        for(j = 0 ; j < i_width; j++){
            if(j % 10 == 0) fprintf(file,"\n");
            fprintf(file, "%d ", result[i][j]);
        }
    }
    return 0;
}

void on_close(void){
    int i;
    if(image != NULL){
        for(i = 0; i < i_height; i++) free(image[i]);

        free(image);
    }

    
    if(result != NULL){
        for(i = 0; i < i_height; i++) free(result[i]);

        free(result);
    }
    if(filter != NULL){
        for(i = 0; i < f_size; i++) free(filter[i]);

        free(filter);
    }

}

int main(int argc, char **argv){
    int i;
    if(argc != 6) FAILURE_EXIT(3, "Expected 5 arguments: thread count, calculation mode, input image, filter definition and output image name \n");
    FILE *file = fopen(argv[3],"r");
    if(file == NULL) FAILURE_EXIT(3, "Error during file opening \n");
    char* nullptr = NULL;
    thread_count = strtol(argv[1],&nullptr,10);
    if(!(!*nullptr)) FAILURE_EXIT(3,"Thread count should be a number \n");

    threads = malloc(thread_count*sizeof(int));
    for(i = 0; i < thread_count; i++) threads[i] = i;

    if(load_image(file) != 0) FAILURE_EXIT(3,"Unsupported file format \n");
    fclose(file);
    file = fopen(argv[4],"r");
    if(file == NULL) FAILURE_EXIT(3, "Error during file opening \n");
    if(load_filter(file) != 0) FAILURE_EXIT(3,"Unsupported file format \n");
    fclose(file);
    atexit(on_close);

    pthread_t tids[thread_count];
    if(strcmp(argv[2],"block") == 0){
        for(i = 0; i < thread_count; i++){
            if(pthread_create(&tids[i],NULL,(void*)(void*)thread_calculation_block,&threads[i]) !=0) {FAILURE_EXIT(3,"Error during thread creation \n");}
            else{
                log("Thread %ld started \n",tids[i]);
            }
        }
    }
    else if(strcmp(argv[2],"interleaved") == 0){
        for(i = 0; i < thread_count; i++){
            if(pthread_create(&tids[i],NULL,(void*)(void*)thread_calculation_interleaved,&threads[i]) != 0) {FAILURE_EXIT(3,"Error during thread creation \n");}
            else{
                log("Thread %ld started \n",tids[i]);
            }
        }
    }
    else FAILURE_EXIT(3,"Unsupported calculation mode\n");

    for(i = 0; i < thread_count; i++){
        struct timeval *work_time;
        if(pthread_join(tids[i],(void*)&work_time) != 0) {FAILURE_EXIT(3, "Error on thread join \n");}
        else{
            log("Thread %ld worked %ld.%.6ld seconds \n",tids[i],work_time->tv_sec,work_time->tv_usec);
        }
    }

    file = fopen(argv[5],"w");
    if(file == NULL) FAILURE_EXIT(3, "Error during file opening \n");
    if(save_result(file) != 0) {FAILURE_EXIT(3,"Error on result save \n");}
    else{
        log("File saved successfully \n");
    }
    fclose(file);

    return 0;
}


