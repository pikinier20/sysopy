#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/times.h>
#include <string.h>
#include <zconf.h>
#include <errno.h>
#include "file_sys.h"
#include "file_lib.h"

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
    fclose(file);
    fclose(random);
    return 0;
}
int main(int argc, char** argv){
    int res;
    res = generate("./tempsys",100,512);
    if(res != 0) printf("NGenerate");
    res = sys_sort("./tempsys",100,512);
    if(res != 0) printf("NSort %d",res);
    res = generate("./templib",100,512);
    lib_copy("./templib","./copylib",100,512);
    if(res != 0) printf("NGenerate");
    res = lib_sort("./templib",100,512);
    if(res != 0) printf("NSort %d",res);
    res = sys_copy("./tempsys","./copysys",100,512);
    if(res != 0) printf("NCopy error");
    lib_copy("./templib","./copylib",100,512);
    return 0;
}
