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
    if(argc != 3){
        printf("Niewlasciwa ilosc argumentow w wywolaniu. Spodziewano sie nazwy pliku. \n");
        return -1;
    }
    int fifo_desc = open(argv[1],O_WRONLY);
    char buffer[128];
    char date_str[128];
    FILE* date_pipe;
    char *endptr = NULL;
    if(fifo_desc == -1){
        printf("Wystapil blad przy probie otwarcia potoku. \n");
        return -1;
    }
    errno = 0;
    int n = (int) strtol(argv[2],&endptr,10);
    if(errno != 0 || *endptr){
        printf("Drugim argumentem powinna byc liczba. \n");
        return -1;
    }
    printf("%d \n", getpid());
    while(n--){
        date_pipe = popen("date","r");
        if(date_pipe == NULL){
            printf("Wystapil blad przy probie otwarcia potoku date \n");
            break;
        }
        sprintf(buffer, "%d ", getpid());
        fread(date_str, sizeof(char), 128, date_pipe);
        strcat(buffer, date_str);
        write(fifo_desc,buffer, 128*sizeof(char));
        if(pclose(date_pipe) != 0){
            printf("Wystapil blad przy probie zamkniecia potoku date \n");
            break;
        }
        sleep(3);
    }
    if(close(fifo_desc) != 0){
        printf("Wystapil blad przy probie zamkniecia potoku \n");
        return;
    }
    return 0;
}