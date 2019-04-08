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
#include <sys/time.h>
#include <fcntl.h>
#include <libgen.h>
#include <time.h>
#include <errno.h>
#include <signal.h>


int main(int argc, char **argv){
    if(argc != 2){
        printf("Niewlasciwa ilosc argumentow w wywolaniu. Spodziewano sie nazwy pliku. \n");
        return -1;
    }
    if(mkfifo(argv[1],S_IRWXU) == -1){
        printf("Wystapil blad przy probie utworzenia potoku. \n");
        return -1;
    }
    int fifo_desc = open(argv[1],O_RDONLY);
    if(fifo_desc == -1){
        printf("Wystapil blad przy probie otwarcia potoku. \n");
        return -1;
    }
    char *buffer = malloc(1024*sizeof(char));
    while(1){
        if(read(fifo_desc, buffer,1024) != 0){
            printf("%s",buffer);
        }

    }
    return 0;
}