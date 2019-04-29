//
// Created by przjab98 on 27.04.19.
//

#ifndef SIMPLE_CHAT_CHAT_H
#define SIMPLE_CHAT_CHAT_H


#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stddef.h>
#include <string.h>

#define LETTER 'a'
#define MAX_CLIENTS 1000
#define MAX_MSG_LENGTH 500
#define MAX_COMMAND_LENGTH 500
#define SPLITTER ":"
#define COMMAND_TYPES 11

struct msg {
    long mType;
    pid_t sender;
    char msg[MAX_MSG_LENGTH];
};

#define MSGSZ sizeof(struct msg)

enum MSG_COMMAND {
    STOP = 1L,
    FRIENDS = 2L,
    INIT = 3L,
    LIST = 4L,
    ADD = 5L,
    DEL = 6L,
    ECHO = 7L,
    _2ONE = 8L,
    _2FRIENDS = 9L,
    _2ALL = 10L,
};

key_t getServerQueueKey();

key_t getClientQueueKey();

void raise_error(char *msg);

int convert_to_num(char *given_string);


#endif //SIMPLE_CHAT_CHAT_H

