#ifndef BELT_H
#define BELT_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

#define PROJECT_ID 77

typedef enum semType{
    TRUCKER = 0, BELT = 1, LOADERS = 2
}semType;

typedef struct Box{
    int weight;
    pid_t pid;
    long time;
}Box;

typedef struct Belt{
    int curWeight;
    int curQuantity;
    int maxWeight;
    int maxQuantity;
    int head;
    int tail;
    Box fifo[1024];
}Belt;

int beltInit(Belt *belt,int maxQuantity, int maxWeight);

int popBelt(Belt *belt,Box *box);

int pushBelt(Belt *belt, Box box);

long getMicroTime();

#endif