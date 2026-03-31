#pragma once

#include <stdio.h>
#include <string.h>

void handle_command(const char* str){
    if (strncmp(str, "LOGIN:", strlen("LOGIN:")) == 0){
        printf("Handling LOGIN command\n");
    } else if (strncmp(str, "MSG:", strlen("MSG:")) == 0){
        printf("Handling MSG command\n");
    } else {
        printf("Unknown command: %s\n", str);
    }
}