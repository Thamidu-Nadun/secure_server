#include <stdio.h>

#define PATH_LOG "server.log"

void write_log(char* message){
    FILE* file = fopen(PATH_LOG, "a");
    if (file == NULL){
        perror("couldn't open log file");
        return;
    }
    fprintf(file, "%s\n", message);
    fclose(file);
}