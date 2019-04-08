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

int flag=1;

void sigtstp_function(int sig){
    if(flag == 1){
    printf("Oczekuje na CTRL+Z - kontynuacja albo CTRL+C - zakonczenie programu \n");
    }
    flag = (flag + 1) % 2;
    return;
}

void sigint_function(int sig){
    printf("Odebrano sygnal SIGINT \n");
    exit(0);
}

int main(int argc, char **argv){
    struct sigaction action;
    struct sigaction oldaction;
    action.sa_handler = sigtstp_function;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGTSTP,&action,&oldaction);
    signal(SIGINT,sigint_function);
    struct tm *curr_date;
    time_t time_from_epoch;
    char date_string[255];
    while(1){
        while(flag){
        time_from_epoch = time(NULL);
        curr_date = localtime(&time_from_epoch);
        strftime(date_string,255*sizeof(char),"%Y-%m-%dT%H:%M:%S",curr_date);
        printf("%s \n",date_string);
        sleep(1);
        }
    }
    return 0;
}