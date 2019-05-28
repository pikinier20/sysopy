#define log(format, ...) {printf("\033[1;31m%ld.%.6ld: \033[0m",getTime().tv_sec,getTime().tv_usec); printf(format, ##__VA_ARGS__);}
#define FAILURE_EXIT(code, format, ...) { log(format, ##__VA_ARGS__); exit(code);}
#define _XOPEN_SOURCE 500
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

typedef struct Car{
    int order_number;
    int current_people_number;
    int car_status;
    int run_number;
    int tries_count;
    pthread_mutex_t access;
    pthread_cond_t status;
    pthread_cond_t status_change;
}Car;

struct timeval getTime();

Car *car_init(int order_number);

void car_destroy(Car *car);

void *car_function(void *args);

void *person_function(void *args);

void free_res(void);

int people_number,car_number,car_capacity,run_number,working_cars;
int current_order = 0;
int finish_order = 0;
Car *current_car = NULL;
Car **cars;
pthread_t **car_threads;
pthread_t **people_threads;
int *people_numbers;
pthread_cond_t *car_order_condition;
pthread_cond_t *car_present_condition;
pthread_cond_t *car_finish_order_cond;
pthread_mutex_t *car_access_mutex;
pthread_mutex_t *car_stop_access_mutex;
pthread_mutex_t *car_finish_mutex;

int main(int argc, char **argv){
    char *nullptr = NULL;
    int i;
    if(argc != 5) FAILURE_EXIT(3, "Expected people number, car number, car capacity and number of runs \n");
    people_number = strtol(argv[1],&nullptr,10);
    car_number = strtol(argv[2],&nullptr,10);
    car_capacity = strtol(argv[3],&nullptr,10);
    run_number = strtol(argv[4],&nullptr,10);
    working_cars = car_number;

    if(!(!*nullptr)) FAILURE_EXIT(3, "Arguments parse error \n");

    car_order_condition = malloc(sizeof(pthread_cond_t));
    car_present_condition = malloc(sizeof(pthread_cond_t));
    car_finish_order_cond = malloc(sizeof(pthread_cond_t));
    car_access_mutex = malloc(sizeof(pthread_mutex_t));
    car_stop_access_mutex = malloc(sizeof(pthread_mutex_t));
    car_finish_mutex = malloc(sizeof(pthread_mutex_t));

    if(pthread_cond_init(car_order_condition,NULL) != 0) FAILURE_EXIT(3,"Condition variables initialization error \n");
    if(pthread_cond_init(car_present_condition,NULL) != 0) FAILURE_EXIT(3,"Condition variables initialization error \n");
    if(pthread_cond_init(car_finish_order_cond,NULL) != 0) FAILURE_EXIT(3,"Condition variables initialization error \n");
    if(pthread_mutex_init(car_access_mutex,NULL) != 0) FAILURE_EXIT(3,"Mutex initialization error \n");
    if(pthread_mutex_init(car_stop_access_mutex,NULL) != 0) FAILURE_EXIT(3,"Mutex initialization error \n");
    if(pthread_mutex_init(car_finish_mutex,NULL) != 0) FAILURE_EXIT(3,"Mutex initialization error \n");


    cars = malloc(car_number*sizeof(Car*));
    car_threads = malloc(car_number*sizeof(pthread_t*));
    people_threads = malloc(people_number*sizeof(pthread_t*));
    people_numbers = malloc(people_number*sizeof(int));

    atexit(free_res);
    for(i = 0; i < car_number; i++){
        cars[i] = car_init(i);
        car_threads[i] = malloc(sizeof(pthread_t));
        pthread_create(car_threads[i],NULL,car_function,(void*)cars[i]);
    }
    for(i = 0; i < people_number; i++){
        people_numbers[i] = i;
        people_threads[i] = malloc(sizeof(pthread_t));
        pthread_create(people_threads[i],NULL,person_function,(void*)(&people_numbers[i]));
    }

    for(i = 0; i < car_number; i++){
        pthread_join(*car_threads[i],NULL);
    }

    for(i = 0; i < people_number; i++){
        pthread_join(*people_threads[i],NULL);
    }
}

Car *car_init(int order_number){
    Car *car = malloc(sizeof(Car));
    car->order_number = order_number;
    car->current_people_number = 0;
    car->car_status = 0;
    car->run_number = run_number;
    pthread_mutex_init(&car->access,NULL);
    pthread_cond_init(&car->status,NULL);
    pthread_cond_init(&car->status_change,NULL);
    return car;
}

void car_destroy(Car *car){
    pthread_cond_destroy(&car->status);
    pthread_cond_destroy(&car->status_change);
    pthread_mutex_destroy(&car->access);
}

void *car_function(void *args){
    Car *own = (Car*) args;
    while(1 == 1){
        pthread_mutex_lock(car_stop_access_mutex);
        while(current_order != own->order_number){
            pthread_cond_wait(car_order_condition,car_stop_access_mutex);
        }
        pthread_mutex_lock(car_access_mutex);
        current_car = own;
        pthread_mutex_lock(&own->access);
        log("Car %d arrived to car stop \n",own->order_number);
        own->car_status = 3;
        pthread_cond_broadcast(&own->status);
        pthread_mutex_unlock(&own->access);

        pthread_mutex_lock(&own->access);
        while(own->current_people_number != 0){
            pthread_cond_wait(&own->status_change,&own->access);
        }
        own->run_number--;
        if(own->run_number < 0) {
            break;
        }
        own->car_status = 0;
        pthread_cond_broadcast(car_present_condition);
        pthread_mutex_unlock(&own->access);
        pthread_mutex_unlock(car_access_mutex);


        pthread_mutex_lock(&own->access);
        while(own->current_people_number != car_capacity){
            pthread_cond_wait(&own->status_change,&own->access);
        }
        own->tries_count = rand() % car_capacity;
        own->car_status = 1;
        pthread_cond_broadcast(&own->status);
        pthread_mutex_unlock(&own->access);

        pthread_mutex_lock(&own->access);
        while(own->car_status != 2){
            pthread_cond_wait(&own->status_change,&own->access);
        }
        log("Car %d starts run \n",own->order_number);
        current_order = (current_order + 1) % car_number;
        pthread_cond_broadcast(car_order_condition);
        pthread_mutex_unlock(car_stop_access_mutex);
        usleep(2000);
        pthread_mutex_unlock(&own->access);

        pthread_mutex_lock(car_finish_mutex);
        while(finish_order != own->order_number){
            pthread_cond_wait(car_finish_order_cond,car_finish_mutex);
        }
        log("Car %d finishes run \n",own->order_number);
        finish_order = (finish_order + 1) % car_number;
        pthread_cond_broadcast(car_finish_order_cond);
        pthread_mutex_unlock(car_finish_mutex);
    }
    log("Car %d finishes work \n",own->order_number);
    current_order = (current_order + 1) % car_number;
    pthread_cond_broadcast(car_order_condition);
    pthread_mutex_unlock(car_stop_access_mutex);
    working_cars--;
    pthread_mutex_unlock(car_access_mutex);
    if(working_cars == 0) {
        pthread_cond_broadcast(car_present_condition);
    }
    int ret = 0;
    pthread_exit((void*)&ret);
}

void *person_function(void *args){
    int i = (int) ((int*)args)[0];
    Car *my_car;
    srand(time(NULL));
    while(working_cars != 0){
        pthread_mutex_lock(car_access_mutex);
        while(working_cars != 0 && (current_car == NULL || current_car->car_status != 0 || current_car->current_people_number == car_capacity)){
            pthread_cond_wait(car_present_condition,car_access_mutex);
        }
        if(working_cars == 0) {
            pthread_mutex_unlock(car_access_mutex);
            break;
        }
        my_car = current_car;
        pthread_mutex_lock(&my_car->access);
        while(current_car->car_status != 0 || current_car->current_people_number == car_capacity){
            pthread_cond_wait(car_present_condition,&my_car->access);
        }

        my_car->current_people_number++;
        log("Person %d gets in car %d. Currently %d people in car \n",i,my_car->order_number,my_car->current_people_number);

        if(my_car-> current_people_number == car_capacity){
            pthread_cond_broadcast(&my_car->status_change);
        }
        pthread_mutex_unlock(&my_car->access);

        pthread_mutex_unlock(car_access_mutex);

        pthread_mutex_lock(&my_car->access);
        while(my_car->car_status == 0){
            pthread_cond_wait(&my_car->status,&my_car->access);
        }
        if(current_car->car_status == 1 && !(&my_car->tries_count--)){
            my_car->car_status = 2;
            log("Person %d clicked start button in car %d \n",i,my_car->order_number);
            pthread_cond_broadcast(&my_car->status_change);
        }
        pthread_mutex_unlock(&my_car->access);

        pthread_mutex_lock(&my_car->access);
        while(my_car->car_status <= 2){
            pthread_cond_wait(&my_car->status,&my_car->access);
        }
        my_car->current_people_number--;
        log("Person %d gets off car %d. Currently %d people in car\n",i,my_car->order_number,my_car->current_people_number);
        pthread_cond_broadcast(&my_car->status_change);
        pthread_mutex_unlock(&my_car->access);
    }
    log("Person %d finishes work \n",i);
    int ret = 0;
    pthread_exit((void*)&ret);
}

struct timeval getTime(){
    struct timeval timebuffer;
    gettimeofday(&timebuffer, NULL);
    return timebuffer;
}

void free_res(){
    int i;
    for(i = 0; i < car_number; i++){
        car_destroy(cars[i]);
        free(cars[i]);
        free(car_threads[i]);
    }
    for(i = 0; i < people_number; i++){
        free(people_threads[i]);
    }
    free(cars);
    free(car_threads);
    free(people_threads);
    free(people_numbers);

    if(pthread_cond_destroy(car_order_condition) != 0) FAILURE_EXIT(3,"Condition variables initialization error \n");
    if(pthread_cond_destroy(car_present_condition) != 0) FAILURE_EXIT(3,"Condition variables initialization error \n");
    if(pthread_cond_destroy(car_finish_order_cond) != 0) FAILURE_EXIT(3,"Condition variables initialization error \n");
    if(pthread_mutex_destroy(car_access_mutex) != 0) FAILURE_EXIT(3,"Mutex initialization error \n");
    if(pthread_mutex_destroy(car_stop_access_mutex) != 0) FAILURE_EXIT(3,"Mutex initialization error \n");
    if(pthread_mutex_destroy(car_finish_mutex) != 0) FAILURE_EXIT(3,"Mutex initialization error \n");
    
    free(car_order_condition);
    free(car_present_condition);
    free(car_finish_order_cond);
    free(car_access_mutex);
    free(car_stop_access_mutex);
    free(car_finish_mutex);
}
