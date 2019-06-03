#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include "common.h"
#include <netinet/in.h>
#include <sys/un.h>
#include <endian.h>
#include <arpa/inet.h>
#include <pthread.h>


char *name;
int client_socket;
int ID = 0;

void init(char *connection_type, char *server_ip_path, char *port);
void handle_message();
void connect_to_server();
void send_message(uint8_t message_type);
void clean();

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]) {

    name = argv[1];
    char *connection_type = argv[2];
    char *server_ip = argv[3];
    char *port = NULL;
    if (argc == 5) {
        port = argv[4];
    }

    init(connection_type, server_ip, port);
    connect_to_server();
    handle_message();
    clean();
    return 0;
}

void* handle_request(void * arg) {

    request_t* req_tmp = (request_t *)arg;
    request_t req;
    strcpy(req.text,req_tmp->text);
    int id = req_tmp ->ID;
    //free(req_tmp);

    char *buffer = calloc(sizeof(char), 100 + 2 * strlen(req.text));
    char *buffer_res = calloc(sizeof(char),100 + 2 * strlen(req.text));
    sprintf(buffer, "echo '%s' | awk '{for(x=1;$x;++x)print $x}' | sort | uniq -c", (char *) req.text);
    FILE *result = popen(buffer, "r");
    int n = fread(buffer, 1, 99 + 2 * strlen(req.text), result);
    pclose(result);
    buffer[n] = '\0';

    int words_count = 1;
    char *res = strtok(req.text, " ");
    while (strtok(NULL, " ") && res) {
        words_count++;
    }

    sprintf(buffer_res, "ID: %d || Sum: %d\n%s",id, words_count, buffer);
    printf("%s\n", buffer_res);
    //sleep(4);
    pthread_mutex_lock(&mutex);
    send_message(RESULT);
    int len = strlen(buffer_res);
    if (write(client_socket,&len, sizeof(int)) != sizeof(int))
        raise_error(" Could not write message type");
    if (write(client_socket, buffer_res, len) != len)
        raise_error(" Could not write message type");
    printf("Result has been sent to server \n");
    pthread_mutex_unlock(&mutex);
    free(buffer);
    free(buffer_res);
    return NULL;
}

void send_message(uint8_t message_type) {
    uint16_t message_size = (uint16_t) (strlen(name) + 1);
    if (write(client_socket, &message_type, TYPE_SIZE) != TYPE_SIZE)
        raise_error(" Could not write message type");
    if (write(client_socket, &message_size, LEN_SIZE) != LEN_SIZE)
        raise_error(" Could not write message size");
    if (write(client_socket, name, message_size) != message_size)
        raise_error(" Could not write name message");
}

void connect_to_server() {
    send_message(REGISTER);

    uint8_t message_type;
    if (read(client_socket, &message_type, 1) != 1) raise_error("\n Could not read response message type\n");

    switch (message_type) {
        case WRONGNAME:
            raise_error("Name already in use\n");
        case FAILSIZE:
            raise_error("Too many clients logged\n");
        case SUCCESS:
            printf("Logged in successfully\n");
            break;
        default:
            raise_error("Impossible \n");
    }
}

void handle_message() {
    uint8_t message_type;
    pthread_t thread;
    request_t *req;
    int x = 1;
    while (x) {
        if (read(client_socket, &message_type, TYPE_SIZE) != TYPE_SIZE)
            raise_error(" Could not read message type");
        switch (message_type) {
            case REQUEST:
                req =(request_t*) calloc(1,sizeof(request_t));
                uint16_t req_len;
                if (read(client_socket, &req_len, 2) <= 0) {
                    raise_error("cannot read length");
                }

                if (read(client_socket, req-> text, req_len) < 0) {
                    raise_error("cannot read whole text");
                }
                printf("Processing request ID: %d \n",req->ID);
                pthread_create(&thread, NULL, handle_request, req);
                pthread_detach(thread);
                break;
            case PING:
                pthread_mutex_lock(&mutex);
                send_message(PONG);
                pthread_mutex_unlock(&mutex);
                break;
            default:
                printf("Unknown message type\n");
                break;
        }
    }
}

void handle_signals(int signo) {
    send_message(UNREGISTER);
    //printf("KILLED BY SIGNAL \n");
    exit(1);
}

void init(char *connection_type, char *server_ip_path, char *port) {

    struct sigaction act;
    act.sa_handler = handle_signals;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
    int conn_type;

    if (strcmp("WEB", connection_type) == 0) {
        conn_type = WEB;
        uint32_t ip = inet_addr(server_ip_path);
        uint16_t port_num = (uint16_t) atoi(port);
        if (port_num < 1024 || port_num > 65535) {
            raise_error("wrong port");
        }
        struct sockaddr_in web_address;
        memset(&web_address, 0, sizeof(struct sockaddr_in));
        web_address.sin_family = AF_INET;
        web_address.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr(server_ip_path);//inet_addr("192.168.0.66"); //htonl(ip);
        web_address.sin_port = htons(port_num);
        if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            raise_error("socket");
        }
        if (connect(client_socket, (const struct sockaddr *) &web_address, sizeof(web_address)) == -1) {
            raise_error("connect");
        }
        printf("Connected to web socket \n");

    } else if (strcmp("LOCAL", connection_type) == 0) {
        conn_type = LOCAL;

        char *unix_path = server_ip_path;
        //todo -> check len of unix_path
        struct sockaddr_un local_address;
        local_address.sun_family = AF_UNIX;
        snprintf(local_address.sun_path, MAX_PATH, "%s", unix_path);
        if ((client_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
            raise_error("\n Could not create local socket\n");

        if (connect(client_socket, (const struct sockaddr *) &local_address, sizeof(local_address)) == -1)
            raise_error("\n Could not connect to local socket\n");
        printf("Connected to local socket \n");
    } else {
        raise_error("wrong type of argument");
    }
}

void clean() {
    send_message(UNREGISTER);
    if (shutdown(client_socket, SHUT_RDWR) == -1)
        fprintf(stderr, "\n Could not shutdown Socket\n");
    if (close(client_socket) == -1)
        fprintf(stderr, "\n Could not close Socket\n");
}