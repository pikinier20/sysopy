#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <ftw.h>
#include "lib.h"
char op;
time_t time_from_epoch;

char* get_file_type(int st_mode){
    if(S_ISDIR(st_mode) != 0) return "dir";
    if(S_ISCHR(st_mode) != 0) return "char dev";
    if(S_ISBLK(st_mode) != 0) return "block dev";
    if(S_ISFIFO(st_mode) != 0) return "fifo";
    if(S_ISLNK(st_mode) != 0) return "slink";
    if(S_ISSOCK(st_mode) != 0) return "sock";
    if(S_ISREG(st_mode) != 0) return "file";
    return "error";
}

int get_date_string(time_t time, char *buffer){
    struct tm *times = localtime(&time);
    strftime(buffer, 256*sizeof(char), "%F", times);
    return 0;
}
int get_full_path(char *filename, char *buffer){
    if(realpath(filename,buffer) == NULL) return -1;
    return 0;
}

void traverse_directory(char *dirpath){
    DIR* dir = opendir(dirpath);
    struct dirent *dirptr = NULL;
    struct stat *buffer = malloc(sizeof(struct stat));
    int flag;
    char modify_date[256];
    char access_date[256];
    char path[256];
    if(dir == NULL) return;
    chdir(dirpath);
    errno = 0;
    dirptr = readdir(dir);
    while(dirptr != NULL)
    {
        if(strcmp(dirptr->d_name, ".") == 0 || strcmp(dirptr->d_name,"..") == 0){
            dirptr = readdir(dir);
            continue;
        }
        if(lstat(dirptr->d_name,buffer) != 0) break;
        flag = 1;
        switch(op){
            case '>': 
                if(buffer->st_mtime <= time_from_epoch){ 
                flag = 0;
                }
                break;
            case '<': if(buffer->st_mtime >= time_from_epoch){
                flag = 0;
                }
                break;
            case '=': if(buffer->st_mtime == time_from_epoch){ 
                flag = 0;
                }
            default: 
                flag=0;
                break;
        }
        get_date_string(buffer->st_mtime, modify_date);
        get_date_string(buffer->st_atime, access_date);
        get_full_path(dirptr->d_name, path);
        if(flag == 1){
            printf("%s\t%s\t%d\t%s\t%s\n",
                path,
                get_file_type(buffer->st_mode),
                (int) buffer->st_size,
                access_date,
                modify_date
            );
        }
        if(S_ISDIR(buffer->st_mode) != 0){
            traverse_directory(path);
        }
        chdir(dirpath);
        dirptr = readdir(dir);
    }
    free(buffer);
    closedir(dir);
}

static int nftw_function(const char* fpath, const struct stat *file_stat, int typeflag, struct FTW* ftwbuf){
    if(ftwbuf->level == 0) return 0;
    char modify_date[256];
    char access_date[256];
    int flag;
    flag = 1;
    switch(op){
        case '>': 
            if(file_stat->st_mtime <= time_from_epoch){ 
            flag = 0;
            }
            break;
        case '<': if(file_stat->st_mtime >= time_from_epoch){
            flag = 0;
            }
            break;
        case '=': if(file_stat->st_mtime == time_from_epoch){ 
            flag = 0;
            }
        default: 
            flag=0;
            break;
    }
    get_date_string(file_stat->st_mtime, modify_date);
    get_date_string(file_stat->st_atime, access_date);
    if(flag == 1){
        printf("%s\t%s\t%d\t%s\t%s\n",
            fpath,
            get_file_type(file_stat->st_mode),
            (int) file_stat->st_size,
            access_date,
            modify_date
        );
    }
    return 0;
}

void nftw_wrapper(char *path){
    char *buffer = malloc(256*sizeof(char));
    get_full_path(path, buffer);
    nftw(buffer, nftw_function, 10, FTW_PHYS);
    free(buffer);
}