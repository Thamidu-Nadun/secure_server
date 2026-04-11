#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../types/general_types.h"
#include "db.c"
#include "logger.c"
#include "token.h"

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

    printf("Parsing message: %s\n", str_ptr);

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

int command_handler(Command* commands, char* client_ip, uint16_t port){
    for (int i = 0; i < 5; i++){
        if (commands[i].command_name[0] == 0) continue; // empty command

        char* cmd_name = commands[i].command_name;
        char* cmd_args = (char*)commands[i].args;
        printf("Handling command: %s with args: %s\n", cmd_name, cmd_args);

        // ****** Command handling logic ****** //

        if (strcmp(cmd_name, "LOGIN") == 0){
            printf("Processing LOGIN command with args: %s\n", cmd_args);
            
            char username[256], password[256];
            sscanf(cmd_args, "%s %s", username, password);

            // 1. get user,
            User* user = get_user(username);
            if (user == NULL) {
                printf("User not found: %s\n", cmd_args);
                client_log(client_ip, port, "Failed login attempt", strlen("Failed login attempt"));
                return EXIT_FAILURE;
            }
            // 2. check with password,
            if (strcmp(user->password, password) != 0) {
                printf("Incorrect password for user: %s\n", cmd_args);
                client_log(client_ip, port, "Failed login attempt", strlen("Failed login attempt"));
                free(user);
                return EXIT_FAILURE;
            }
            // 3. store token and return token to client
            FILE *file = fopen("tokens.txt", "a");
            if (file == NULL) {
                perror("Could not open tokens file");
                free(user);
                return EXIT_FAILURE;
            }
            // 4. generate token, store in tokens.txt
            int chars = 6;
            char token_str[2* chars + 1];
            generate_token(token_str, sizeof(token_str));
            fprintf(file, "%s:%s\n", user->username, token_str);
            fclose(file);
            printf("Generated token for user '%s': %s\n", user->username, token_str);
            free(user);
            // 5. log the login event
            client_log(client_ip, port, "User logged in successfully", strlen("User logged in successfully"));
            return EXIT_SUCCESS;
        }
        else if (strcmp(cmd_name, "REGISTER") == 0)
        {
            printf("Processing REGISTER command with args: %s\n", cmd_args);

            char username[100], password[100];
            sscanf(cmd_args, "%s %s", username, password);
            if (add_user(username, password) == EXIT_SUCCESS) {
                printf("User '%s' registered successfully.\n", username);
                client_log(client_ip, port, "User registered successfully", strlen("User registered successfully"));
                return EXIT_SUCCESS;
            } else {
                printf("Failed to register user '%s'.\n", username);
                client_log(client_ip, port, "Failed registration attempt", strlen("Failed registration attempt"));
                return EXIT_FAILURE;
            }
        }
        else if (strcmp(cmd_name, "LOGOUT") == 0)
        {
            printf("Processing LOGOUT command with args: %s\n", cmd_args);
            char username[256];
            sscanf(cmd_args, "%s", username);
            if (logout_user(username) != EXIT_SUCCESS) {
                printf("Failed to logout user '%s'.\n", username);
                client_log(client_ip, port, "Failed logout attempt", strlen("Failed logout attempt"));
                return EXIT_FAILURE;
            }
            printf("User '%s' logged out successfully.\n", username);
            client_log(client_ip, port, "User logged out successfully", strlen("User logged out successfully"));
            return EXIT_SUCCESS;
        }
        else if (strcmp(cmd_name, "MSG") == 0)
        {
            printf("Processing MSG command with args: %s\n", cmd_args);
            printf("Received message: %s\n", cmd_args);

            client_log(client_ip, port, cmd_args, strlen(cmd_args));
            return EXIT_SUCCESS;
        }
        else
        {
            printf("Unknown command: %s\n", cmd_name);
            client_log(client_ip, port, "Unknown command received", strlen("Unknown command received"));
            return EXIT_FAILURE;
        }
    }
    free(commands);
}

// int main(){
//     char msg1[] = "LOGIN: user1 1234";
//     char msg2[] = "MSG: Hello, World";

//     char str[sizeof(msg1) + 10];
//     char str2[sizeof(msg2) + 10];

//     char format[] = "LEN: %lu %s;";
//     sprintf(str, format, strlen(msg1), msg1);
//     sprintf(str2, format, strlen(msg2), msg2);

//     char* msg = strcat(str, str2);
//     printf("Combined message: %s\n", msg);

//     Command* commands = parser(msg);
//     command_handler(commands);
// }