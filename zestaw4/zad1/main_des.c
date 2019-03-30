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

int child_pid=-1;

void create_descendant(){
    child_pid = fork();
    return;
}

void sigtstp_function(int sig){
    if(child_pid != -1){
        printf("Zabito proces potomny o PID: %d \n",child_pid);
        kill(child_pid,SIGKILL);
        child_pid = -1;
    }
    else{
        create_descendant();
        if(child_pid != 0){
            printf("Utworzono proces potomny o PID: %d \n",child_pid);
        }
    }
    return;
}

void sigint_function(int sig){
    printf("Odebrano sygnal SIGINT \n");
    if(child_pid != -1){
        printf("Zabito proces potomny o PID: %d \n",child_pid);
        child_pid = -1;
    }
    else{
        printf("Nie zabito procesu potomnego, bo nie istnieje\n");
    }
    printf("Proces konczy dzialanie \n");
    exit(0);
}


int main(int argc, char **argv){
    struct sigaction action;
    struct sigaction oldaction;
    action.sa_handler = sigtstp_function;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if(child_pid != 0){
        create_descendant();
        sigaction(SIGTSTP,&action,&oldaction);
        signal(SIGINT,sigint_function);
    }

    while(child_pid != 0){
        sleep(1);
    }

    printf("xd");
    execlp("./script","./script",(char*) 0);

    return 0;
}