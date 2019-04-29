#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include "chat.h"

//signatures of available commands
void init(int clientPID, char msg[MAX_MSG_LENGTH]);

void echo(int clientID, char msg[MAX_MSG_LENGTH]);

void stop(int clientID);

void list(int clientID);

void friends(int clientID, char msg[MAX_MSG_LENGTH]);

void add(int clientID, char msg[MAX_MSG_LENGTH]);

void delete(int clientID, char msg[MAX_MSG_LENGTH]);

void _2all(int clientID, char msg[MAX_MSG_LENGTH]);

void _2friends(int clientID, char msg[MAX_MSG_LENGTH]);

void _2one(int clientID, char msg[MAX_MSG_LENGTH]);

//utills
void executeCommands(struct msg *msg);

void sendMessage(enum MSG_COMMAND type, char msg[MAX_MSG_LENGTH], int clientID);


int working = 1;
int serverQueueID = -1;
int workingClients = 0;

typedef struct {
    int clientQueue;
    int friends[MAX_CLIENTS];
    int curr_friends_number;
    pid_t pid;
} client_t;

client_t clients[MAX_CLIENTS];

void exitHandler(int signo) {

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].clientQueue != -1) {
            //SEND SOME STOP!
            kill(clients[i].pid, SIGINT);
        }
    }

    if (msgctl(serverQueueID, IPC_RMID, NULL) == -1)
        raise_error("Blad podczas usuwania kolejki serwera\n");
    else
        printf("\033[1;31mSerwer:\033[0m Kolejka serwera zostala usnieta \n");
    exit(EXIT_SUCCESS);
}

int main() {
    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        clients[i].clientQueue = -1;
        clients[i].curr_friends_number = 0;
    }
    //handle SIGINT -> delete queue
    struct sigaction act;
    act.sa_handler = exitHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    if ((serverQueueID = msgget(getServerQueueKey(), IPC_CREAT | 0666)) == -1)
        raise_error("Blad podczas tworzenia kolejki serwera \n");

    struct msg msgBuff;
    while (working) {
        if (msgrcv(serverQueueID, &msgBuff, MSGSZ, -COMMAND_TYPES, 0) == -1)
            raise_error("Blad podczas otrzymywania wiadomosci \n");
        executeCommands(&msgBuff);
    }

    if (msgctl(serverQueueID, IPC_RMID, NULL) == -1)
        raise_error("Blad podczas usuwania kolejki serwera\n");
    else
        printf("\033[1;31mSerwer:\033[0m Kolejka serwera zostala usunieta\n");

    return 0;
}

void executeCommands(struct msg *msg) {
    printf("\033[1;31mSerwer:\033[0m Serwer otrzymal komende \n");
    long type = msg->mType;
    if (type == STOP) {
        stop(msg->sender);
    } else if (type == INIT) {
        init(msg->sender, msg->msg);
    } else if (type == ECHO) {
        echo(msg->sender, msg->msg);
    } else if (type == FRIENDS) {
        friends(msg->sender, msg->msg);
    } else if (type == LIST) {
        list(msg->sender);
    } else if (type == _2ALL) {
        _2all(msg->sender, msg->msg);
    } else if (type == _2ONE) {
        _2one(msg->sender, msg->msg);
    } else if (type == _2FRIENDS) {
        _2friends(msg->sender, msg->msg);
    } else if (type == ADD) {
        add(msg->sender, msg->msg);
    } else if (type == DEL) {
        delete(msg->sender, msg->msg);
    } else {
        raise_error("Nie rozpoznano komendy\n");
    }
}

void sendMessage(enum MSG_COMMAND type, char msg[MAX_MSG_LENGTH], int clientID) {
    if (clientID >= MAX_CLIENTS || clientID < 0 || clients[clientID].clientQueue < 0) {
        raise_error("Blad podczas wysylania wiadomosci do klienta\n");
    }

    struct msg message;
    // sender is useless field in this case
    message.sender = -1;
    message.mType = type;
    strcpy(message.msg, msg);

    if (msgsnd(clients[clientID].clientQueue, &message, MSGSZ, IPC_NOWAIT))
        raise_error("Blad podczas wysylania wiadomosci do klienta\n");
}

void init(int clientPID, char msg[MAX_MSG_LENGTH]) {
    int id;
    for (id = 0; id < MAX_CLIENTS; id++) {
        if (clients[id].clientQueue == -1)
            break;
    }

    if (id >= MAX_CLIENTS)
        raise_error("Zbyt duzo klientow\n");

    int client_queue = -1;
    sscanf(msg, "%d", &client_queue);
    if (client_queue < 0)
        raise_error("Blad podczas otwierania kolejki klienta \n");

    clients[id].clientQueue = client_queue;
    clients[id].pid = clientPID;
    clients[id].curr_friends_number = 0;

    printf("\033[1;31mSerwer:\033[0m Zainicjalizowano nowego klienta %d \n", id);

    char toClient[MAX_MSG_LENGTH];
    sprintf(toClient, "%d", id);
    sendMessage(INIT, toClient, id);
    workingClients++;


}

void echo(int clientID, char msg[MAX_MSG_LENGTH]) {
    printf("\033[1;31mSerwer:\033[0m ECHO dla klienta %d: %s\n", clientID, msg);
    char response[MAX_MSG_LENGTH];
    char date[64];
    FILE *f = popen("date", "r");
    int check = fread(date, sizeof(char), 31, f);
    if (check == EOF)
        raise_error("Blad podczas odczytywania daty \n");
    pclose(f);
    sprintf(response, "%s Data: %s", msg, date);
    //printf("what i sent back: %s \n", response);
    sendMessage(ECHO, response, clientID);
}

void stop(int clientID) {
    if (clientID >= 0 && clientID < MAX_CLIENTS) {
        clients[clientID].clientQueue = -1;
        clients[clientID].curr_friends_number = 0;
        for (int i = 0; i < MAX_CLIENTS; i++)
            clients[clientID].friends[i] = -1;
        workingClients--;
        if (workingClients == 0) {
            kill(getpid(), SIGINT);
        }
    }


}

void list(int clientID) {
    printf("\033[1;31mSerwer:\033[0m LIST dla klienta: %d \n", clientID);
    char response[MAX_MSG_LENGTH], buf[MAX_MSG_LENGTH];
    strcpy(response, "");
    int i = 0;
    for (; i < MAX_CLIENTS; i++) {
        if (clients[i].clientQueue >= 0) {
            sprintf(buf, "ID: %d  ID kolejki: %d \n", i, clients[i].clientQueue);
            strcat(response, buf);
        }
    }
    sendMessage(LIST, response, clientID);
}

void makeFriendsList(int clientID, char friends[MAX_MSG_LENGTH]) {
    char *friend = strtok(friends, SPLITTER);

    while (friend && clients[clientID].curr_friends_number < MAX_CLIENTS) {
        //to improve:
        int f = convert_to_num(friend);
        if (f < 0 || f >= MAX_CLIENTS || f < 0 || clientID == f) {
            printf("\033[1;31mSerwer:\033[0m friend: %s cannot be added (wrong type or value)\n", friend);
        }

        int found = 0;
        int i = 0;

        //friends cannot be repeated
        for (; i < clients[clientID].curr_friends_number; i++)
            if (f == clients[clientID].friends[i]) {
                found = 1;
            }
        if (!found) {
            clients[clientID].friends[clients[clientID].curr_friends_number++] = f;
            printf("\033[1;31mSerwer:\033[0m Przyjaciel %d zostal dodany \n", f);
        } else {
            printf("\033[1;31mServer:\033[0m %d juz jest przyjacielem\n", f);
        }
        friend = strtok(NULL, SPLITTER);
    }
}

void friends(int clientID, char msg[MAX_MSG_LENGTH]) {
    if (clientID >= MAX_CLIENTS || clientID < 0 || clients[clientID].clientQueue < 0) {
        raise_error("Niepoprawny klient\n");
    }

    printf("\033[1;31mSerwer:\033[0m Zmieniono liste przyjaciol klienta\n");

    char friends[MAX_MSG_LENGTH];
    int state = sscanf(msg, "%s", friends);

    if (state != 1)
        raise_error("Blad podczas zmiany listy przyjaciol \n");

    //override
    clients[clientID].curr_friends_number = 0;
    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        clients[clientID].friends[i] = -1;
    }

    makeFriendsList(clientID, friends);
}

void add(int clientID, char msg[MAX_MSG_LENGTH]) {
    if (clientID >= MAX_CLIENTS || clientID < 0 || clients[clientID].clientQueue < 0) {
        raise_error("Niepoprawny klient \n");
    }

    char friends[MAX_MSG_LENGTH];
    int state = sscanf(msg, "%s", friends);

    if (state != 1)
        raise_error("Blad podczas zmiany listy przyjaciol\n");

    makeFriendsList(clientID, friends);
}

void delete(int clientID, char msg[MAX_MSG_LENGTH]) {
    if (clientID >= MAX_CLIENTS || clientID < 0 || clients[clientID].clientQueue < 0) {
        raise_error("Niepoprawny klient\n");
    }

    char list[MAX_MSG_LENGTH];
    int num = sscanf(msg, "%s", list);
    char *elem = NULL;
    int id = -1;
    int i = 0;

    if (num == EOF || num == 0) {
        raise_error("Blad podczas wczytywania danych \n");
    } else if (num == 1) {
        elem = strtok(list, SPLITTER);
        while (elem != NULL && (clients[clientID].curr_friends_number) > 0) {
            id = (int) strtol(elem, NULL, 10);
            i = 0;
            for (; i < (clients[clientID].curr_friends_number); i++)
                if (id == clients[clientID].friends[i]) break;
            if (i >= (clients[clientID].curr_friends_number))id = -1;
            if (id < MAX_CLIENTS && id >= 0 && id != clientID) {
                clients[clientID].friends[i] = clients[clientID].friends[(clients[clientID].curr_friends_number) - 1];
                (clients[clientID].curr_friends_number)--;
            }
            elem = strtok(NULL, SPLITTER);
        }
    }

    printf("\033[1;31mSerwer:\033[0m Lista przyjaciol po usunieciu: \n");
    int j;
    for (j = 0; j < clients[clientID].curr_friends_number; j++) {
        printf("\t ID: %d\n", clients[clientID].friends[j]);
    }
}

void _2all(int clientID, char msg[MAX_MSG_LENGTH]) {
    printf("Serwer wysle wiadomosc do wszystkich klientow\n");

    char response[MAX_MSG_LENGTH];
    char date[32];
    FILE *p = popen("date", "r");
    int check = fread(date, sizeof(char), 31, p);
    if (check == EOF)
        raise_error("Blad podczas czytania danych \n");
    pclose(p);

    sprintf(response, "Wiadomosc: %s od: %d Data: %s \n", msg, clientID, date);

    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].clientQueue == -1 || i == clientID)
            continue;
        sendMessage(_2ALL, response, i);
        kill(clients[i].pid, SIGRTMIN);
    }
}

void _2one(int clientID, char msg[MAX_MSG_LENGTH]) {
    printf("\033[1;31mSerwer:\033[0m Serwer wysle wiadomosc do jednego klienta \n");
    char date[32];

    char content[MAX_MSG_LENGTH];
    char response[MAX_MSG_LENGTH];

    int addressee;
    FILE *f = popen("date", "r");
    int check = fread(date, sizeof(char), 31, f);
    if (check == EOF)
        raise_error("Blad podczas czytania danych \n");
    pclose(f);
    int x = sscanf(msg, "%i %s", &addressee, content);
    if (x != 2)
        raise_error("Blad podczas wczytywania parametrow wiadomosci\n");
    sprintf(response, "Wiadomosc %s od: %d Data: %s \n", content, clientID, date);

    if (addressee >= MAX_CLIENTS || addressee < 0 || clients[addressee].clientQueue < 0) {
        raise_error("Zly adres \n");
    }

    if (addressee == clientID) {
        printf("\033[1;31mSerwer:\033[0m Nie mozna wyslac wiadomosci do samego siebie - uzyj ECHO \n");
        return;
    }

    sendMessage(_2ONE, response, addressee);
    kill(clients[addressee].pid, SIGRTMIN);


}

void _2friends(int clientID, char msg[MAX_MSG_LENGTH]) {
    printf("\033[1;31mSerwer:\033[0m Serwer wysle wiadomosc do przyjaciol klienta \n");
    char response[MAX_MSG_LENGTH];
    char date[64];
    FILE *f = popen("date", "r");
    int check = fread(date, sizeof(char), 31, f);
    if (check == EOF)
        raise_error("Blad podczas czytania daty \n");
    pclose(f);
    sprintf(response, "Wiadomosc: %s od: %d Data: %s\n", msg, clientID, date);
    int i;
    for (i= 0 ; i < clients[clientID].curr_friends_number; i++) {
        int addressee = clients[clientID].friends[i];
        if (clients[clientID].friends[i] == -1)
            continue;
        if (addressee >= MAX_CLIENTS || addressee < 0 || clients[addressee].clientQueue < 0) {
            raise_error("Zly adres klienta \n");
        }
        printf("%d\t", addressee);
        sendMessage(_2FRIENDS, response, addressee);
        kill(clients[addressee].pid, SIGRTMIN);

    }
}


