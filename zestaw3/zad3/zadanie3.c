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
#define _XOPEN_SOURCE_EXTENDED 1
#include <sys/resource.h>

const char format[] = "_%Y-%m-%d_%H-%M-%S";

struct Block {
    char * table;
    int size;
    time_t mod;
} Block;

struct WrappedBlock {
    struct Block * block;
    int size;
} WrappedBlock;



void copy(struct WrappedBlock * wp, char * destination, int i) {
    FILE * fd_d;
    char dest[100] = "./archiwum/";
    strcat(dest, destination);
    fd_d = fopen(dest, "w+");
    if(fd_d < 0) {
        printf("Error during file creation");
        return;
    } else {
        fwrite(wp->block[i].table, sizeof(char), wp->block[i].size, fd_d);
    }
    fclose(fd_d);
}

struct WrappedBlock *create(int size) {
    struct WrappedBlock *wp;
    wp = (struct WrappedBlock*) calloc (1, sizeof(struct WrappedBlock));
    wp->block = (struct Block*) calloc (size, sizeof(struct Block));
    wp->size = size;
    return wp;
}

void remove_block(struct WrappedBlock *wp, int index) {
    if(wp->size < index || index < 0) {
        printf("Cannot remove membory block on given index");
        return;
    }
    free(wp->block[index].table);}

int copy_result_to_memory(struct WrappedBlock *wp, char * filename, int i) {
    FILE *fp;
    int size;
    if(wp == NULL || wp->block == NULL) return -1;
    fp = fopen(filename, "r");
    if(fp == NULL) return -1;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if(wp->block[i].table != NULL) remove_block(wp, i);
    wp->block[i].table = (char*) calloc (size, sizeof(char));
    wp->block[i].size = size;
    fread(wp->block[i].table, sizeof(char), size, fp);
    fclose(fp);
    return 0;
}

void free_memory(struct WrappedBlock *wp) {
    for(int i = 0; i < wp->size; i++) {
        free(wp->block[i].table);
    }
    free(wp->block);
    
    free(wp);
}

void cp(char * source, char * filename) {
    char dest[100] = "./archiwum/";
    strcat(dest, filename);
    execlp("cp", "cp", source, dest, NULL);
    exit(0);
}

int child_process_in_memory(FILE * file, struct WrappedBlock * wp, struct stat attr, char * filename, int seconds, int terminate_seconds, int i) {
    copy_result_to_memory(wp, filename, i);
    stat(filename, &attr);
    wp->block[i].mod = attr.st_mtime;
    int count = 0, j = 0;
    char * tmpfilename = (char *) calloc (100, sizeof(char));
    char * str = (char *) calloc (100, sizeof(char));
    while(count*seconds < terminate_seconds && (count+1)*seconds < terminate_seconds) {
        sleep(seconds);
        count++;
        stat(filename, &attr);
        if(attr.st_mtime > wp->block[i].mod) {
            strcpy(tmpfilename, basename(filename));
            strftime(str, 100, format, localtime(&wp->block[i].mod));
            strcat(tmpfilename, str);
            copy(wp, tmpfilename, i);
            copy_result_to_memory(wp, filename, i);
            wp->block[i].mod = attr.st_mtime;
            j++;
        }
    }
    free_memory(wp);
    free(filename);
    free(str);
    free(tmpfilename);
    fclose(file);
    exit(j);
}

int child_process_on_disc(FILE * file, struct WrappedBlock * wp, struct stat attr, char * filename, int seconds, int terminate_seconds, int i) {
    
    stat(filename, &attr);
    time_t mod = attr.st_mtime;
    int count = 0, j = 1;
    char tmpfilename[100];
    char str[100];

    strcpy(tmpfilename, basename(filename));
    strftime(str, 100, format, localtime(&mod));
    strcat(tmpfilename, str);

    pid_t pid = fork();
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
    free_memory(wp);
    free(filename);
    fclose(file);
    exit(j);
}

int main(int argc, char ** argv) {
    FILE * file;
    file = fopen(argv[1], "r");
    if(file == NULL) return -1;
    if(argc != 6) return -1;

    char * filename = (char *) calloc (100, sizeof(char));
    int terminate_seconds;
    int time_limit;
    int size_limit;
    sscanf(argv[2], "%d", &terminate_seconds);
    sscanf(argv[4], "%d", &time_limit);
    sscanf(argv[5], "%d", &size_limit);
    int seconds;
    char c;
    int count = 0;
    struct WrappedBlock * wp;
    int i = 0;
    struct stat attr;

    for (c = getc(file); c != EOF; c = getc(file)) 
        if (c == '\n')
            count = count + 1; 
    fseek(file, 0, SEEK_SET);

    wp = create(count);
    pid_t pid[count];
    

    struct rusage prev_usage;
    getrusage(RUSAGE_CHILDREN, &prev_usage);

    while(fscanf(file,"%s %d", filename, &seconds)>0) {
        pid[i] = fork();
        if(pid[i] == 0 ) {
            struct rlimit r;
            r.rlim_max = (rlim_t) time_limit;
            r.rlim_cur = (rlim_t) time_limit;
            if (setrlimit(RLIMIT_CPU, &r) != 0) {
                printf("I cannot set this limit cpu ");
                return 1;
            }

            r.rlim_max = (rlim_t) size_limit;
            r.rlim_cur = (rlim_t) size_limit;

            if (setrlimit(RLIMIT_AS, &r) != 0) {
                printf("I cannot set this limit cpu ");
                return 1;
            }
            if(strcmp(argv[3], "in_memory") == 0) {
                child_process_in_memory(file, wp, attr, filename, seconds, terminate_seconds, i);
            } else if(strcmp(argv[3], "on_disc") == 0) {
                child_process_on_disc(file, wp, attr, filename, seconds, terminate_seconds, i);
            } else {
                printf("Unsupported operation. Closing program!");
            }

        }
        i++;
    }
    sleep(terminate_seconds);
    struct rusage usage;
    
    for(int i = 0; i < count; i++) {
        int status;
        waitpid(pid[i], &status, 0);
        
        printf("Proces %d utworzyl %d kopii pliku\n", pid[i], (status/256));
    }
    struct timeval ru_utime;
    struct timeval ru_stime;
    getrusage(RUSAGE_CHILDREN, &usage);
    timersub(&usage.ru_utime, &prev_usage.ru_utime, &ru_utime);
    timersub(&usage.ru_stime, &prev_usage.ru_stime, &ru_stime);
    printf("Czas systemowy %d.%d Czas uzytkownika %d.%d Zuzycie pamieci\n", (int) ru_stime.tv_sec, (int) ru_stime.tv_usec, (int) ru_utime.tv_sec, (int) ru_utime.tv_usec);
    free_memory(wp);
    free(filename); 

    
    fclose(file);
    return 0;
}