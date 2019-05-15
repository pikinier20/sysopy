#include "belt.h"

int beltInit(Belt *belt, int maxQuantity, int maxWeight){
    belt->maxQuantity = maxQuantity;
    belt->maxWeight = maxWeight;
    belt->curWeight = 0;
    belt->curQuantity = 0;
    belt->head = -1;
    belt->tail = 0;
    return 0;
}

int isEmptyBelt(Belt *fifo) {
    if (fifo->head == -1) return 1;
    else return 0;
}

int isFullBelt(Belt *fifo) {
    if (fifo->head == fifo->tail) return 1;
    else return 0;
}

int popBelt(Belt *fifo,Box *boxaddr) {
    if (isEmptyBelt(fifo) == 1) return -1;

    Box box = fifo->fifo[fifo->head++];

    if (fifo->head == fifo->maxQuantity) fifo->head = 0;

    if (fifo->head == fifo->tail) fifo->head = -1;

    fifo->curWeight -= box.weight;
    fifo->curQuantity--;
    *boxaddr = box;
    return 0;
}

int pushBelt(Belt *fifo, Box box) {
    if (fifo->curQuantity == fifo->maxQuantity || fifo->curWeight + box.weight > fifo->maxWeight) {
        return -1;
    }
    if (isEmptyBelt(fifo) == 1)
        fifo->head = fifo->tail = 0;

    fifo->fifo[fifo->tail++] = box;

    fifo->curWeight += box.weight;
    fifo->curQuantity++;
    if (fifo->tail == fifo->maxQuantity) fifo->tail = 0;
    return 0;
}

long getMicroTime() {
    struct timespec marker;
    clock_gettime(CLOCK_MONOTONIC, &marker);
    return marker.tv_nsec / 1000;
}

struct timeval getTime(){
    struct timeval timebuffer;
    gettimeofday(&timebuffer, NULL);
    return timebuffer;
}