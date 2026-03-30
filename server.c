#include "util/mem_util.c"
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8000
#define BUFFER_SIZE 1024


int handle_client(int client){
    char* buffer = allocate_mem(BUFFER_SIZE);
    ssize_t bytes_recv;
    while ((bytes_recv = recv(client, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_recv] = '\0';
        printf("Received: %s\n", buffer);
    }
    if (bytes_recv < 0) {
        perror("error receiving data");
    }

    free(buffer);
    return EXIT_SUCCESS;
}

int main(){
    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = INADDR_ANY;

    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("couldn't create the socket");
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, (struct sockaddr*)&srv, sizeof(srv)) < 0) {
        perror("couldn't bind to the socket");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 5) < 0) {
        perror("couldn't listen on the socket");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1){
        int clientfd;
        if ((clientfd = accept(sockfd, NULL, NULL)) < 0) {
            perror("couldn't accept the connection");
            continue;
        }

        if (fork() == 0){
            close(sockfd);
            if (handle_client(clientfd) != EXIT_SUCCESS) {
                fprintf(stderr, "Error handling client\n");
            }
            close(clientfd);
            exit(EXIT_SUCCESS);
        }
    }
    close(sockfd);
    return EXIT_SUCCESS;
}