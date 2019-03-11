#include <dlfcn.h>

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/times.h>
#include <string.h>
#include <zconf.h>
#include <errno.h>

void *dl_handle; //Dynamic library handle

struct wrapped_array {
    int size;
    char **array;
}; // No dynamic loading of struct definition in C
typedef struct wrapped_array wrapped_array;


double calculate_time(clock_t start,clock_t end){
    return (double) (end-start) / sysconf(_SC_CLK_TCK);
}

int find_empty_index(wrapped_array* array){
    if(array == NULL) return -1;
    int i = 0;
    while(i < array->size){
        if(array->array[i] == NULL) return i;
        i++;
    }
    return -1;
}

int main(int argc, char **argv){
    printf("Using dynamic library\n");
    dl_handle = dlopen("./sharedlib.so", RTLD_LAZY);
    if(dl_handle == NULL){
        printf("Cannot find library mylib.so. Terminating...");
        return -1;
    }

    struct wrapped_array* (*dlcreate)();
    char* (*dlfind_file)();
    int (*dldelete_block_at_index)();
    int (*dladd_block_at_index)();
    void (*dlfree_memory)();
    dlcreate = dlsym(dl_handle,"create"); //*(struct wrapped_array**) &
    dlfind_file = dlsym(dl_handle,"find_file");
    dldelete_block_at_index = dlsym(dl_handle,"delete_block_at_index");
    dladd_block_at_index = dlsym(dl_handle,"add_block_at_index"); 
    dlfree_memory = dlsym(dl_handle, "free_memory");

    
    wrapped_array* base_array = NULL;
    char* temporary_file_name;
    char *endptr = NULL; // Used to strtol function call
    struct tms *global_start_time = malloc(sizeof(struct tms));
    struct tms *global_end_time = malloc(sizeof(struct tms));
    struct tms *start_time = malloc(sizeof(struct tms));
    struct tms *end_time = malloc(sizeof(struct tms));
    clock_t real_start_time;
    clock_t real_end_time;
    clock_t global_real_start_time;
    clock_t global_real_end_time;
    global_real_start_time = times(global_start_time);
    int inc = 1;
    printf("\t\tReal\t\tUser\t\tSystem\n");
    while(inc < argc){
        real_start_time = times(start_time);
        if(strcmp(argv[inc],"create_table") == 0 && inc+1 < argc){
            errno = 0;
            endptr = NULL;
            long table_size = strtol(argv[inc+1],&endptr,10);
            if(errno == 0 && !*endptr){ // If errno is 0 and endptr is still set to NULL then number is valid

                if(base_array != NULL) free(base_array);
                base_array = dlcreate(table_size);
                if(base_array == NULL){
                    printf("Provided size is less or equal 0\n");
                    printf("create\t");
                }
                else{
                    printf("create\t");
                    //printf("Table created successfully.\n");
                }
                inc+=2;
            }
            else{
                printf("Bad argument to create_table, expected table_size (integer)\n");
                return -1;
            }
        }
        else if(strcmp(argv[inc],"search_directory") == 0){

            temporary_file_name = dlfind_file(argv[inc+1],argv[inc+2],argv[inc+3]);
            if(temporary_file_name != NULL){
                //printf("Find command completed. Results saved to file. Any error saved to error.log\n");
                printf("find\t");
            }
            else{
                printf("Error during command execution. Bad arguments. \n");
                return -1;
            }
            inc+=4;
        }
        else if(strcmp(argv[inc], "remove_block") == 0 && inc+1 < argc){
            errno = 0;
            endptr = NULL;
            long index = strtol(argv[inc+1],&endptr,10);
            if(errno == 0 && !*endptr){

                int result;
                result = dldelete_block_at_index(base_array, (int) index);
                if(result == 0){
                    //printf("Deleted block no. %d .  \n",index);
                    printf("delete\t");
                }
                else{
                    printf("Error during deleting block. Index out of range or NaN or table doesn't exist \n");
                }
                inc+=2;
            }
            else{
                printf("Index argument is not a number \n");
                return -1;
            }
        }
        else if(strcmp(argv[inc], "save_search") == 0 && inc+1 < argc){
            int index = find_empty_index(base_array);
            if(index != -1){

                int result = dladd_block_at_index(base_array,argv[inc+1],index);
                if(result != -1){
                    //printf("File saved at index no. %d \n",result);
                    printf("save\t");
                }
                else{
                    printf("Error during saving file. Probably file doesn't exist \n");
                    return -1;
                }
            }
            else{
                printf("No place in array or array doesn't exist\n");
            }
            inc+=2;
        }
        else{
            printf("Argument no. %d is not a valid command. Terminating... \n",inc);
            inc = argc;
        }
        real_end_time = times(end_time);
        //printf("Real\t\tUser\t\tSystem\n");
        printf("%lf\t",calculate_time(real_start_time,real_end_time));
        printf("%lf\t",calculate_time(start_time->tms_cutime,end_time->tms_cutime));
        printf("%lf\n",calculate_time(start_time->tms_cstime,end_time->tms_cstime));
    }
    global_real_end_time = times(global_end_time);
 //   printf("Global times\n");
//    printf("Real\t\tUser\t\tSystem\n");
    printf("global\t");
    printf("%lf\t",calculate_time(global_real_start_time,global_real_end_time));
    printf("%lf\t",calculate_time(global_start_time->tms_cutime,global_end_time->tms_cutime));
    printf("%lf\n\n\n",calculate_time(global_start_time->tms_cstime,global_end_time->tms_cstime));
    dlfree_memory(base_array);
    dlclose(dl_handle);
    free(global_start_time);
    free(global_end_time);
    free(start_time);
    free(end_time);
    return 0;
}