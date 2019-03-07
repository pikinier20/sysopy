#include "mylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

struct wrapped_array* create(int array_size){
    if(array_size < 0) return NULL;

    struct wrapped_array *res = malloc(sizeof(struct wrapped_array));
    res->size = array_size;
    char** arr = (char**) calloc(array_size,sizeof(char*));
    res->array = arr;
    
    return res;
}

int add_block_at_index(struct wrapped_array* arr, char* block, int index){
    if(arr == NULL || index >= arr->size || index < 0) return -1;

    arr->array[index] = (char*) calloc(strlen(block), sizeof(char));
    strcpy(arr->array[index], block);

    return index;
}

void delete_block_at_index(struct wrapped_array* arr,int index){
    if(arr == NULL || arr->array[index] == NULL || index < 0 || index >= arr->size ) return;

    free(arr->array[index]);
    arr->array[index]=NULL;
}

char* find_file(char* dir, char* file, char* name_file_temp){
    if(dir == NULL || file == NULL || name_file_temp == NULL) return NULL;
    int state = system(strcat("find ",strcat(dir,strcat(" ",strcat(file,strcat(" > ",name_file_temp))))));

    if(state != 0) return NULL;

    return name_file_temp;
}



