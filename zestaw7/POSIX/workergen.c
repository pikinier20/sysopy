#define _GNU_SOURCE
#include "belt.h"
#define log(format, ...) {printf("Gen %d:",getpid()); printf(format, ##__VA_ARGS__);}
#define FAILURE_EXIT(code, format, ...) { log(format, ##__VA_ARGS__); exit(code);}

int main(int argc, char **argv){
    
    char* nullptr = NULL;
    int cycleNumber;
    int cycles = 0;
    if (argc < 3) FAILURE_EXIT(3, "Oczekiwane parametry: ilosc pracownikow, maksymalny rozmiar paczki, (opcjonalnie) ilosc cykli \n");
    int workerNumber = (int) strtol(argv[1], &nullptr, 10); 
    if (workerNumber < 1) FAILURE_EXIT(3, "Niepoprawna liczba pracownikow \n");
    if(!(!*nullptr)) printf("Argument pierwszy nie jest liczba \n");
    int boxWeight = (int) strtol(argv[2], &nullptr, 10); 
    if (boxWeight < 1) FAILURE_EXIT(3, "Niepoprawna liczba pracownikow \n");
    if(!(!*nullptr)) printf("Argument drugi nie jest liczba \n");
    if(argc == 4){
        cycles = 1;
        cycleNumber = (int) strtol(argv[3], &nullptr, 10);
        if (cycleNumber < 1) FAILURE_EXIT(3, "Niepoprawna liczba cykli \n");
        if(!(!*nullptr)) printf("Argument trzeci nie jest liczba \n");
    }

    int i,n,status;
    char nstring[100];
    char cyclestring[100];
    int pid[workerNumber];
    for(i = 0; i < workerNumber; i++){
        pid[i] = fork();
        if(pid[i] == 0){
            srand(time(NULL) + getpid());
            n = boxWeight;
            sprintf(nstring,"%d",n);
            if(cycles){
                sprintf(cyclestring,"%d",cycleNumber);
                if(execlp("./loader","./loader", nstring, cyclestring, NULL) == -1) FAILURE_EXIT(3,"Blad przy wykonywaniu funkcji exec \n");
                exit(0);
            }
            else{
                if(execlp("./loader","./loader", nstring, NULL) == -1) FAILURE_EXIT(3,"Blad przy wykonywaniu funkcji exec \n");
                exit(0);
            }
        }
    }
    for(i = 0; i < workerNumber; i++) waitpid(pid[i],&status,0);
}

