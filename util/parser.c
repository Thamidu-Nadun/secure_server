#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../types/general_types.h"
#include "db.c"
#include "logger.c"
#include "token.h"
#include "secure.c"

int secure_send(int sockfd, char *buff, int len, long shared_secret);

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

int command_handler(Command* commands, char* client_ip, uint16_t port, int clientfd, long shared_secret){
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
            char password_hash[65];
            create_password_hash(password, user->salt, password_hash);

            printf("Comparing password hash: %s with stored hash: %s\n", password_hash, user->password);
            printf("Salt used for hashing: %s\n", user->salt);

            if (strcmp(user->password, password_hash) != 0) {
                printf("Incorrect password for user: %s\n", cmd_args);
                char log_msg_format[] = "Failed login attempt for user '(%s)'";
                char log_msg[sizeof(log_msg_format) + strlen(username)];
                sprintf(log_msg, log_msg_format, username);
                client_log(client_ip, port, log_msg, strlen(log_msg));
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
            int chars = 3;
            char token_str[2* chars + 1];
            generate_token(token_str, sizeof(token_str));

            // 4.1 generate expiry time
            int expiry_min = 5;
            time_t now = time(NULL);
            time_t expiry_time = now + expiry_min * 60;
            fprintf(file, "%s:%s:%ld\n", user->username, token_str, expiry_time);
            fclose(file);
            printf("Generated token for user '%s': %s\n", user->username, token_str);
            free(user);

            // 5. log the login event
            char log_msg_format[] = "User logged in successfully '(%s)'";
            char log_msg[sizeof(log_msg_format) + strlen(username)];
            sprintf(log_msg, log_msg_format, username);
            client_log(client_ip, port, log_msg, strlen(log_msg));

            // 6. send token back to client
            char response_format[] = "OK: 1;SID: 1042; TOKEN: %s";
            char response[sizeof(response_format) + strlen(token_str)];
            sprintf(response, response_format, token_str);
            secure_send(clientfd, response, strlen(response), shared_secret);
            return EXIT_SUCCESS;
        }
        else if (strcmp(cmd_name, "REGISTER") == 0)
        {
            printf("Processing REGISTER command with args: %s\n", cmd_args);

            char username[100], password[100];
            sscanf(cmd_args, "%s %s", username, password);
            if (add_user(username, password) == EXIT_SUCCESS) {
                printf("User '%s' registered successfully.\n", username);
                char log_msg_format[] = "User registered successfully '(%s)'";
                char log_msg[sizeof(log_msg_format) + strlen(username)];
                sprintf(log_msg, log_msg_format, username);
                client_log(client_ip, port, log_msg, strlen(log_msg));

                char response_format[] = "OK: 1;SID: 1042; User '%s' registered successfully";
                char response[sizeof(response_format) + strlen(username)];
                sprintf(response, response_format, username);
                secure_send(clientfd, response, strlen(response), shared_secret);
                return EXIT_SUCCESS;
            } else {
                printf("Failed to register user '%s'.\n", username);
                char log_msg_format[] = "Failed registration attempt for user '(%s)'";
                char log_msg[sizeof(log_msg_format) + strlen(username)];
                sprintf(log_msg, log_msg_format, username);
                client_log(client_ip, port, log_msg, strlen(log_msg));
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
                char log_msg_format[] = "Failed logout attempt for user '(%s)'";
                char log_msg[sizeof(log_msg_format) + strlen(username)];
                sprintf(log_msg, log_msg_format, username);
                client_log(client_ip, port, log_msg, strlen(log_msg));
                return EXIT_FAILURE;
            }
            printf("User '%s' logged out successfully.\n", username);
            client_log(client_ip, port, "User logged out successfully", strlen("User logged out successfully"));

            char response_format[] = "OK: 1;SID: 1042; User '%s' logged out successfully";
            char response[sizeof(response_format) + strlen(username)];
            sprintf(response, response_format, username);
            secure_send(clientfd, response, strlen(response), shared_secret);
            return EXIT_SUCCESS;
        }
        else if (strcmp(cmd_name, "MSG") == 0)
        {
            printf("Processing MSG command with args: %s\n", cmd_args);
            printf("Received message: %s\n", cmd_args);

            char log_msg_format[] = "Received message: '%s'";
            char log_msg[sizeof(log_msg_format) + strlen(cmd_args)];
            sprintf(log_msg, log_msg_format, cmd_args);
            client_log(client_ip, port, log_msg, strlen(log_msg));

            char response_format[] = "OK: 1;SID: 1042; Message received";
            char response[sizeof(response_format) + strlen(cmd_args)];
            sprintf(response, response_format, cmd_args);
            secure_send(clientfd, response, strlen(response), shared_secret);
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