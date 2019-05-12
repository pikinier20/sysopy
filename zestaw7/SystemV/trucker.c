#define _GNU_SOURCE
#include "belt.h"

#define log(format, ...) {printf("\033[1;35mTrucker: \033[0m"); printf(format, ##__VA_ARGS__);}
#define FAILURE_EXIT(code, format, ...) { log(format, ##__VA_ARGS__); exit(code);}

Belt *fifo = NULL;
key_t fifoKey;
int shmID = -1;
int SID = -1;
int currAmount;
int maxAmount;
int maxWeight;
int maxQuantity;

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
    Box box;
    int status;
    struct sembuf sops;
    sops.sem_flg = 0;
    while (1) {
        sops.sem_num = TRUCKER;
        sops.sem_op = -1;

        if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Blad przy przywlaszczeniu semaforu TRUCKER \n");

        sops.sem_num = BELT;
        sops.sem_op = -1;
        if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Blad przy przywlaszczeniu semaforu BELT \n");

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
        sops.sem_num = BELT;
        sops.sem_op = 1;
        if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Blad przy oddawaniu semaforu BELT \n");

        sops.sem_num = TRUCKER;
        sops.sem_op = 1;
        if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Blad przy oddawaniu semaforu TRUCKER \n");
    }

}
void unloadTruck(){
    currAmount = 0;
    log("Ciezarowka zostala rozladowana i podjechala pusta \n");
}

void prepareFifo(int maxWeight, int maxQuantity) {
    char *path = getenv("HOME");
    if (path == NULL) FAILURE_EXIT(3, "Blad podczas pobierania zmiennej srodowiskowej PATH \n");

    fifoKey = ftok(path, PROJECT_ID);
    if (fifoKey == -1) FAILURE_EXIT(3, "Blad podczas pobierania klucza pamieci wspoldzielonej \n");

    shmID = shmget(fifoKey, sizeof(Belt), IPC_CREAT | IPC_EXCL | 0666);
    if (shmID == -1) FAILURE_EXIT(3, "Blad podczas tworzenia pamieci wspoldzielonej \n");

    void *tmp = shmat(shmID, NULL, 0);
    if (tmp == (void *) (-1)) FAILURE_EXIT(3, "Blad podczas dolaczania pamieci wspoldzielonej do przestrzeni adresowej procesu \n");
    fifo = (Belt *) tmp;

    beltInit(fifo, maxQuantity, maxWeight);
}

void prepareSemaphores() {
    SID = semget(fifoKey, 4, IPC_CREAT | IPC_EXCL | 0666);
    if (SID == -1) FAILURE_EXIT(3, "Blad podczas tworzenia semaforu \n");
    int i;
    for (i = 1; i < 3; i++)
        if (semctl(SID, i, SETVAL, 1) == -1) FAILURE_EXIT(3, "Blad podczas ustawiania wartosci semaforu \n");

    if (semctl(SID, TRUCKER, SETVAL, 0) == -1) FAILURE_EXIT(3, "Blad podczas ustawiania wartosci semaforu \n");
}

void clearResources(void) {
    if (shmdt(fifo) == -1) {log("Blad podczas odlaczania pamieci wspoldzielonej od przestrzeni adresowej \n");}
    else log("Odlaczono pamiec wspoldzielona\n");

    if (shmctl(shmID, IPC_RMID, NULL) == -1) {log("Blad podczas usuwania pamieci wspoldzielonej \n");}
    else log("Usunieto pamiec wpoldzielona\n");

    if (semctl(SID, 0, IPC_RMID) == -1) {log("Blad podczas usuwania semaforow \n");}
    else log("Usunieto semafory\n");
}

void intHandler(int signo) {
    exit(2);
}