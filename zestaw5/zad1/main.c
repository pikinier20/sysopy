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
#include <libgen.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

char* programs[64][64];

int run_exec(int arg_n){
    exit(0);
}

int parse_programs(char *buffer){
    char* p = strtok(buffer," ");
    int i = 0;
    int j = 0;
    while(p != NULL){
        if(p[0] == '|' && p[1] == '\0'){
            programs[i][j] = NULL;
            i++;
            j = 0;
        }
        else{
            programs[i][j++] = p;
        }
        p = strtok(NULL," \n");
    }
    return i;
}


int main(int argc, char **argv){
    char buffer[512];
    int curr_desc[2];
    int prev_desc[2];
    int child_pids[256];
    int pid;
    int i = 0;
    int j;
    int flag = 1;
    int arg_n;
    int status;
    if(argc != 2){
        printf("Niewlasciwa ilosc argumentow w wywolaniu. Spodziewano sie nazwy pliku. \n");
        return -1;
    }
    FILE* file = fopen(argv[1],"r");
    if(file == NULL){
        printf("Wystapil blad przy probie otwarcia pliku. \n");
        return -1;
    }
    if(fgets(buffer,512 * sizeof(char),file) == NULL){
        printf("Wystapil problem z wywolaniem pierwszej komendy. \n");
        return -1;
    }
    while(1){
        arg_n = parse_programs(buffer);
        for(i = 0; i <= arg_n; i++){
            if(pipe(curr_desc) == -1){
                fprintf(stderr, "Blad w czasie otwarcia potoku \n");
                exit(-1);
            }
            child_pids[i] = fork();
            if(child_pids[i]){
                if(i > 0){
                    dup2(prev_desc[0],STDIN_FILENO);
                    close(prev_desc[1]);
                    close(prev_desc[0]);
                }
                if(i < arg_n){
                    dup2(curr_desc[1],STDOUT_FILENO);
                    close(curr_desc[0]);
                    close(curr_desc[1]);
                }
                execvp(programs[i][0],programs[i]);
                exit(0);
            }
            else if(pid > 0){
                if(i > 0){
                    close(prev_desc[0]);
                    close(prev_desc[1]);
                }
                if(i < arg_n){
                    prev_desc[0] = curr_desc[0];
                    prev_desc[1] = curr_desc[1];
                }
            }
        }
        for(i = 0; i < arg_n; i++) waitpid(child_pids[i],&status,WIFEXITED(0));
        if(fgets(buffer,512 * sizeof(char),file) == NULL) break;

    }

    return 0;
}