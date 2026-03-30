#include "util/mem_util.c"
#include <unistd.h>
#include <arpa/inet.h>

#define HOST "127.0.0.1"
#define PORT 8000
#define BUFFER_SIZE 1024

#define HEADER "LEN: %d PAYLOAD: %s"

int main(){
    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);

    if (inet_pton(AF_INET, HOST, &srv.sin_addr) <= 0) {
        perror("invalid address");
        exit(EXIT_FAILURE);
    }

    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("couldn't create the socket");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr*)&srv, sizeof(srv)) < 0) {
        perror("couldn't connect to the server");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char *payload = malloc(BUFFER_SIZE);
    if (payload == NULL) {
        perror("couldn't allocate payload buffer");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Enter the payload to send: ");
    if (fgets(payload, BUFFER_SIZE, stdin) == NULL) {
        perror("error reading input");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char *buffer = allocate_mem(BUFFER_SIZE);
    int payload_len = strlen(payload);

    snprintf(buffer, BUFFER_SIZE, HEADER, payload_len, payload);

    send(sockfd, buffer, strlen(buffer), 0);

    free(buffer);
    close(sockfd);

    return EXIT_SUCCESS;
}