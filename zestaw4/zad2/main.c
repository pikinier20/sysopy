#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

const char format[] = "_%Y-%m-%d_%H-%M-%S";

struct Block {
    char * table;
    int size;
    time_t mod;
} Block;

int flag = 1;
int sleep_flag = 1;
pid_t pid[1024];
int count = 0;

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
    if(block->table == NULL) return -1;
    if(fread(block->table, sizeof(char), size, fp) != size*sizeof(char)) return -1;
    fclose(fp);
    return 0;
}

void cp(char * source, char * filename) {
    char dest[100] = "./archiwum/";
    strcat(dest, filename);
    execlp("cp", "cp", source, dest, NULL);
    exit(0);
}

void end(int sig){
    int i;
    for(i = 0; i < count; i++) {
        int status;
        kill(pid[i],SIGINT);
        waitpid(pid[i], &status, 0);
        printf("Proces %d utworzyl %d kopii pliku\n", pid[i], (status/256));
    }
    exit(0);
}


void sigusr1_function(int sig){
    if(sleep_flag == 1){
        sleep_flag = 0;
        printf("Proces potomny %d zostal zatrzymany \n",getpid());
    }
    else{
        printf("Proces potomny %d juz jest zatrzymany \n",getpid());
    }
}

void sigusr2_function(int sig){
    if(sleep_flag == 0){
        sleep_flag = 1;
        printf("Proces potomny %d zostal wznowiony \n",getpid());
    }
    else{
        printf("Proces potomny %d pracuje \n",getpid());
    }
}
void sigint_function(int sig){
    flag = 0;
}

void set_endsignal_handle(){
    struct sigaction action;
    action.sa_handler = sigusr2_function;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGUSR2,&action,NULL);
}

void set_sleepsignal_handle(){
    struct sigaction action;
    action.sa_handler = sigusr1_function;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGUSR1,&action,NULL);
}
void set_intsignal_handle(){
    struct sigaction action;
    action.sa_handler = sigint_function;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT,&action,NULL);
}


int child_process_in_memory(FILE * file, struct stat attr, char * filename, int seconds) {
    set_endsignal_handle();
    set_sleepsignal_handle();
    set_intsignal_handle();
    struct Block *block = create_block();
    if(copy_result_to_memory(block, filename) != 0){
        printf("%d Unable to copy file to memory. \n",getpid());
        exit(0);
    }
    if(stat(filename, &attr) != 0){
        printf("Cannot find file.\n");
        exit(0);
    }
    block->mod = attr.st_mtime;
    int count = 0, j = 0;
    char * tmpfilename = (char *) calloc (100, sizeof(char));
    char * str = (char *) calloc (100, sizeof(char));
    while(flag == 1) {
        sleep(seconds);
        while(sleep_flag != 1){
            if(flag == 0){ 
                remove_block(block);
                free(filename);
                free(str);
                free(tmpfilename);
                fclose(file);
                exit(j);
            }
            sleep(1);
        }
        count++;
        stat(filename, &attr);
        if(attr.st_mtime > block->mod) {
            strcpy(tmpfilename, basename(filename));
            strftime(str, 100, format, localtime(&block->mod));
            strcat(tmpfilename, str);
            if(copy(block, tmpfilename) != 0){
                printf("%d Unable to copy file to disc. \n",getpid());
                exit(0);
            }
            if(copy_result_to_memory(block, filename) != 0){
                printf("%d Unable to copy file to memory. \n",getpid());
                exit(0);
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

int child_process_on_disc(FILE * file, struct stat attr, char * filename, int seconds) {
    set_endsignal_handle();
    set_sleepsignal_handle();
    set_intsignal_handle();
    if(stat(filename, &attr) != 0){
        printf("Cannot find file.\n");
        exit(0);
    }
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

    while(flag == 1) {
        sleep(seconds);
        while(sleep_flag != 1){
            if(flag == 0){
                free(filename);
                fclose(file);
                exit(j);
            }
            sleep(1);
        }
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
    if(argc != 3){ 
        printf("Expected 2 arguments\n");
        return -1;
    }
    FILE * file;
    file = fopen(argv[1], "r");
    if(file == NULL){ 
        printf("Error while opening file\n");
        return -1;
    }

    char * filename = (char *) calloc (100, sizeof(char));
    int seconds;

    int i = 0;
    struct stat attr;



    while(fscanf(file,"%s %d", filename, &seconds) == 2) {
        if(strcmp(argv[2], "in_memory") == 0) {
            pid[count] = fork();
            if(pid[count] == 0 ) {
                child_process_in_memory(file, attr, filename, seconds);
            }
        }
        else if(strcmp(argv[2], "on_disc") == 0) {
            pid[count] = fork();
            if(pid[count] == 0 ) {
                child_process_on_disc(file, attr, filename, seconds);
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

    free(filename);
    fclose(file);

    struct sigaction action;
    action.sa_handler = end;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT,&action,NULL);
    char command[255];
    char helper[255];
    int pid_no = 0;
    int j;

    while(flag == 1){
        for(i = 0; i < 255; i++) command[i] = '\0';
        if(scanf("%s",command) != 1) continue;
        if(strcmp(command,"LIST") == 0){
            for(i = 0; i < count ; i++){
                printf("Proces potomny PID: %d \n",pid[i]);
            }
        }
        else if(strcmp(command,"END") == 0){
            raise(SIGINT);
        }
        else if(strcmp(command,"STOP") == 0){
            if(scanf("%s",helper) != 1) raise(SIGINT);
            else{
                if(strcmp(helper, "ALL") == 0){
                    for(j = 0; j < count ; j++){
                        kill(pid[j],SIGUSR1);
                    }
                }
                else if(sscanf(helper,"%d",&pid_no)==1){
                    for(j = 0; j < count ; j++){
                        if(pid[j] == pid_no) kill(pid[j],SIGUSR1);
                    }
                }
                else continue;
            }
        }
        else if(strcmp(command,"START") == 0){
            if(scanf("%s",helper) != 1) raise(SIGINT);
            else{
                if(strcmp(helper, "ALL") == 0){
                    for(j = 0; j < count ; j++){
                        kill(pid[j],SIGUSR2);
                    }
                }
                else if(sscanf(helper,"%d",&pid_no)==1){
                    for(j = 0; j < count ; j++){
                        if(pid[j] == pid_no) kill(pid[j],SIGUSR2);
                    }
                }
                else continue;
            }
        }
        else continue;
    }



    return 0;
}