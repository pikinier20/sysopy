
#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <libgen.h>
#include <time.h>
#include <errno.h>
#include <sys/resource.h>

const char format[] = "_%Y-%m-%d_%H-%M-%S";

struct Block {
    char * table;
    int size;
    time_t mod;
} Block;

double calculate_time(clock_t start,clock_t end){
    return (double) (end-start);
}

int handle_limit(int time_limit, int size_limit){
    struct rlimit r;
    r.rlim_max = (rlim_t) time_limit;
    r.rlim_cur = (rlim_t) time_limit;
    if (setrlimit(RLIMIT_CPU, &r) != 0) {
        printf("I cannot set this time limit. \n");
        return 1;
    }

    r.rlim_max = (rlim_t) size_limit;
    r.rlim_cur = (rlim_t) size_limit;
    if (setrlimit(RLIMIT_AS, &r) != 0) {
        printf("I cannot set this size limit. \n");
        return 1;
    }
    return 0;
}

int copy(struct Block * block, char * destination) {
    FILE * fd_d;
    char dest[100] = "./archiwum/";
    strcat(dest, destination);
    fd_d = fopen(dest, "w+");
    if(fd_d == NULL) {
        printf("Error during file creation \n");
        return -1;
    } else {
        if(fwrite(block->table, sizeof(char), block->size, fd_d) != block->size * sizeof(char)) return -1;
    }
    fclose(fd_d);
    return 0;
}
struct Block *create_block(){
    struct Block *result = (struct Block*) calloc(1,sizeof(struct Block));
    return result;
}
void remove_block(struct Block *block){
    if(block == NULL) return;
    if(block->table != NULL) free(block->table);
    free(block);
    block = NULL;
}

int copy_result_to_memory(struct Block *block, char * filename) {
    FILE *fp;
    int size;
    if(block == NULL) return -1;
    fp = fopen(filename, "r");
    if(fp == NULL) return -1;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if(block->table != NULL) free(block->table);
    block->table = (char*) calloc (size, sizeof(char));
    block->size = size;
    fread(block->table, sizeof(char), size, fp);
    fclose(fp);
    return 0;
}

void cp(char * source, char * filename) {
    char dest[100] = "./archiwum/";
    strcat(dest, filename);
    execlp("cp", "cp", source, dest, NULL);
    exit(0);
}

int child_process_in_memory(FILE * file, struct stat attr, char * filename, int seconds, int terminate_seconds) {
    struct Block *block = create_block();
    if(copy_result_to_memory(block, filename) != 0){
        printf("%d Unable to copy file to memory. \n",getpid());
    }
    stat(filename, &attr);
    block->mod = attr.st_mtime;
    int count = 0, j = 0;
    char * tmpfilename = (char *) calloc (100, sizeof(char));
    char * str = (char *) calloc (100, sizeof(char));
    while(count*seconds < terminate_seconds && (count+1)*seconds < terminate_seconds) {
        sleep(seconds);
        count++;
        stat(filename, &attr);
        if(attr.st_mtime > block->mod) {
            strcpy(tmpfilename, basename(filename));
            strftime(str, 100, format, localtime(&block->mod));
            strcat(tmpfilename, str);
            if(copy(block, tmpfilename) != 0){
                printf("%d Unable to copy file to disc. \n",getpid());
            }
            if(copy_result_to_memory(block, filename) != 0){
                printf("%d Unable to copy file to memory. \n",getpid());
            }
            block->mod = attr.st_mtime;
            j++;
        }
    }
    remove_block(block);
    free(filename);
    free(str);
    free(tmpfilename);
    fclose(file);
    exit(j);
}

int child_process_on_disc(FILE * file, struct stat attr, char * filename, int seconds, int terminate_seconds) {
    
    stat(filename, &attr);
    time_t mod = attr.st_mtime;
    int count = 0, j = 1;
    char tmpfilename[100];
    char str[100];

    strcpy(tmpfilename, basename(filename));
    strftime(str, 100, format, localtime(&mod));
    strcat(tmpfilename, str);

    pid_t pid = vfork();
    if(pid == 0) {
        cp(filename, tmpfilename);
    }

    while(count*seconds < terminate_seconds && (count+1)*seconds < terminate_seconds) {
        sleep(seconds);
        count++;
        stat(filename, &attr);
        if(attr.st_mtime > mod) {
            mod = attr.st_mtime;
            strcpy(tmpfilename, basename(filename));
            strftime(str, 100, format, localtime(&mod));
            strcat(tmpfilename, str);
            pid = fork();
            if(pid == 0) {
                cp(filename, tmpfilename);
            }
            j++;
        }
    }
    free(filename);
    fclose(file);
    exit(j);
}

int main(int argc, char ** argv) {
    if(argc != 6){ 
        printf("Expected 6 arguments\n");
        return -1;
    }
    FILE * file;
    file = fopen(argv[1], "r");
    if(file == NULL){ 
        printf("Error while opening file\n");
        return -1;
    }
    char * filename = (char *) calloc (100, sizeof(char));
    int terminate_seconds;
    int time_limit;
    int size_limit;
    if(sscanf(argv[2], "%d", &terminate_seconds) != 1){
        printf("Second parameter should be integer\n");
        return -1;
    }
    if(sscanf(argv[4], "%d", &time_limit) != 1){
        printf("Fourth parameter should be integer\n");
        return -1;
    }
    if(sscanf(argv[5], "%d", &size_limit) != 1){
        printf("Fifth parameter should be integer\n");
        return -1;
    }
    size_limit = size_limit * 1024 * 1024;
    int seconds;
    int count = 0;
    int i = 0;
    struct stat attr;

    pid_t pid[1024];
    struct rusage prev_usage;
    struct rusage current_usage;

    struct timeval ru_utime;
    struct timeval ru_stime;
    getrusage(RUSAGE_CHILDREN, &prev_usage);
    

    while(fscanf(file,"%s %d", filename, &seconds) == 2) {
        if(strcmp(argv[3], "in_memory") == 0) {
            pid[count] = fork();
            if(pid[count] == 0 ) {
                if(handle_limit(time_limit,size_limit) != 0){ 
                    free(filename); 
                    fclose(file);
                    return -1;
                }

                child_process_in_memory(file, attr, filename, seconds, terminate_seconds);
            }
        }
        else if(strcmp(argv[3], "on_disc") == 0) {
            pid[count] = fork();
            if(pid[count] == 0 ) {
                if(handle_limit(time_limit,size_limit) != 0) { 
                    free(filename); 
                    fclose(file);
                    return -1;
                }

                child_process_on_disc(file, attr, filename, seconds, terminate_seconds);
            }
        }
        else {
            printf("Unsupported operation. Closing program!");
            free(filename); 
            fclose(file);
            return -1;
            }
        count = count+1;
    }
    for(i = 0; i < count; i++) {
        int status;
        waitpid(pid[i], &status, 0);
        getrusage(RUSAGE_CHILDREN,&current_usage);
        timersub(&current_usage.ru_stime,&prev_usage.ru_stime,&ru_stime);
        timersub(&current_usage.ru_utime,&prev_usage.ru_utime,&ru_utime);
        printf("Process %d created %d copies of file\n", pid[i], (status/256));
        printf("System time: ");
        printf("%d.%06d ",(int)ru_stime.tv_sec,(int)ru_stime.tv_usec);
        printf("User time: ");
        printf("%d.%06d \n",(int)ru_utime.tv_sec,(int)ru_utime.tv_usec);
        prev_usage = current_usage;
    }
    free(filename); 
    fclose(file);
    return 0;
}