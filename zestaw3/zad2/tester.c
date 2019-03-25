#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>
#include <time.h>


const char format[] = "_%Y-%m-%d_%H-%M-%S";



int main(int argc, char ** argv) {
    FILE * file;
    file = fopen(argv[1], "a+");
    if(file == NULL) return -1;
    if(argc != 5) return -1;
    int pmin, pmax, bytes;
    sscanf(argv[2], "%d", &pmin);
    sscanf(argv[3], "%d", &pmax);
    sscanf(argv[4], "%d", &bytes);
    int size = (int) floor(log10(abs(bytes))) + 20 + bytes + 1;
    char dest[size];
    char sec[(int) floor(log10(abs(bytes))) + 1];
    char str[21];
    srand(time(NULL));
    int s;
    struct tm *t;

    while(1 == 1) {
        s = rand()%(pmax - pmin) + pmin;
        sleep(s);
        time_t mytime = time((time_t*)0);
        t = localtime(&mytime);
        strftime(str, 21, format, t);
        sprintf(sec, "%d", s);
        strcpy(dest, sec);
        strcat(dest, str);
        for(int i=0; i<bytes; i++) {
            dest[(int) (floor(log10(abs(bytes)))) + 20 + i] = 'A';
        }
        fseek(file, 0, SEEK_END);
        fputs(dest, file);
    }
    
    free(t);
    fclose(file);
    return 0;
}