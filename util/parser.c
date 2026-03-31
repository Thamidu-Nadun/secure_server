#include <stdio.h>
#include <string.h>

void parser(char* str_ptr, int size){
    char* lines = strtok(str_ptr, ";");

    while (lines != NULL){
        printf("Line: %s\n", lines);
        lines = strtok(NULL, ";");
    }
}

int main(){
    char msg1[] = "LOGIN: user1";
    char msg2[] = "MSG: Hello, World";

    char str[sizeof(msg1) + 10];
    char str2[sizeof(msg2) + 10];

    char format[] = "LEN: %lu PAYLOAD: %s;";
    sprintf(str, format, strlen(msg1), msg1);
    sprintf(str2, format, strlen(msg2), msg2);

    char* msg = strcat(str, str2);
    printf("Combined message: %s\n", msg);

    parser(msg, strlen(msg));
}