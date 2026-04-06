#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../types/general_types.h"
#include "db.c"

/**
 * This function parse one line string and if there is a command, 
 * command and args will be returned. Otherwise, it will return NULL.
 * 
 * @param line the string to parse
 * @return Command* the parsed command, or NULL if no command is found
 * 
 */
Command command_parser(char* line){
    Command cmd;
    int matched = sscanf(line, "%[^:]:%*[ ]%[^\n]", cmd.command_name, cmd.args);
    if (matched < 2) {
        printf("No command found in line: %s\n", line);
        return (Command){0};
    }
    return cmd;
}

Command* parser(char* str_ptr){
    Command* commands = malloc(sizeof(Command) * 5); // max 5 commands
    int cmd_count = 0;

    char* lines = strtok(str_ptr, ";");
    while (lines != NULL){
        printf("Line: %s\n", lines);

        unsigned int len;
        char command_str[256];
        sscanf(lines, "LEN: %u %[^\n]", &len, command_str);

        Command cmd = command_parser(command_str);
        printf("Command name: %s, Command args: %s\n", cmd.command_name, (char*)cmd.args);
        commands[cmd_count++] = cmd;
        lines = strtok(NULL, ";");
    }
    return commands;
}

void command_handler(Command* commands){
    for (int i = 0; i < 5; i++){
        if (commands[i].command_name[0] == 0) continue; // empty command
        char* cmd_name = commands[i].command_name;
        char* cmd_args = (char*)commands[i].args;
        printf("Handling command: %s with args: %s\n", cmd_name, cmd_args);

        if (strcmp(cmd_name, "LOGIN") == 0){
            printf("Processing LOGIN command with args: %s\n", cmd_args);
        } else if (strcmp(cmd_name, "REGISTER") == 0){
            printf("Processing REGISTER command with args: %s\n", cmd_args);
        } else if (strcmp(cmd_name, "LOGOUT") == 0){
            printf("Processing LOGOUT command with args: %s\n", cmd_args);
        } else if (strcmp(cmd_name, "MSG") == 0){
            printf("Processing MSG command with args: %s\n", cmd_args);
        } else {
            printf("Unknown command: %s\n", cmd_name);
        }
    }
    free(commands);
}

int main(){
    char msg1[] = "LOGIN: user1 1234";
    char msg2[] = "MSG: Hello, World";

    char str[sizeof(msg1) + 10];
    char str2[sizeof(msg2) + 10];

    char format[] = "LEN: %lu %s;";
    sprintf(str, format, strlen(msg1), msg1);
    sprintf(str2, format, strlen(msg2), msg2);

    char* msg = strcat(str, str2);
    printf("Combined message: %s\n", msg);

    Command* commands = parser(msg);
    command_handler(commands);
}