//
// Created by przjab98 on 27.04.19.
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include "chat.h"

int serverQueue = -1;
int clientQueue = -1;
int clientID = -1;
int working = 1;
char *queueName;

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

void communicationHandler(int signo) {
    char msg[MAX_MSG_LENGTH];
    if (mq_receive(clientQueue, msg, MAX_MSG_LENGTH, NULL) == -1)
        raise_error("Blad podczas otrzymywania wiadomosci \n");
    long type = convert_to_num(strtok(msg, ";"));
    if (type == _2ALL || type == _2ONE || type == _2FRIENDS) {
        printf("\033[1;35mKlient:\033[0m Otrzymano nowa wiadomosc: \t%s \n", strtok(NULL, ";"));
    } else if (type == STOP) {
        if (mq_close(clientQueue) == -1)
            raise_error("Blad podczas zamykania kolejki\n");
        if (mq_close(serverQueue) == -1)
            raise_error("Blad podczas zamykania kolejki \n");
        if (mq_unlink(queueName) == -1)
            raise_error("Blad podczas zamykania kolejki \n");
        else
            printf("\033[1;35mKlient:\033[0m Kolejka zostala zamknieta i usunieta \n");
    }

}

void _exit(int signo) {
    if (mq_close(clientQueue) == -1)
        raise_error("Blad podczas usuwania kolejki \n");
    if (mq_unlink(queueName) == -1)
        raise_error("Blad podczas usuwania kolejki \n");
    else
        printf("\033[1;35mKlient:\033[0m Kolejka zostala zamknieta i usunieta \n");
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

    queueName = getClientQueueName();

    if ((serverQueue = mq_open(SERVER_NAME, O_WRONLY)) == -1)
        raise_error("Nie mozna otworzyc kolejki serwera \n");
    struct mq_attr queue_attr;
    queue_attr.mq_maxmsg = MAX_QUEUE_SIZE;
    queue_attr.mq_msgsize = MAX_MSG_LENGTH;

    if ((clientQueue = mq_open(queueName, O_RDONLY | O_CREAT | O_EXCL, 0666, &queue_attr)) == -1)
        raise_error("Nie mozna otworzyc kolejki serwera \n");

    printf("\033[1;35mKlient:\033[0m ID kolejki serwera:\t%d \n", serverQueue);
    printf("\033[1;35mKlient:\033[0m Nazwa kolejki klienta:\t%s \n", queueName);
    init();


    while (working) {
        executeCommands(fdopen(STDIN_FILENO, "r"));
    }
    if (mq_close(clientQueue) == -1)
        raise_error("Nie mozna zamknac kolejki\n");
    if (mq_close(serverQueue) == -1)
        raise_error("Nie mozna zamknac kolejki\n");
    if (mq_unlink(queueName) == -1)
        raise_error("Nie mozna zamknac kolejki\n");
    else
        printf("\033[1;35mKlient:\033[0m Kolejka zostala zamknieta i usunieta \n");


    return 0;
}


void send(enum MSG_COMMAND type, char content[MAX_MSG_LENGTH]) {

    //printf("what I sent: %s \n",content);
    if (mq_send(serverQueue, content, MAX_MSG_LENGTH, cmdPriority(type)))
        raise_error("Nie mozna wyslac wiadomosci do serwera \n");
}

void init() {

    char content[MAX_MSG_LENGTH];
    sprintf(content, "%d;%d;%s", INIT, getpid(), queueName);
    //printf("im gonna send: %s \n", content);

    if (mq_send(serverQueue, content, MAX_MSG_LENGTH, cmdPriority(INIT)))
        raise_error("Nie mozna wyslac wiadomosci\n");

    char response[MAX_MSG_LENGTH];
    if (mq_receive(clientQueue, response, MAX_MSG_LENGTH, NULL) == -1)
        raise_error("Blad podczas otrzymywania wiadomosci z serwera \n");

    sscanf(response, "%d", &clientID);
    printf("\033[1;35mKlient:\033[0m Klient otrzymal ID: %d \n", clientID);
}

void echo(char content[MAX_MSG_LENGTH]) {
    char command[MAX_COMMAND_LENGTH];
    char toSend[MAX_MSG_LENGTH];

    int state = sscanf(content, "%s %s", command, toSend);
    if (state == EOF || state < 2)
        raise_error("Blad komendy ECHO \n");
    char posix_msg[MAX_MSG_LENGTH];
    sprintf(posix_msg, "%d;%d;%s", ECHO, clientID, toSend);
//    printf("im gonna send: %s \n", posix_msg);

    send(ECHO, posix_msg);

    char fromServer[MAX_MSG_LENGTH];
    if (mq_receive(clientQueue, fromServer, MAX_MSG_LENGTH, NULL) == -1)
        raise_error("Blad podczas otrzymywania wiadomosci z serwera \n");

    printf("\033[1;35mKlient:\033[0m Odpowiedz ECHO: %s \n", fromServer);

}

void list() {
    char toServer[MAX_MSG_LENGTH];
    char fromServer[MAX_MSG_LENGTH];

    sprintf(toServer, "%d;%d;%s", LIST, clientID, "");
//    printf("im gonna send: %s \n", toServer);
    send(LIST, toServer);


    if (mq_receive(clientQueue, fromServer, MAX_MSG_LENGTH, NULL) == -1)
        raise_error("Blad podczas otrzymywania listy z serwera\n");

    printf("\033[1;35mKlient:\033[0m Lista pracujacych klientow:\n %s \n", fromServer);
}

void stop() {
    char toServer[MAX_MSG_LENGTH];
    sprintf(toServer, "%d;%d;%s", STOP, clientID, "");
    working = 0;
    send(STOP, toServer);
}

void friends(char arguments[MAX_MSG_LENGTH]) {
    char command[MAX_COMMAND_LENGTH];
    char text[MAX_MSG_LENGTH];
    int args_no = sscanf(arguments, "%s %s", command, text);
    if (args_no == EOF || args_no == 0)
        raise_error("Blad podczas czytania parametrow\n");


    char toServer[MAX_MSG_LENGTH];
    sprintf(toServer, "%d;%d;%s", FRIENDS, clientID, text);
//    printf("im gonna send: %s \n", toServer);
    send(FRIENDS, toServer);
}

void add(char arguments[MAX_MSG_LENGTH]) {
    char command[MAX_COMMAND_LENGTH];
    char list[MAX_MSG_LENGTH];

    int args_no = sscanf(arguments, "%s %s", command, list);
    if (args_no == EOF || args_no == 0)
        raise_error("Blad podczas czytania argumentow\n");
    if (args_no == 1)
        raise_error("Zle parametry komendy ADD\n");

    char toServer[MAX_MSG_LENGTH];
    sprintf(toServer, "%d;%d;%s", ADD, clientID, list);

    send(ADD, toServer);
}

void del(char arguments[MAX_MSG_LENGTH]) {
    char command[MAX_COMMAND_LENGTH];
    char list[MAX_MSG_LENGTH];
    int numberOfArguments = sscanf(arguments, "%s %s", command, list);
    if (numberOfArguments == EOF || numberOfArguments == 0)
        raise_error("Blad podczas czytania argumentow \n");
    if (numberOfArguments == 1) {
        raise_error("Zle parametry komendy DEL \n");
    }
    char toServer[MAX_MSG_LENGTH];
    sprintf(toServer, "%d;%d;%s", DEL, clientID, list);
    send(DEL, toServer);
}


void _2_one(char arguments[MAX_MSG_LENGTH]) {
    char command[MAX_COMMAND_LENGTH];
    char content[MAX_MSG_LENGTH];
    int addressee;
    int args_no = sscanf(arguments, "%s %d %s", command, &addressee, content);
    if (args_no == EOF || args_no < 3)
        raise_error("Spodziewano sie wiadomosci i odbiorcy \n");
    sprintf(command, "%d %s", addressee, content);
    char toServer[MAX_MSG_LENGTH];
    sprintf(toServer, "%d;%d;%s", _2ONE, clientID, command);
    send(_2ONE, toServer);
}

void _2_friends(char arguments[MAX_MSG_LENGTH]) {
    char command[MAX_COMMAND_LENGTH], text[MAX_MSG_LENGTH];
    int args_no = sscanf(arguments, "%s %s", command, text);
    if (args_no == EOF || args_no < 2)
        raise_error("Zla ilosc argumentow dla komendy 2FRIENDS \n");
    char toServer[MAX_MSG_LENGTH];
    sprintf(toServer, "%d;%d;%s", _2FRIENDS, clientID, text);
    send(_2FRIENDS, toServer);
}

void _2_all(char arguments[MAX_MSG_LENGTH]) {
    char command[MAX_COMMAND_LENGTH], text[MAX_MSG_LENGTH];
    int args_no = sscanf(arguments, "%s %s", command, text);
    if (args_no == EOF || args_no < 2)
        raise_error("Zla iosc argumentow dla komendy 2ALL \n");
    char toServer[MAX_MSG_LENGTH];
    sprintf(toServer, "%d;%d;%s", _2ALL, clientID, text);
    send(_2ALL, toServer);
}

void executeRead(char *args) {
    char command[MAX_COMMAND_LENGTH], fileName[MAX_COMMAND_LENGTH];
    int numberOfArguments = sscanf(args, "%s %s", command, fileName);
    if (numberOfArguments == EOF || numberOfArguments < 2) {
        printf("Spodziewano sie nazwy pliku");
        return;
    }
    FILE *f = fopen(fileName, "r");
    if (f == NULL)
        raise_error("Blad podczas otwierania pliku \n");
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
