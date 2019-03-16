#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/times.h>
#include <string.h>
#include <zconf.h>
#include <errno.h>
#include "file_sys.h"
#include "file_lib.h"

double calculate_time(clock_t start,clock_t end){
    return (double) (end-start) / sysconf(_SC_CLK_TCK);
}

int generate(char* path, int count, int size){
    FILE* file = fopen(path,"w+");
    FILE* random = fopen("/dev/urandom","r");
    char* tmp = malloc((size+1)*sizeof(char));
    int i,j;
    for(i=0; i < count; i++){
        if(fread(tmp,sizeof(char),size+1,random) != size+1) return 1;
        for(j=0; j < size; j++) tmp[j] = abs(tmp[j] % 26) + 97;
        tmp[size] = '\n';
        if(fwrite(tmp,sizeof(char),size+1,file) != size+1) return 1;
    }
    free(tmp);
    fclose(file);
    fclose(random);
    return 0;
}
int main(int argc, char** argv){
    struct tms *real_start_time = malloc(sizeof(struct tms));
    struct tms *real_end_time = malloc(sizeof(struct tms));
    char* endptr1 = NULL;
    char* endptr2 = NULL;
    char* file1;
    char* file2;
    int size;
    int count;
    int result;
    errno = 0;
    times(real_start_time);
    if(strcmp(argv[1],"generate") == 0){
        if(argc < 5){
            printf("Too few arguments to execute: generate.\n");
        } else{
            file1 = argv[2];
            errno = 0;
            count = strtol(argv[3],&endptr1,10);
            size = strtol(argv[4],&endptr2,10);
            if(errno == 0 && !*endptr1 && !*endptr2){
                result = generate(argv[2],count,size);
                printf("generate %d lines of size %d \n",count,size);

            } else{
                printf("Wrong type of size or count - expected integer. \n");
            }
        }
    } else if(strcmp(argv[1],"sort") == 0){
        if(argc < 6){
            printf("Too few arguments to execute: generate.\n");
        } else{
            file1 = argv[2];
            errno = 0;
            count = (int) strtol(argv[3], &endptr1,10);
            size = (int) strtol(argv[4], &endptr2,10);
            if(errno == 0 && !*endptr1 && !*endptr2){
                if(strcmp(argv[5],"sys") == 0){
                    result = sys_sort(file1, count, size);
                    printf("sorting %d lines of size %d using syscalls \n",count,size);
                }
                else if(strcmp(argv[5],"lib") == 0){
                    result = lib_sort(file1, count, size);
                    printf("sorting %d lines of size %d using standard library \n",count,size);
                } else{
                    printf("Last argument should be one of these: lib, sys \n");
                    return 1;
                }
            }
        }
    } else if(strcmp(argv[1],"copy") == 0){
        if(argc < 7){
            printf("Too few arguments to execute: generate.\n");
            return -1;
        } else{
            file1 = argv[2];
            file2 = argv[3];
            errno = 0;
            count = strtol(argv[4],&endptr1,10);
            size = strtol(argv[5],&endptr2,10);
            if(errno == 0 && !*endptr1 && !*endptr2){
                if(strcmp(argv[6],"sys") == 0){
                    result = sys_copy(file1, file2, count, size);
                    printf("copying file %s into %s. %d lines of size %d using syscalls \n",file1,file2,count,size);
                }
                else if(strcmp(argv[6],"lib") == 0){
                    result = lib_copy(file1, file2, count, size);
                    printf("copying file %s into %s. %d lines of size %d using standard library \n",file1,file2,count,size);
                } else{
                    printf("Last argument should be one of these: lib, sys \n");
                    return 1;
                }
            }
        }
    }
    if(result == 1){
        printf("Error occured during file opening\n");
    } else if(result == -1){
        printf("Error occured while reading a file\n");
    }
    times(real_end_time);
    printf("System time: %lf \n", calculate_time(real_start_time->tms_cstime,real_end_time->tms_cstime));
    printf("User time: %lf \n", calculate_time(real_start_time->tms_cutime,real_end_time->tms_cutime));
    free(real_end_time);
    free(real_start_time);
    return 0;
}
