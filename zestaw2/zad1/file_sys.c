#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "file_sys.h"


int sys_sort(char *filepath, int count, int size){
    int offset = (size+1)*sizeof(char);
    int desc = open(filepath,O_RDWR);
    char* current = malloc(offset);
    char* min = malloc(offset);
    int minpos;
    int sortpos = 0;
    int i;

    if(desc < 0) return -1;
    while(sortpos < count){

        lseek(desc, sortpos*offset, SEEK_SET);
        minpos = sortpos;
        if(read(desc, min, offset) != offset) {
            return 1;
        }
        for(i = sortpos+1; i < count; i++){
            if(read(desc,current,offset) != size+1) return 1;
            if(current[0] < min[0]){
                minpos = i;
                strcpy(min,current);
            }
        }
        if(minpos != sortpos){
            lseek(desc, sortpos*offset,SEEK_SET);
            if(read(desc,current,offset) != size+1) return 1;
            lseek(desc,sortpos*offset,SEEK_SET);
            if(write(desc,min,offset) != size+1) return 1;
            lseek(desc,minpos*offset,SEEK_SET);
            if(write(desc,current,offset) != size+1) return 1;
        }
        sortpos = sortpos+1;
    }
    close(desc);
    free(min);
    free(current);
    return 0;

}

int sys_copy(char *source, char *dest, int count, int size){
    int sourcedesc = open(source, O_RDONLY);
    int destdesc = open(dest, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if(sourcedesc < 0 || destdesc < 0) return -1;
    char* buffer = malloc((size+1) * sizeof(char));
    int i;
    for(i = 0; i < count; i++){
        if(read(sourcedesc, buffer,(size_t) (size+1) * sizeof(char)) != (size+1) * sizeof(char)) return 1;
        if(write(destdesc, buffer,(size_t) (size+1) * sizeof(char)) != (size+1) * sizeof(char)) return 1;
    }
    free(buffer);
    close(sourcedesc);
    close(destdesc);
    return 0;
}