#include <stdio.h>
#include <stdlib.h>
#include "mem_util.c"
#include "../types/general_types.h"

#define SID 1001

Response* create_response(int code, char* message){
    Response* res;
    if (!res)
        return NULL;

    res->code = code;
    res->sid = SID;
    res->message = message;
    return res;
}

void send_response(int sockfd, ServerResponseTypes type, Response* response){
    if (type == OK){
        // send OK response
        char* buffer = allocate_mem(strlen(response->message)+sizeof(char)*15);
        sprintf(buffer, "OK: %d SID: %d %s", response->code, response->sid, response->message);
        send(sockfd, buffer, strlen(buffer), 0);
        free(buffer);
    }else if (type == ERR){
        // send ERR response
        char* buffer = allocate_mem(strlen(response->message)+sizeof(char)*15);
        sprintf(buffer, "ERR: %d SID: %d %s", response->code, response->sid, response->message);
        send(sockfd, buffer, strlen(buffer), 0);
        free(buffer);
    }else {
        fprintf(stderr, "Unknown response type: %d\n", type);
    }
}