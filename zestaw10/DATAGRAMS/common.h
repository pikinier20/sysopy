//
// Created by przjab98 on 31.05.19.
//

#ifndef SOCKETS_COMMON_H
#define SOCKETS_COMMON_H

#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#define MAX_PATH 108
#define CLIENT_MAX 12
typedef enum message_type {
    REGISTER = 0,
    UNREGISTER = 1,
    SUCCESS = 2,
    FAILSIZE = 3,
    WRONGNAME = 4,
    REQUEST = 5,
    RESULT = 6,
    PING = 7,
    PONG = 8,
    END = 9
} message_type;

typedef enum connect_type {
    LOCAL = 0,
    WEB = 1
} connect_type;

typedef struct message_t {
    enum message_type message_type;
    char name[64];
    enum connect_type connect_type;
    char value[10240];

} message_t;

typedef struct request_t{
    char text[10240];
    int ID;
} request_t;

typedef struct Client {
    char *name;
    int active_counter;
    enum connect_type connect_type;
    struct sockaddr* sockaddr;
    int reserved;
    socklen_t socklen;
} Client;


void raise_error(char* message){
    fprintf(stderr, "%s :: %s \n", message, strerror(errno));
    exit(1);
}

#endif //SOCKETS_COMMON_H
