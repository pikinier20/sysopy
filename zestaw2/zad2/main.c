#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/times.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "lib.h"

int main(int argc, char **argv){
    char *dirpath;
    struct tm* time_struct = malloc(sizeof(struct tm));
    if(argc != 5){
        printf("Provide 3 arguments: directory path, date, compare operator and `first` or `second`\n");
        free(time_struct);
        return -1;
    }
    dirpath = argv[1];
    if(strptime(argv[2],"%F",time_struct) == NULL){
        printf("Provide date in format: YYYY-MM-DD\n");
        free(time_struct);
        return -1;
    }
    if(strlen(argv[3]) != 3 && argv[3][0] != '>' && argv[3][0] != '<' && argv[3][0] != '='){
        printf("Third arument should be compare operator\n");
        free(time_struct);
        return -1;
    }
    op = argv[3][0];
    time_from_epoch = mktime(time_struct);
    if(strcmp(argv[4],"first") == 0){
        traverse_directory(realpath(dirpath,NULL));
    }
    if(strcmp(argv[4],"second") == 0){
        nftw_wrapper(dirpath);
    }


    free(time_struct);
    return 0;
}