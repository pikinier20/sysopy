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
#include <libgen.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

int count = 0;
int signals;

void send_signal(int mode, int pid, int i){
    union sigval value;
    switch(mode){
        case 0:
            kill(pid, SIGUSR1);
            break;
        case 1:
            value.sival_int = i;
            sigqueue(pid,SIGUSR1,value);
            break;
        case 2:
            kill(pid,SIGRTMIN);
            break;
    }
}

void send_termination_signal(int mode, int pid){
    union sigval value;
    switch(mode){
        case 0:
            kill(pid, SIGUSR2);
            break;
        case 1:
            value.sival_int = -1;
            sigqueue(pid,SIGUSR2,value);
            break;
        case 2:
            kill(pid, SIGRTMIN+1);
            break;
    }
}

void count_function(int sig,siginfo_t *info, void *void_ptr){
    count = count + 1;
    if(info->si_code == SI_QUEUE){
        printf("Odebrano sygnal nr %d od catchera \n",info->si_value.sival_int);
    }
}

void termination_function(int sig,siginfo_t *info, void *void_ptr){
    printf("Wyslano %d sygnalow do catchera\n",signals);
    printf("Odebrano w sumie %d sygnalow od catchera\n", count);
    exit(0);
}

void set_count_handler(int mode){
    struct sigaction action;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    switch(mode){
        case 0:
            action.sa_sigaction = &count_function;
            sigaction(SIGUSR1,&action,NULL);
            break;
        case 1:
            action.sa_sigaction = &count_function;
            sigaction(SIGUSR1,&action,NULL);
            break;
        case 2:
            action.sa_sigaction = &count_function;
            sigaction(SIGRTMIN,&action,NULL);
            break;
    }
}

void set_termination_handler(int mode){
    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = &termination_function;
    switch(mode){
        case 0:
            sigaction(SIGUSR2,&action,NULL);
            break;
        case 1:
            sigaction(SIGUSR2,&action,NULL);
            break;
        case 2:
            sigaction(SIGRTMIN+1,&action,NULL);
            break;
    }
}

int main(int argc, char **argv){
    int catcher_pid;
    int mode=-1;
    int i;
    if(argc != 4){
        printf("Oczekuje 3 argumentow - PID catchera, ilosc sygnalow oraz tryb uruchomienia: KILL, SIGQUEUE lub SIGRT \n");
        return -1;
    }
    if(sscanf(argv[1],"%d",&catcher_pid) != 1){
        printf("Podaj poprawny PID catchera \n");
        return -1;
    }
    if(sscanf(argv[2],"%d",&signals) != 1){
        printf("Podaj poprawna ilosc sygnalow do wyslania \n");
        return -1;
    }
    if(strcmp(argv[3],"KILL") == 0) mode = 0;
    if(strcmp(argv[3],"SIGQUEUE") == 0) mode = 1;
    if(strcmp(argv[3],"SIGRT") == 0) mode = 2;
    if(mode == -1){
        printf("Wpisz poprawny tryb uruchomienia: KILL, SIGQUEUE lub SIGRT \n");
        return -1;
    }
    sigset_t masked_set;
    sigset_t old_set;
    sigfillset(&masked_set);
    sigdelset(&masked_set,SIGUSR1);
    sigdelset(&masked_set,SIGUSR2);
    sigdelset(&masked_set,SIGRTMIN);
    sigdelset(&masked_set,SIGRTMIN+1);
    //sigprocmask(SIG_SETMASK,&masked_set,&old_set);

    set_count_handler(mode);
    set_termination_handler(mode);

    for(i = 0; i < signals; i++){
        send_signal(mode,catcher_pid,i);
    }

    send_termination_signal(mode, catcher_pid);
    while(1){
        sleep(1);
    }

    return 0;
}