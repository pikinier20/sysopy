#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include "file_lib.h"


int lib_sort(char *filepath, int count, int size){
    FILE* file = fopen(filepath,"r+");
    int offset = (size+1)*sizeof(char);
    char* current = malloc(offset);
    char* min = malloc(offset);
    int minpos;
    int sortpos = 0;
    int i;

    if(file == NULL) return -1;
    while(sortpos < count){
        fseek(file, sortpos*offset, SEEK_SET);
        minpos = sortpos;
        if(fread(min, sizeof(char), size+1,file) != size+1) {
            return 1;
        }
        for(i = sortpos+1; i < count; i++){
            if(fread(current,sizeof(char),size+1,file) != size+1) return 1;
            if(current[0] < min[0]){
                minpos = i;
                strcpy(min,current);
            }
        }
        if(minpos != sortpos){
            fseek(file, sortpos*offset,SEEK_SET);
            if(fread(current,sizeof(char),size+1,file) != size+1) return 1;
            fseek(file,sortpos*offset,SEEK_SET);
            if(fwrite(min,sizeof(char),size+1,file) != size+1) return 1;
            fseek(file,minpos*offset,SEEK_SET);
            if(fwrite(current,sizeof(char),size+1,file) != size+1) return 1;
        }
        sortpos = sortpos+1;
    }
    fclose(file);
    free(min);
    free(current);
    return 0;

}

int lib_copy(char *source, char* dest, int count, int size){
    FILE* sourcefile = fopen(source, "r");
    FILE* destfile = fopen(dest,"w");
    if(sourcefile == NULL || destfile == NULL) return -1;
    char* buffer = malloc((size+1) * sizeof(char));
    int i;
    for(i = 0; i < count; i++){
        if(fread(buffer,sizeof(char),(size_t) size+1,sourcefile) != size+1) return 1;
        if(fwrite(buffer,sizeof(char),(size_t) size+1,destfile) != size+1) return 1;
    }
    free(buffer);
    fclose(sourcefile);
    fclose(destfile);
    return 0;
}