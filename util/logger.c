#include <stdio.h>
#include <time.h>

#define SERVER_PATH_LOG "server_IT24104294.log"
#define CLIENT_PATH_LOG "srv/ie2102/IT24104294/%s.log"

int write_log(long timestamp, char* path, char* message){
    FILE* file = fopen(path, "a");
    if (file == NULL){
        perror("couldn't open log file");
        return EXIT_FAILURE;
    }
    fprintf(file, "%ld-%s\n", timestamp, message);
    fclose(file);

    return EXIT_SUCCESS;
}

int server_log(char* client_ip, uint16_t port, char* message, int msg_len){
    long now = (long)time(NULL);
    int pid = (int)getpid();
    char format[] = "%s:%u PID: %d MESSAGE=> %s";
    char* log_entry = malloc(sizeof(format) + strlen(client_ip)+ sizeof(uint16_t) + sizeof(int) + msg_len);
    snprintf(log_entry, sizeof(format) + strlen(client_ip) + sizeof(uint16_t) + sizeof(int) + strlen(message), format, client_ip, port, pid, message);
    if (write_log(now, SERVER_PATH_LOG, log_entry) != EXIT_SUCCESS){
        fprintf(stderr, "Error writing log entry\n");
        free(log_entry);
        return EXIT_FAILURE;
    }
    free(log_entry);
    return EXIT_SUCCESS;
}

int client_log(char* client_ip, uint16_t port, char* username, char* message, int msg_len){
    long now = (long)time(NULL);
    char client_log_path[256];
    snprintf(client_log_path, sizeof(client_log_path), CLIENT_PATH_LOG, username);
    int pid = (int)getpid();
    char format[] = "%s:%u PID: %d MESSAGE=> %s";
    char* log_entry = malloc(sizeof(format) + strlen(client_ip)+ sizeof(uint16_t) + sizeof(int) + msg_len);
    snprintf(log_entry, sizeof(format) + strlen(client_ip) + sizeof(uint16_t) + sizeof(int) + strlen(message), format, client_ip, port, pid, message);
    if (write_log(now, client_log_path, log_entry) != EXIT_SUCCESS){
        fprintf(stderr, "Error writing log entry\n");
        free(log_entry);
        return EXIT_FAILURE;
    }
    free(log_entry);
    return EXIT_SUCCESS;
}
