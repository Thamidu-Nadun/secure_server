#include <stdio.h>
#include <string.h>
#include "../types/general_types.h"

void line_parser(char* str_ptr, int size){
    char* lines = strtok(str_ptr, ";");

    while (lines != NULL){
        printf("Line: %s\n", lines);
        lines = strtok(NULL, ";");
    }
}

/**
 * This function parse one line string and if there is a command, 
 * command and args will be returned. Otherwise, it will return NULL.
 * 
 * @param str_ptr the string to parse
 * @return Command* the parsed command, or NULL if no command is found
 * 
 */
Command command_parser(char* str_ptr){}

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

    line_parser(msg, strlen(msg));
}