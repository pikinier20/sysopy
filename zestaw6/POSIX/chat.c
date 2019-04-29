//
// Created by przjab98 on 27.04.19.
//


#include "chat.h"

void raise_error(char *msg){
    fprintf(stderr,"%s",msg);
    kill(getpid(), SIGINT);
}
int convert_to_num(char *given_string) {
    if (!given_string) {
        return -1;
    }
    char *tmp;
    int result = (int) strtol(given_string, &tmp, 10);
    if (strcmp(tmp, given_string) != 0) {
        return result;
    } else {
        return -1;
    }
}
unsigned cmdPriority(enum MSG_COMMAND cmd){
    if(cmd == STOP){
        return 4;
    }else if(cmd == LIST){
        return 3;
    }else if(cmd == FRIENDS ){
        return 2;
    }else{
        return 1;
    }
}

char* getClientQueueName() {
    char *name=malloc(32*sizeof(char));
    sprintf(name, "/cli%i%i", getpid(), rand()%1000);
    return name;
}


struct msg* parse(char* message, int* addressee){
    int type = convert_to_num(&message[0]);
    struct msg* msg = malloc(sizeof(msg));
    msg->mType = type;
    int sender;

    if(type != _2ONE){
        sscanf(message+1,"%d%s",&sender, msg->msg );
    }else{
        sscanf(message+1, "%d%d%s", addressee,&sender, msg->msg );
    }

    msg->sender = sender;
    return msg;
}


