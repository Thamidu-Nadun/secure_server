#include <stdio.h>

#define PATH_LOG "server.log"

int write_log(char* message){
    FILE* file = fopen(PATH_LOG, "a");
    if (file == NULL){
        perror("couldn't open log file");
        return EXIT_FAILURE;
    }
    fprintf(file, "%s\n", message);
    fclose(file);

    return EXIT_SUCCESS;
}

int client_log(char* client_ip, uint16_t port, char* message, int msg_len){
    char format[] = "%s:%u => %s";
    char* log_entry = malloc(sizeof(format) + strlen(client_ip)+ sizeof(uint16_t) + msg_len);
    snprintf(log_entry, sizeof(format) + strlen(client_ip) + sizeof(uint16_t) + msg_len, format, client_ip, port, message);
    if (write_log(log_entry) != EXIT_SUCCESS){
        fprintf(stderr, "Error writing log entry\n");
        free(log_entry);
        return EXIT_FAILURE;
    }
    free(log_entry);
    return EXIT_SUCCESS;
}
