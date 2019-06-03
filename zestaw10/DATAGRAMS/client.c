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
enum connect_type c_type;
int client_socket;

int request_counter = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t request1_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;

void init(char *connection_type, char *server_ip_path, char *port);

void send_message(uint8_t message_type, char *value);

void handle_message();

void clean();

void register_on_server();

int signal_ = 0;

int main(int argc, char *argv[]) {

    name = argv[1];
    char *connection_type = argv[2];
    char *server_ip = argv[3];
    char *port = NULL;
    if (argc == 5) {
        port = argv[4];
    }

    atexit(clean);
    init(connection_type, server_ip, port);
    register_on_server();
    handle_message();
    return 0;
}

void* handle_request(void* arg) {


    struct request_t* got_arg = arg;
    struct request_t req;
    strcpy(req.text, got_arg->text);
    req.ID = got_arg->ID;

    //printf("HALKOOOO \n");

    //printf("STARTED WITH: %s\n", req.text);
    char *buffer = malloc(100 + 2 * strlen(req.text));
    char *buffer_res = malloc(100 + 2 * strlen(req.text));

    sprintf(buffer, "echo '%s' | awk '{for(x=1;$x;++x)print $x}' | sort | uniq -c", (char *) req.text);
    FILE *result = popen(buffer, "r");

    int n = fread(buffer, 1, 99 + 2 * strlen(req.text), result);
    buffer[n] = '\0';
    pclose(result);
    //todo -> check len!!!!
    int words_count = 1;
    char *res = strtok(req.text, " ");
    while (strtok(NULL, " ") && res) {
        words_count++;
    }
    sprintf(buffer_res, "ID: %d || Sum: %d\n%s",req.ID, words_count, buffer);
    printf("%s\n", buffer_res);
    //sleep(5);
    pthread_mutex_lock(&request1_mutex);
    send_message(RESULT, buffer_res);
    pthread_mutex_unlock(&request1_mutex);
   // printf("RESULT SENT \n");
    free(buffer);
    free(buffer_res);
    return NULL;

}
void send_message(uint8_t message_type, char *value) {
    message_t msg;
    msg.message_type = message_type;
    snprintf(msg.name, 64, "%s", name);
    msg.connect_type = c_type;
    if(value){
        snprintf(msg.value, strlen(value), "%s", value);
    }
    //msg.fd = client_socket;
    pthread_mutex_lock(&mutex);
    if (write(client_socket, &msg, sizeof(message_t)) != sizeof(message_t))
        raise_error("\nError : Could not send message\n");
    pthread_mutex_unlock(&mutex);
    //printf("MESSAGE SENT \n");
}

void register_on_server() {
    send_message(REGISTER, NULL);

    uint8_t message_type;
    if (read(client_socket, &message_type, 1) != 1) raise_error("\n Could not read response message type\n");

    switch (message_type) {
        case WRONGNAME:
            raise_error("Name already in use\n");
        case FAILSIZE:
            raise_error("Too many clients logged to the server\n");
        case SUCCESS:
            printf("Logged in successfully\n");
            break;
        default:
            raise_error("\n Unpredicted REGISTER behavior\n");
    }
}

void handle_message() {
    uint8_t message_type;
    pthread_t thread;
    request_t req;
    int x = 1;
    while (x) {
       // printf("GOT STH \n");
        if (read(client_socket, &message_type, sizeof(uint8_t)) != sizeof(uint8_t))
            raise_error(" Could not read message type");
        //printf("HANDLING: %d \n", message_type);
        switch (message_type) {
                
            case REQUEST:
                //handle_request();
                if (read(client_socket, &req, sizeof(request_t)) <= 0) {
                    raise_error("cannot read length");
                }
                printf("Processing request %d \n",req.ID);
                pthread_create(&thread, NULL, handle_request, &req);
                pthread_detach(thread);
                //sleep(1);
                break;
            case PING:
                //printf("GOT PING \n");
                pthread_mutex_lock(&request1_mutex);
                send_message(PONG, NULL);
                pthread_mutex_unlock(&request1_mutex);
                break;
            case END:
                raise(SIGINT);
            default:
                //printf("Unknown message type %d\n", message_type);
                break;
        }
    }
}

void handle_signals(int signo) {
    signal_++;
    printf("KILLED BY SIGNAL \n");
    exit(1);
}

void init(char *connection_type, char *server_ip_path, char *port) {

    struct sigaction act;
    act.sa_handler = handle_signals;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    if (strcmp("WEB", connection_type) == 0) {

        c_type = WEB;
        uint16_t port_num = (uint16_t) atoi(port);
        if (port_num < 1024 || port_num > 65535) {
            raise_error("wrong port");
        }

        struct sockaddr_in web_address;
        memset(&web_address, 0, sizeof(struct sockaddr_in));

        web_address.sin_family = AF_INET;
        web_address.sin_addr.s_addr = htonl(
                INADDR_ANY);  //inet_addr(server_ip_path);//inet_addr("192.168.0.66");
        web_address.sin_port = htons(port_num);

        if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            raise_error("socket");
        }
        if (connect(client_socket, (const struct sockaddr *) &web_address, sizeof(web_address)) == -1) {
            raise_error("connect");
        }
        printf("Connected to web socket \n");

    } else if (strcmp("LOCAL", connection_type) == 0) {
        c_type = LOCAL;

        //char *unix_path = server_ip_path;

        //todo -> check len of unix_path

        if ((client_socket = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
            raise_error("\n Could not create local socket\n");


        struct sockaddr_un local_address_bind;
        local_address_bind.sun_family = AF_UNIX;
        snprintf(local_address_bind.sun_path, MAX_PATH, "%s", name);

        if (bind(client_socket, (const struct sockaddr *) &local_address_bind, sizeof(local_address_bind)))
            raise_error("\n Could not bind\n");

        struct sockaddr_un local_address;
        local_address.sun_family = AF_UNIX;
        snprintf(local_address.sun_path, MAX_PATH, "%s", server_ip_path);

        if (connect(client_socket, (const struct sockaddr *) &local_address, sizeof(local_address)) == -1)
            raise_error("\n Could not connect to local socket\n");
        printf("Connected to local socket \n");

    } else {
        raise_error("wrong type of argument");
    }
}

void clean() {
    if (signal_ > 0){
        send_message(UNREGISTER, NULL);
        unlink(name);
    }

    if (close(client_socket) == -1)
        fprintf(stderr, "\n Could not close Socket\n");
    unlink(name);
}