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
int sender_pid = -1;

void count_function_kill(int sig,siginfo_t *info, void *void_ptr){
    count = count + 1;
    sender_pid = info->si_pid;

    kill(sender_pid, SIGUSR1);
}
void count_function_queue(int sig,siginfo_t *info, void *void_ptr){
    sender_pid = info->si_pid;
    union sigval value;
    value.sival_int = count;
    sigqueue(sender_pid,SIGUSR1,value);
    count = count + 1;
}
void count_function_kill_rt(int sig,siginfo_t *info, void *void_ptr){
    count = count + 1;
    sender_pid = info->si_pid;

    kill(sender_pid, SIGRTMIN);
}

void end_function_kill(int sig,siginfo_t *info, void *void_ptr){
    kill(sender_pid, SIGUSR2);
    printf("Catcher: Dostalem %d sygnalow \n",count);
    exit(0);
}

void end_function_queue(int sig,siginfo_t *info, void *void_ptr){
    union sigval value;
    value.sival_int = count;
    sigqueue(sender_pid, SIGUSR2, value);
    printf("Catcher: Dostalem %d sygnalow \n",count);
    exit(0);
}
void end_function_kill_rt(int sig,siginfo_t *info, void *void_ptr){
    int i;
    if(sender_pid != -1){
    for(i = 0; i < count; i++){
        kill(sender_pid, SIGRTMIN);
    }
    kill(sender_pid, SIGRTMIN+1);
    }
    printf("Catcher: Dostalem %d sygnalow \n",count);
    exit(0);
}


void set_count_handler(int mode){
    struct sigaction action;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    switch(mode){
        case 0:
            action.sa_sigaction = &count_function_kill;
            sigaction(SIGUSR1,&action,NULL);
            break;
        case 1:
            action.sa_sigaction = &count_function_queue;
            sigaction(SIGUSR1,&action,NULL);
            break;
        case 2:
            action.sa_sigaction = &count_function_kill_rt;
            sigaction(SIGRTMIN,&action,NULL);
            break;
    }
}

void set_end_handler(int mode){
    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;
    switch(mode){
        case 0:
            action.sa_sigaction = &end_function_kill;
            sigaction(SIGUSR2,&action,NULL);
            break;
        case 1:
            action.sa_sigaction = &end_function_queue;
            sigaction(SIGUSR2,&action,NULL);
            break;
        case 2:
            action.sa_sigaction = &end_function_kill_rt;
            sigaction(SIGRTMIN+1,&action,NULL);
            break;
    }
}

int main(int argc, char **argv){
    if(argc != 2){
        printf("Oczekuje 1 argumentu - tryb uruchomienia: KILL, SIGQUEUE lub SIGRT \n");
        return -1;
    }
    int mode=-1;
    if(strcmp(argv[1],"KILL") == 0) mode = 0;
    if(strcmp(argv[1],"SIGQUEUE") == 0) mode = 1;
    if(strcmp(argv[1],"SIGRT") == 0) mode = 2;
    if(mode == -1){
        printf("Wpisz poprawny tryb uruchomienia: KILL, SIGQUEUE lub SIGRT \n");
        return -1;
    }
    set_count_handler(mode);
    set_end_handler(mode);
    sigset_t masked_set;
    sigset_t old_set;
    sigfillset(&masked_set);
    sigdelset(&masked_set,SIGUSR1);
    sigdelset(&masked_set,SIGUSR2);
    sigdelset(&masked_set,SIGRTMIN);
    sigdelset(&masked_set,SIGRTMIN+1);
    //sigprocmask(SIG_SETMASK,&masked_set,&old_set);

    printf("PID catchera: %d \n",getpid());
    while(1){
        pause();
    }
    return 0;
}