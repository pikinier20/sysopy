#define _XOPEN_SOURCE 500
#include "belt.h"
#define log(format, ...) {printf("\033[1;35m%ld.%.6ld Trucker: \033[0m",getTime().tv_sec,getTime().tv_usec); printf(format, ##__VA_ARGS__);}
#define FAILURE_EXIT(code, format, ...) { log(format, ##__VA_ARGS__); exit(code);}

Belt *fifo = NULL;
key_t fifoKey;
int shmID = -1;
int shm2ID = -1;
int SID = -1;
int currAmount;
int maxAmount;
int maxWeight;
int maxQuantity;
Box box;
int status;
int queue;
sem_t *trucker_sem;
sem_t *loaders_sem;
sem_t *belt_sem;

char *nullptr = NULL;

void intHandler(int);

void clearResources(void);

void prepareFifo(int maxWeight, int maxQuantity);

void prepareSemaphores();

void loadBoxes();

void unloadTruck();

int main(int argc, char **argv){
    if (argc < 4) FAILURE_EXIT(3, "Oczekiwano parametrow: pojemnosc ciezarowki, pojemnosc tasmy, maksymalna waga tasmy \n");
    maxAmount = (int) strtol(argv[1], &nullptr, 10);
    if (maxAmount < 1) FAILURE_EXIT(3, "Zla pojemnosc ciezarowki \n");
    if(!(!*nullptr)) FAILURE_EXIT(3,"Zla pojemnosc ciezarowki \n");

    maxQuantity = (int) strtol(argv[2], &nullptr, 10);
    if (maxQuantity < 1) FAILURE_EXIT(3, "Zla pojemnosc tasmy \n");
    if(!(!*nullptr)) FAILURE_EXIT(3,"Zla pojemnosc tasmy \n");

    maxWeight = (int) strtol(argv[3], &nullptr, 10);
    if (maxWeight < 1) FAILURE_EXIT(3, "Zla maksymalna waga tasmy \n");
    if(!(!*nullptr)) FAILURE_EXIT(3,"Zla maksymalna waga tasmy \n");

    if (atexit(clearResources) == -1) FAILURE_EXIT(3, "Blad podczas ustawiania operacji atexit");
    if (signal(SIGINT, intHandler) == SIG_ERR) FAILURE_EXIT(3, "Blad podczas obslugi sygnalu SIGINT");

    prepareFifo(maxWeight, maxQuantity);

    prepareSemaphores();

    loadBoxes();
}

void loadBoxes(){
    while (1) {
        if (sem_wait(trucker_sem) == -1) FAILURE_EXIT(3, "Blad przy przywlaszczeniu semaforu TRUCKER \n");
        if (sem_wait(belt_sem) == -1) FAILURE_EXIT(3, "Blad przy przywlaszczeniu semaforu BELT \n");
        status = popBelt(fifo, &box);
        while(status == 0){
            if(currAmount == maxAmount) {
                log("Brak miejsca - nastepuje rozladowanie ciezarowki \n");
                unloadTruck();
            }
            currAmount++;
            log("Zaladowano na ciezarowke paczke o wadze: %d, pid: %d. Minelo %ld czasu od polozenia na tasmie. Pozostalo %d wolnych miejsc \n",
            box.weight,box.pid,getMicroTime()-box.time,maxAmount-currAmount);
            status = popBelt(fifo, &box);
        }
        if (sem_post(belt_sem) == -1) FAILURE_EXIT(3, "Blad przy oddawaniu semaforu BELT \n");

        if (sem_post(loaders_sem) == -1) FAILURE_EXIT(3, "Blad przy oddawaniu semaforu TRUCKER \n");
    }

}
void unloadTruck(){
    currAmount = 0;
    log("Ciezarowka zostala rozladowana i podjechala pusta \n");
}

void prepareFifo(int maxWeight, int maxQuantity) {
    shmID = shm_open("belt_memory", O_CREAT | O_EXCL | O_RDWR, 0666);
    if (shmID == -1) {FAILURE_EXIT(3, "Blad podczas tworzenia pamieci wspoldzielonej \n");}
    if (ftruncate(shmID, sizeof(Belt)) == -1) FAILURE_EXIT(3, "Blad podczas ustawiania rozmiaru pamieci wspoldzielonej \n");
    void *tmp = mmap(NULL, sizeof(Belt), PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
    if (tmp == (void *) (-1)) FAILURE_EXIT(3, "Blad podczas dolaczania pamieci wspoldzielonej do przestrzeni adresowej procesu \n");
    fifo = (Belt *) tmp;
    beltInit(fifo, maxQuantity, maxWeight);


    struct mq_attr queue_attr;
    queue_attr.mq_maxmsg = 10;
    queue_attr.mq_msgsize = 10;
    char error[50];

    if ((queue = mq_open("/pidQueue",O_CREAT | O_EXCL | O_RDONLY | O_NONBLOCK, 0666, &queue_attr)) == -1){
        perror(error);
        FAILURE_EXIT(3,"Nie mozna otworzyc kolejki serwera %s\n",error);
    }
    
}

void prepareSemaphores() {
    trucker_sem = sem_open("trucker_sem", O_CREAT | O_EXCL | O_RDWR, 0666, 0);
    if (trucker_sem == SEM_FAILED) FAILURE_EXIT(3, "Blad podczas pobierania semaforow \n");
    loaders_sem = sem_open("loaders_sem",O_CREAT | O_EXCL | O_RDWR, 0666, 1);
    if (loaders_sem == SEM_FAILED) FAILURE_EXIT(3, "Blad podczas pobierania semaforow \n");
    belt_sem = sem_open("belt_sem",O_CREAT | O_EXCL | O_RDWR, 0666, 1);
    if (belt_sem == SEM_FAILED) FAILURE_EXIT(3, "Blad podczas pobierania semaforow \n");
}

void clearResources(void) {

    char pid[10];
    char error[100];
    int i;
    while(mq_receive(queue, pid, 10, NULL) != -1){
        i = strtol(pid,NULL,10);
        kill(i, SIGINT);
    }

    printf("tutaj \n");

    if (mq_close(queue) == -1)
        log("Nie mozna zamknac kolejki\n");
    if (mq_unlink("/pidQueue") == -1) log("Blad podczas usuwania kolejki \n");

    if(belt_sem != NULL && fifo != NULL){
        status = popBelt(fifo, &box);
        while(status == 0){
            if(currAmount == maxAmount) {
                log("Brak miejsca - nastepuje rozladowanie ciezarowki \n");
                unloadTruck();
            }
            currAmount++;
            log("Zaladowano na ciezarowke paczke o wadze: %d, pid: %d. Minelo %ld czasu od polozenia na tasmie. Pozostalo %d wolnych miejsc \n",
            box.weight,box.pid,getMicroTime()-box.time,maxAmount-currAmount);
            status = popBelt(fifo, &box);
        }
    }

    printf("tutaj \n");

    if(fifo != NULL)
    if (munmap(fifo, sizeof(fifo)) == -1) {log("Blad podczas odlaczania pamieci wspoldzielonej od przestrzeni adresowej \n");}
    else log("Odlaczono pamiec wspoldzielona\n");

    if (shm_unlink("belt_memory") == -1) {log("Blad podczas usuwania pamieci wspoldzielonej \n");}
    else log("Usunieto pamiec wspoldzielona belt\n");

    if(trucker_sem != NULL)
    if(sem_close(trucker_sem) == -1){log("Blad podczas odlaczania semaforu \n");}
    if(sem_unlink("trucker_sem") == -1){log("Blad podczas usuwania semaforu \n");}
    if(loaders_sem != NULL)
    if(sem_close(loaders_sem) == -1){log("Blad podczas odlaczania semaforu \n");}
    if(sem_unlink("loaders_sem") == -1) {log("Blad podczas usuwania semaforu \n");}
    if(belt_sem != NULL)
    if(sem_close(belt_sem) == -1){log("Blad podczas odlaczania semaforu \n");}
    if(sem_unlink("belt_sem") == -1) {log("Blad podczas usuwania semaforu \n");}
    else log("Usunieto semafory\n");
}

void intHandler(int signo) {
    exit(2);
}