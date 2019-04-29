//
// Created by przjab98 on 27.04.19.
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include "chat.h"

int serverQueue = -1;
int clientQueue = -1;
int clientID = -1;
int working = 1;

//commands handlers
void send(enum MSG_COMMAND type, char content[MAX_MSG_LENGTH]);

void executeRead(char *args);

int executeCommands(FILE *file);

void echo(char content[MAX_MSG_LENGTH]);

void list();

void stop();

void friends(char arguments[MAX_MSG_LENGTH]);

void add(char arguments[MAX_MSG_LENGTH]);

void del(char arguments[MAX_MSG_LENGTH]);

void _2_one(char arguments[MAX_MSG_LENGTH]);

void _2_friends(char arguments[MAX_MSG_LENGTH]);

void _2_all(char arguments[MAX_MSG_LENGTH]);

void init();

//signal handlers
void communicationHandler(int signo) {
    struct msg msg;
    if (msgrcv(clientQueue, &msg, MSGSZ, -COMMAND_TYPES, 0) == -1)
        raise_error("Wystapil blad podczas otrzymywania komunikatu z kolejki\n");

    if (msg.mType == _2ALL || msg.mType == _2ONE || msg.mType == _2FRIENDS) {
        printf("Otrzymano wiadomosc %s \n", msg.msg);
    } else if (msg.mType == STOP) {
        if (msgctl(clientQueue, IPC_RMID, NULL) == -1)
            raise_error("Blad podczas usuwania kolejki \n");
        exit(EXIT_SUCCESS);
    }

}

void _exit(int signo) {
    if (msgctl(clientQueue, IPC_RMID, NULL) == -1)
        raise_error("Blad podczas usuwania kolejki \n");
    else {
        printf("\033[1;35mKlient:\033[0m kolejka zostala usunieta ");
    }
    stop();
    exit(EXIT_SUCCESS);
}


int main() {
    struct sigaction act;
    act.sa_handler = communicationHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGRTMIN, &act, NULL);

    signal(SIGINT, _exit);

    if ((serverQueue = msgget(getServerQueueKey(), 0)) == -1)
        raise_error("Blad podczas otwierania kolejki \n");
    if ((clientQueue = msgget(getClientQueueKey(), IPC_CREAT | IPC_EXCL | 0666)) == -1)
        raise_error("Blad podczas otwierania kolejki \n");

    init();
    printf("\033[1;35mKlient:\033[0m Kolejka serwera ID:\t%d \n", serverQueue);
    while (working) {
        executeCommands(fdopen(STDIN_FILENO, "r"));
    }
    if (msgctl(clientQueue, IPC_RMID, NULL) == -1)
        raise_error("Blad podczas usuwania kolejki \n");
    else
        printf("\033[1;35mKlient:\033[0m Kolejka zostala usunieta \n");

    return 0;
}


void send(enum MSG_COMMAND type, char content[MAX_MSG_LENGTH]) {
    struct msg msg;
    msg.mType = type;
    strcpy(msg.msg, content);
    msg.sender = clientID;
    //printf("what I sent: %s \n", msg.msg);
    if (msgsnd(serverQueue, &msg, MSGSZ, IPC_NOWAIT) == -1)
        raise_error("Blad podczas wysylania wiadomosci do serwera \n");
}

void init() {
    struct msg msg;
    char content[MAX_MSG_LENGTH];
    msg.mType = INIT;
    sprintf(content, "%i", clientQueue);
    strcpy(msg.msg, content);
    msg.sender = getpid();
    if (msgsnd(serverQueue, &msg, MSGSZ, IPC_NOWAIT))
        raise_error("Blad podczas wysylania wiadomosci do serwera  \n");

    if (msgrcv(clientQueue, &msg, MSGSZ, -COMMAND_TYPES, 0) == -1)
        raise_error("Blad podczas otrzymywania wiadomosci do serwera  \n");

    if (msg.mType != INIT)
        raise_error("Niepoprawny typ odpowiedzi \n");
    sscanf(msg.msg, "%d", &clientID);
    printf("\033[1;35mKlient:\033[0m Kient otrzymal ID: %d \n", clientID);
}

void echo(char content[MAX_MSG_LENGTH]) {
    char command[MAX_COMMAND_LENGTH];
    char toSend[MAX_MSG_LENGTH];

    int state = sscanf(content, "%s %s", command, toSend);
    if (state == EOF || state < 2)
        raise_error("Blad komendy ECHO - niepoprawne parametry\n");
    send(ECHO, toSend);
    struct msg msg;
    if (msgrcv(clientQueue, &msg, MSGSZ, -COMMAND_TYPES, 0) == -1)
        raise_error("Blad podczas otrzymywania wiadomosci do serwera \n");

    if (msg.mType != ECHO)
        raise_error("Niepoprawny typ odpowiedzi ");

    printf("\033[1;35mKlient:\033[0m Odpowiedz ECHO: %s \n", msg.msg);

}

void list() {
    send(LIST, "");
    struct msg msg;
    if (msgrcv(clientQueue, &msg, MSGSZ, -COMMAND_TYPES, 0) == -1)
        raise_error("Blad komendy LIST \n");

    if (msg.mType != LIST)
        raise_error("Niepoprawny typ odpowiedzi \n");

    printf("\033[1;35mKlient:\033[0m Lista pracujacych klientow: \n %s \n", msg.msg);
}

void stop() {
    working = 0;
    send(STOP, "");
}

void friends(char arguments[MAX_MSG_LENGTH]) {
    char command[MAX_COMMAND_LENGTH];
    char text[MAX_MSG_LENGTH];
    int args_no = sscanf(arguments, "%s %s", command, text);
    if (args_no == EOF || args_no == 0)
        raise_error("Blad komendy FRIENDS \n");
    send(FRIENDS, text);
}

void add(char arguments[MAX_MSG_LENGTH]) {
    char command[MAX_COMMAND_LENGTH];
    char list[MAX_MSG_LENGTH];

    int args_no = sscanf(arguments, "%s %s", command, list);
    if (args_no == EOF || args_no == 0)
        raise_error("Blad komendy ADD \n");
    if (args_no == 1)
        raise_error("Zle parametry komendy ADD \n");

    send(ADD, list);
}

void del(char arguments[MAX_MSG_LENGTH]) {
    char command[MAX_COMMAND_LENGTH];
    char list[MAX_MSG_LENGTH];
    int numberOfArguments = sscanf(arguments, "%s %s", command, list);
    if (numberOfArguments == EOF || numberOfArguments == 0)
        raise_error("Blad komendy DEL \n");
    if (numberOfArguments == 1) {
        raise_error("Zle parametry komendy DEL \n");
    }
    send(DEL, list);
}

void _2_one(char arguments[MAX_MSG_LENGTH]) {
    char command[MAX_COMMAND_LENGTH];
    char content[MAX_MSG_LENGTH];
    int addressee;
    int args_no = sscanf(arguments, "%s %d %s", command, &addressee, content);
    if (args_no == EOF || args_no < 3)
        raise_error("Zle parametry komendy 2ONE \n");
    sprintf(command, "%d %s", addressee, content);
    send(_2ONE, command);
}

void _2_friends(char arguments[MAX_MSG_LENGTH]) {
    char command[MAX_COMMAND_LENGTH], text[MAX_MSG_LENGTH];
    int args_no = sscanf(arguments, "%s %s", command, text);
    if (args_no == EOF || args_no < 2)
        raise_error("Zle parametry komendy 2FRIENDS \n");
    send(_2FRIENDS, text);
}

void _2_all(char arguments[MAX_MSG_LENGTH]) {
    char command[MAX_COMMAND_LENGTH], text[MAX_MSG_LENGTH];
    int args_no = sscanf(arguments, "%s %s", command, text);
    if (args_no == EOF || args_no < 2)
        raise_error("Zle parametry komendy 2ALL \n");
    send(_2ALL, text);
}

void executeRead(char *args) {
    char command[MAX_COMMAND_LENGTH], fileName[MAX_COMMAND_LENGTH];
    int numberOfArguments = sscanf(args, "%s %s", command, fileName);
    if (numberOfArguments == EOF || numberOfArguments < 2) {
        printf("\033[1;35mKlient:\033[0m Spodziewano sie nazwy pliku");
        return;
    }
    FILE *f = fopen(fileName, "r");
    if (f == NULL)
        raise_error("Blad otwierania pliku komend \n");
    while (executeCommands(f) != EOF);
    fclose(f);
}

int executeCommands(FILE *file) {
    char args[MAX_COMMAND_LENGTH];
    char command[MAX_MSG_LENGTH];

    if (fgets(args, MAX_COMMAND_LENGTH * sizeof(char), file) == NULL)
        return EOF;

    //printf("executing: %s \n", args);
    int sscanf_state = sscanf(args, "%s", command);

    if (sscanf_state == EOF || sscanf_state == 0)
        return 0;
    if (!strcmp(command, "ECHO")) {
        echo(args);
    } else if (!strcmp(command, "LIST")) {
        list();
    } else if (!strcmp(command, "STOP")) {
        stop();
    } else if (!strcmp(command, "READ")) {
        executeRead(args);
    } else if (!strcmp(command, "FRIENDS")) {
        friends(args);
    } else if (!strcmp(command, "ADD")) {
        add(args);
    } else if (!strcmp(command, "DEL")) {
        del(args);
    } else if (!strcmp(command, "2ONE")) {
        _2_one(args);
    } else if (!strcmp(command, "2FRIENDS")) {
        _2_friends(args);
    } else if (!strcmp(command, "2ALL")) {
        _2_all(args);
    } else {
        fprintf(stderr, "Nie rozpoznano komendy \n");
        return 1;
    }
    return 0;

}
