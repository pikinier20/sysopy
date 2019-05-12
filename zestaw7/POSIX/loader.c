#define _GNU_SOURCE
#include "belt.h"
#define log(format, ...) {printf("\033[1;31mLoader %d: \033[0m",getpid()); printf(format, ##__VA_ARGS__);}
#define FAILURE_EXIT(code, format, ...) { log(format, ##__VA_ARGS__); exit(code);}

Belt *belt = NULL;
key_t beltKey;
sigset_t fullMask;
int shmID = -1;
int SID = -1;
int boxWeight;
int cycleNumber;
int queue;
sem_t *trucker_sem;
sem_t *loaders_sem;
sem_t *belt_sem;

void getBeltKey();
void prepareFifo();
void prepareSemaphores();
void freeResources(void);
void intHandler(int signo);
void placeBox(Box box);
void writePid();

int main(int argc, char **argv){
    if (atexit(freeResources) == -1) FAILURE_EXIT(3, "Blad podczas ustawiania operacji atexit");
    if (signal(SIGINT, intHandler) == SIG_ERR) FAILURE_EXIT(3, "Blad podczas obslugi sygnalu SIGINT");

    getBeltKey();
    prepareFifo();
    prepareSemaphores();
    writePid();



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
        if (sem_wait(loaders_sem) == -1) FAILURE_EXIT(3, "Blad podczas przywlaszczenia semaforu LOADERS \n");

        if (sem_wait(belt_sem) == -1) FAILURE_EXIT(3, "Blad podczas przywlaszczenia semaforu BELT \n");

        box.time = getMicroTime();
        if(pushBelt(belt,box) == -1){
            log("Nie mozna polozyc paczki. Czekam na zwolnienie tasmy \n");

            if (sem_post(trucker_sem) == -1) FAILURE_EXIT(3, "Blad podczas budzenia truckera \n");

            usleep(10000);

            if (sem_post(belt_sem) == -1) FAILURE_EXIT(3, "Blad podczas oddawania semaforu BELT \n");

            if (sem_wait(trucker_sem) == -1) FAILURE_EXIT(3, "Blad podczas odbierania semaforu TRUCKER \n");

            if (sem_wait(belt_sem) == -1) FAILURE_EXIT(3, "Blad podczas przywlaszczenia semaforu BELT \n");
            box.time = getMicroTime();
            if(pushBelt(belt,box) == -1) FAILURE_EXIT(3, "Blad podczas umieszczania paczki na tasmie \n");
        }
        if (sem_post(belt_sem) == -1) FAILURE_EXIT(3, "Blad podczas oddawania semaforu BELT \n");

        if (sem_post(loaders_sem) == -1) FAILURE_EXIT(3, "Blad podczas oddawania semaforu LOADERS \n");
        usleep(10000);
        log("Polozono paczke o rozmiarze %d o czasie %ld \n",box.weight,box.time);
        


}

void getBeltKey(){
    char *path = getenv("HOME");
    if (path == NULL) FAILURE_EXIT(3, "Blad podczas pobierania zmiennej srodowiskowej PATH \n");

    beltKey = ftok(path, PROJECT_ID);
    if (beltKey == -1) FAILURE_EXIT(3, "Blad podczas pobierania klucza pamieci wspoldzielonej \n");
}

void prepareFifo() {
    int shmID = shm_open("belt_memory", O_RDWR, 0666);
    if (shmID == -1) FAILURE_EXIT(3, "Blad podczas dolaczania pamieci wspoldzielonej do przestrzeni adresowej procesu \n");
    void *tmp = mmap(NULL, sizeof(Belt), PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
    if (tmp == (void *) (-1)) FAILURE_EXIT(3, "Blad podczas dolaczania pamieci wspoldzielonej do przestrzeni adresowej procesu \n");
    belt = (Belt *) tmp;
}

void prepareSemaphores() {
    trucker_sem = sem_open("trucker_sem", O_RDWR);
    if (trucker_sem == SEM_FAILED) FAILURE_EXIT(3, "Blad podczas pobierania semaforow \n");
    loaders_sem = sem_open("loaders_sem", O_RDWR);
    if (loaders_sem == SEM_FAILED) FAILURE_EXIT(3, "Blad podczas pobierania semaforow \n");
    belt_sem = sem_open("belt_sem", O_RDWR);
    if (belt_sem == SEM_FAILED) FAILURE_EXIT(3, "Blad podczas pobierania semaforow \n");
}

void writePid(){
    if ((queue = mq_open("/pidQueue", O_WRONLY)) == -1)
    FAILURE_EXIT(3,"Nie mozna otworzyc kolejki PID \n");

    char pid[10];
    sprintf(pid,"%d",getpid());
    if (mq_send(queue, pid, 10, 1) != 0)
    {
        FAILURE_EXIT(3,"Nie mozna wyslac PID\n");  
    }  

    if (mq_close(queue) == -1)
        FAILURE_EXIT(3,"Nie mozna zamknac kolejki\n");

}

void freeResources(void) {

    if(belt != NULL)
    if (munmap(belt, sizeof(Belt)) == -1) {log("Blad podczas odlaczania pamieci wspoldzielonej od przestrzeni adresowej\n");}
    if(trucker_sem != NULL)
    if (sem_close(trucker_sem) == -1) {log("Blad podczas zamykania semaforu \n");}
    if(loaders_sem != NULL)
    if (sem_close(loaders_sem) == -1) {log("Blad podczas zamykania semaforu \n");}
    if(belt_sem != NULL)
    if (sem_close(belt_sem) == -1) {log("Blad podczas zamykania semaforu \n");}

    log("Zasoby zostaly zwolnione\n");
}

void intHandler(int signo) {
    exit(2);
}