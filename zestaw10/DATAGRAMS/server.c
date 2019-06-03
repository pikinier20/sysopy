//
// Created by przjab98 on 31.05.19.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "common.h"

size_t get_file_size(const char *file_name) {
    int fd;
    if ((fd = open(file_name, O_RDONLY)) == -1) {
        fprintf(stderr, "Unable to open file for size \n");
        return (size_t) -1;
    }
    struct stat stats;
    fstat(fd, &stats);
    size_t size = (size_t) stats.st_size;
    close(fd);
    return size;
}

size_t read_whole_file(const char *file_name, char *buffer) {
    size_t size = get_file_size(file_name);
    if (size == -1) {
        return size;
    }
    FILE *file;
    if ((file = fopen(file_name, "r")) == NULL) {
        fprintf(stderr, "Unable to open file \n");
        return (size_t) -1;
    }
    size_t read_size;
    if ((read_size = fread(buffer, sizeof(char), size, file)) != size) {
        fprintf(stderr, "Unable to read file\n");
        return (size_t) -1;
    }
    fclose(file);
    return read_size;
}

void handle_signal(int);

void init(char *, char *);

void handle_message(int);

void register_client(int socket, message_t msg, struct sockaddr *sockaddr, socklen_t socklen);

void unregister_client(char *);

void clean();

void *ping_clients(void *);

void *handler_terminal(void *);

void delete_client(int);

int in(void *const a, void *const pbase, size_t total_elems, size_t size, __compar_fn_t cmp) {
    char *base_ptr = (char *) pbase;
    if (total_elems > 0) {
        for (int i = 0; i < total_elems; ++i) {
            if ((*cmp)(a, (void *) (base_ptr + i * size)) == 0) return i;
        }
    }
    return -1;
}

int cmp_name(char *name, Client *client) {
    return strcmp(name, client->name);
}

int web_socket;
int local_socket;
int epoll;
char *local_path;
int id = 0;
pthread_t ping;
pthread_t command;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
Client clients[CLIENT_MAX];
int clients_amount = 0;


int main(int argc, char *argv[]) {
    srand(time(NULL));
    if (argc != 3)
        raise_error("\nUsage: %s <port number> <unix path>\n");
    if (atexit(clean) == -1)
        raise_error(" Could not set AtExit\n");

    init(argv[1], argv[2]);

    struct epoll_event event;
    int x = 1;
    while (x) {
        if (epoll_wait(epoll, &event, 1, -1) == -1)
            raise_error(" epoll_wait failed\n");
        handle_message(event.data.fd);

    }

}

void *ping_clients(void *arg) {
    uint8_t message_type = PING;
    printf("PING \n");
    int x = 1;
    while (x) {
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < clients_amount; ++i) {
            if (clients[i].active_counter != 0) {
                printf("Client \"%s\" is not responding. Removing...\n", clients[i].name);
                delete_client(i--);
            } else {
                int socket = clients[i].connect_type == WEB ? web_socket : local_socket;
                if (sendto(socket, &message_type, 1, 0, clients[i].sockaddr, clients[i].socklen) != 1)
                    raise_error(" Could not send ping to client");
                clients[i].active_counter++;
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(5);
    }
    return NULL;
}

void *handler_terminal(void *arg) {
    request_t req;
    int true = 1;
    while (true) {
        char buffer[256];
        printf("Enter command: \n");
        fgets(buffer, 256, stdin);
        memset(req.text, 0, sizeof(req.text));
        sscanf(buffer, "%s", buffer);
        uint8_t message_type = REQUEST;
        int status = read_whole_file(buffer, req.text);
        if (strlen(req.text) <= 0) {
            printf("Empty file provided \n");
            continue;
        }
        if (status < 0) {
            printf("Wrong file format \n");
            continue;
        }
        id++;
        printf("Request ID: %d \n", id);
        req.ID = id;
        // printf("TO SEND: %s \n", req.text);
        int i = 0;
        int sent = 0;
        for (i = 0; i < clients_amount; i++) {
            if (clients[i].reserved == 0) {
                printf("Request sent to %s \n", clients[i].name);

                int socket = clients[i].connect_type == WEB ? web_socket : local_socket;
                if (sendto(socket, &message_type, 1, 0, clients[i].sockaddr, clients[i].socklen) != 1) {
                    raise_error("cannot sendto");
                }
                if (sendto(socket, &req, sizeof(request_t), 0, clients[i].sockaddr, clients[i].socklen) !=
                    sizeof(request_t)) {
                    raise_error("cannot sendto");
                }
                sent = 1;
                clients[i].reserved++;
                break;

            }
        }
        if (!sent) {
            i = 0;
            if (clients[i].reserved > -1) {
                int socket = clients[i].connect_type == WEB ? web_socket : local_socket;
                if (sendto(socket, &message_type, 1, 0, clients[i].sockaddr, clients[i].socklen) != 1) {
                    raise_error("cannot sendto");
                }
                if (sendto(socket, &req, sizeof(request_t), 0, clients[i].sockaddr, clients[i].socklen) !=
                    sizeof(request_t)) {
                    raise_error("cannot sendto");
                }
                clients[i].reserved++;
            }

        }

    }
    return NULL;
}

void handle_message(int socket) {
    struct sockaddr *sockaddr = malloc(sizeof(struct sockaddr));
    socklen_t socklen = sizeof(struct sockaddr);
    message_t msg;

    if (recvfrom(socket, &msg, sizeof(message_t), 0, sockaddr, &socklen) != sizeof(message_t))
        raise_error("Could not receive new message\n");
    //printf("RECEIVED msg from socket: %d \n", socket);
    // printf("MSG: %d, %s, %d, socket: %d \n", msg.connect_type, msg.name, msg.message_type, msg.fd);
    switch (msg.message_type) {
        case REGISTER: {
            register_client(socket, msg, sockaddr, socklen);
            break;
        }
        case UNREGISTER: {
            unregister_client(msg.name);
            break;
        }
        case RESULT: {
            printf("Incoming result...\n");
            int i;
            for (i = 0; i < clients_amount; i++) {
                if (strcmp(clients[i].name, msg.name) == 0) {
                    clients[i].reserved--;
                    clients[i].active_counter = 0;
                }
            }

            //printf("RES \n");
            printf("Result: %s \n", msg.value);
            printf("from: %s \n", msg.name);


            break;
        }
        case PONG: {
            pthread_mutex_lock(&mutex);
            int i = in(msg.name, clients, (size_t) clients_amount, sizeof(Client), (__compar_fn_t) cmp_name);
            if (i >= 0) clients[i].active_counter = clients[i].active_counter == 0 ? 0 : clients[i].active_counter - 1;
            pthread_mutex_unlock(&mutex);
            break;
        }
        default:
            //printf("Unknown message type\n");
            break;
    }

}

void register_client(int socket, message_t msg, struct sockaddr *sockaddr, socklen_t socklen) {
    uint8_t message_type;
    pthread_mutex_lock(&mutex);
    if (clients_amount == CLIENT_MAX) {
        message_type = FAILSIZE;
        if (sendto(socket, &message_type, 1, 0, sockaddr, socklen) != 1)
            raise_error(" Could not write FAILSIZE message to client \"%s\"\n");
        free(sockaddr);
    } else {
        int exists = in(msg.name, clients, (size_t) clients_amount, sizeof(Client), (__compar_fn_t) cmp_name);
        if (exists != -1) {
            message_type = WRONGNAME;
            if (sendto(socket, &message_type, 1, 0, sockaddr, socklen) != 1)
                raise_error(" Could not write WRONGNAME message to client \"%s\"\n");
            free(sockaddr);
        } else {
            printf("REGISTERING \n");
            clients[clients_amount].name = malloc(strlen(msg.name) + 1);
            clients[clients_amount].connect_type = msg.connect_type;
            clients[clients_amount].active_counter = 0;
            clients[clients_amount].reserved = 0;
            clients[clients_amount].socklen = socklen;
            clients[clients_amount].sockaddr = malloc(sizeof(struct sockaddr_un));
            memcpy(clients[clients_amount].sockaddr, sockaddr, socklen);
            message_type = SUCCESS;

            strcpy(clients[clients_amount++].name, msg.name);
            if (sendto(socket, &message_type, 1, 0, sockaddr, socklen) != 1)
                raise_error(" Could not write SUCCESS message to client \"%s\"\n");
        }
    }
    pthread_mutex_unlock(&mutex);
}

void unregister_client(char *client_name) {
    pthread_mutex_lock(&mutex);
    int i = in(client_name, clients, (size_t) clients_amount, sizeof(Client), (__compar_fn_t) cmp_name);
    if (i >= 0) {
        delete_client(i);
        printf("Client \"%s\" unregistered\n", client_name);
    }
    pthread_mutex_unlock(&mutex);
}

void delete_client(int i) {
    free(clients[i].sockaddr);
    free(clients[i].name);

    clients_amount--;
    for (int j = i; j < clients_amount; ++j)
        clients[j] = clients[j + 1];

}

void handle_signal(int signo) {
    printf("\nSIGINT\n");
    exit(1);
}

void init(char *port, char *path) {

    struct sigaction act;
    act.sa_handler = handle_signal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    int i;
    for (i = 0; i < CLIENT_MAX; i++) {
        clients[i].reserved = -1;
    }

    uint16_t port_num = (uint16_t) atoi(port);
    local_path = path;

    //******************** WEB ***********************
    struct sockaddr_in web_address;
    memset(&web_address, 0, sizeof(struct sockaddr_in));
    web_address.sin_family = AF_INET;
    web_address.sin_addr.s_addr = htonl(INADDR_ANY);
    web_address.sin_port = htons(port_num);

    if ((web_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        raise_error(" Could not create web socket\n");

    if (bind(web_socket, (const struct sockaddr *) &web_address, sizeof(web_address)))
        raise_error(" Could not bind web socket\n");

    //******************* LOCAL **********************
    struct sockaddr_un local_address;
    local_address.sun_family = AF_UNIX;
    sprintf(local_address.sun_path, "%s", local_path);

    if ((local_socket = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
        raise_error(" Could not create local socket\n");

    if (bind(local_socket, (const struct sockaddr *) &local_address, sizeof(local_address)))
        raise_error(" Could not bind local socket\n");

    //****************** EPOLL ***********************
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;
    if ((epoll = epoll_create1(0)) == -1)
        raise_error(" Could not create epoll\n");
    event.data.fd = web_socket;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, web_socket, &event) == -1)
        raise_error(" Could not add Web Socket to epoll\n");
    event.data.fd = local_socket;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, local_socket, &event) == -1)
        raise_error(" Could not add Local Socket to epoll\n");


    if (pthread_create(&ping, NULL, ping_clients, NULL) != 0)
        raise_error(" Could not create Pinger Thread");
    if (pthread_create(&command, NULL, handler_terminal, NULL) != 0)
        raise_error(" Could not create Commander Thread");
}

void clean() {
    printf("CLEANUP \n");
    pthread_cancel(ping);
    pthread_cancel(command);
    int i;
    for (i = 0; i < clients_amount; i++) {
        if (clients[i].reserved >= 0) {
            printf("Request sent to %s \n", clients[i].name);
            uint8_t message_type = END;
            int socket = clients[i].connect_type == WEB ? web_socket : local_socket;
            if (sendto(socket, &message_type, 1, 0, clients[i].sockaddr, clients[i].socklen) != 1) {
                raise_error("cannot sendto");
            }
        }
    }

    if (close(web_socket) == -1)
        fprintf(stderr, " Could not close Web Socket\n");
    if (close(local_socket) == -1)
        fprintf(stderr, " Could not close Local Socket\n");
    if (unlink(local_path) == -1)
        fprintf(stderr, " Could not unlink Unix Path\n");
    if (close(epoll) == -1)
        fprintf(stderr, " Could not close epoll\n");
}
