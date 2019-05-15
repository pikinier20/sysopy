#define _GNU_SOURCE
#include "belt.h"
#define log(format, ...) {printf("\033[1;31m%ld.%.6ld Loader %d: \033[0m",getTime().tv_sec,getTime().tv_usec,getpid()); printf(format, ##__VA_ARGS__);}
#define FAILURE_EXIT(code, format, ...) { log(format, ##__VA_ARGS__); exit(code);}

Belt *belt = NULL;
key_t beltKey;
sigset_t fullMask;
int shmID = -1;
int SID = -1;
int boxWeight;
int cycleNumber;

void getBeltKey();
void prepareFifo();
void prepareSemaphores();
void freeResources(void);
void intHandler(int signo);
void placeBox(Box box);

int main(int argc, char **argv){
    getBeltKey();
    prepareFifo();
    prepareSemaphores();

    if (atexit(freeResources) == -1) FAILURE_EXIT(3, "Blad podczas ustawiania operacji atexit");
    if (signal(SIGINT, intHandler) == SIG_ERR) FAILURE_EXIT(3, "Blad podczas obslugi sygnalu SIGINT");

    char* nullptr = NULL;
    int cycleNumber;
    int cycles = 0;

    if (argc < 2) FAILURE_EXIT(3, "Zle parametry pracownika \n");
    boxWeight = (int) strtol(argv[1], &nullptr, 10);
    if (boxWeight < 1) FAILURE_EXIT(3, "Zle parametry pracownika \n");
    if(!(!*nullptr)) printf("Zle parametry pracownika \n");
    if(argc == 3){
        cycles = 1;
        cycleNumber = (int) strtol(argv[2], &nullptr, 10);
        if (cycleNumber < 1) FAILURE_EXIT(3, "Zle parametry pracownika \n");
        if(!(!*nullptr)) printf("Zle parametry pracownika \n");
    }

    Box box;
    box.pid = getpid();
    box.weight = boxWeight;
    while(!cycles || (--cycleNumber)){
        placeBox(box);
    }
    exit(0);
}

void placeBox(Box box) {
        struct sembuf sops;
        sops.sem_num = LOADERS;
        sops.sem_op = -1;
        sops.sem_flg = 0;
        if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Blad podczas przywlaszczenia semaforu LOADERS \n");

        sops.sem_num = BELT;
        if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Blad podczas przywlaszczenia semaforu BELT \n");

        box.time = getMicroTime();
        if(pushBelt(belt,box) == -1){
            log("Nie mozna polozyc paczki. Budze truckera\n");
            sops.sem_num = TRUCKER;
            sops.sem_op = 1;
            if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Blad podczas budzenia truckera \n");
            sops.sem_num = BELT;
            sops.sem_op = 1;
            if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Blad podczas oddawania semaforu BELT \n");
        }
        else{
        log("Polozono paczke o rozmiarze %d o czasie %ld \n",box.weight,box.time);
        sops.sem_num = BELT;
        sops.sem_op = 1;
        if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Blad podczas oddawania semaforu BELT \n");
        sops.sem_num = LOADERS;
        sops.sem_op = 1;
        if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Blad podczas oddawania semaforu LOADERS \n");
        }
        


}

void getBeltKey(){
    char *path = getenv("HOME");
    if (path == NULL) FAILURE_EXIT(3, "Blad podczas pobierania zmiennej srodowiskowej PATH \n");

    beltKey = ftok(path, PROJECT_ID);
    if (beltKey == -1) FAILURE_EXIT(3, "Blad podczas pobierania klucza pamieci wspoldzielonej \n");
}

void prepareFifo() {
    
    shmID = shmget(beltKey, 0, 0);
    if (shmID == -1) FAILURE_EXIT(3, "Blad podczas pobierania pamieci wspoldzielonej \n");

    void *tmp = shmat(shmID, NULL, 0);
    if (tmp == (void *) (-1)) FAILURE_EXIT(3, "Blad podczas dolaczania pamieci wspoldzielonej do przestrzeni adresowej procesu \n");
    belt = (Belt *) tmp;
}

void prepareSemaphores() {
    SID = semget(beltKey, 0, 0);
    if (SID == -1) FAILURE_EXIT(3, "Blad podczas pobierania semaforow \n");
}

void freeResources(void) {
    if (shmdt(belt) == -1) {log("Blad podczas odlaczania pamieci wspoldzielonej od przestrzeni adresowej\n");}
    else {log("Zasoby zostaly zwolnione\n");}
}

void intHandler(int signo) {
    exit(2);
}