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





int main(int argc, char ** argv) {
    if(argc != 5) return -1;
    FILE * file;
    file = fopen(argv[1], "a+");
    if(file == NULL) return -1;
    int pmin, pmax, bytes;
    sscanf(argv[2], "%d", &pmin);
    sscanf(argv[3], "%d", &pmax);
    sscanf(argv[4], "%d", &bytes);
    char dest[257];
    char seconds[128];
    char timestring[128];
    char pid[15];
    srand(time(NULL));
    int s;
    struct tm *t;
    int i;


    while(1 == 1) {
        s = rand()%(pmax - pmin + 1) + pmin;
        sleep(s);
        time_t mytime = time((time_t*)0);
        t = localtime(&mytime);
        strftime(timestring, 128, "%c", t);
        sprintf(seconds, "%d", s);
        sprintf(pid, "%d", getpid());
        strcpy(dest, pid);
        strcat(dest, " ");
        strcat(dest, seconds);
        strcat(dest, " ");
        strcat(dest, timestring);
        fseek(file, 0, SEEK_END);
        fputs(dest, file);
        i = 0;
        while(i < bytes){
            fputs("A",file);
            i++;
        }
        fputs("\n",file);
    }
    free(t);
    fclose(file);
    return 0;
}