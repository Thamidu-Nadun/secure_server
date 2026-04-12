#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/wait.h>
#include "util/mem_util.c"
#include "util/parser.c"
#include "util/dh.h"
#include "util/secure.c"

#define PORT 50294
#define BUFFER_SIZE 1024
#define KEY 6
#define MAX_CLIENTS 100

long exchange_dh(int client);

int handle_client(int client, struct sockaddr_in client_addr){
    // ** Client connected ** //
    char client_ip[INET_ADDRSTRLEN];
    u_int16_t port = htons(client_addr.sin_port);
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    printf("Client connected: %s:%d\n", client_ip, ntohs(client_addr.sin_port));

    // ** Diffie-Hellman key exchange ** //
    long shared_secret = exchange_dh(client);
    if (shared_secret < 0) {
        fprintf(stderr, "Error during Diffie-Hellman key exchange\n");
        return EXIT_FAILURE;
    }

    size_t capacity = BUFFER_SIZE, total = 0;
    char* buffer = allocate_mem(capacity);
    char temp[BUFFER_SIZE];
    ssize_t bytes_recv;

    while ((bytes_recv = recv(client, temp, sizeof(temp), 0)) > 0){
        if (total + bytes_recv + 1 > capacity){
            while (total + bytes_recv + 1 > capacity) capacity *= 2;
            buffer = reallocate_mem(buffer, capacity);
        }
        memcpy(buffer + total, temp, bytes_recv);
        total += bytes_recv;
    }

    if (bytes_recv < 0){
        perror("error receiving data");
        free(buffer);
        return EXIT_FAILURE;
    }

    buffer[total] = '\0';
    printf("Received message: %s\n", buffer);
    xor_cipher(buffer, shared_secret, strlen(buffer));

    Command* commands =  parser(buffer);
    int handler_result = command_handler(commands, client_ip, port, client, shared_secret);
    if (handler_result != EXIT_SUCCESS) {
        fprintf(stderr, "Error handling commands\n");
        free(buffer);
        char error_msg[] = "ERR: 0;SID: 1042; Error processing commands";
        secure_send(client, error_msg, strlen(error_msg), shared_secret);
        return EXIT_FAILURE;
    }
    free(buffer);
    
    return EXIT_SUCCESS;
}

int main(){
    srand(time(NULL)); // seed for token generation

    // socket setup
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
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int clientfd;
        if ((clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len)) < 0) {
            perror("couldn't accept the connection");
            continue;
        }

        if (fork() == 0) {
            close(sockfd);
            srand(time(NULL) ^ getpid()); // seed for token generation in child process
            if (handle_client(clientfd, client_addr) != EXIT_SUCCESS) {
                fprintf(stderr, "Error handling client\n");
            }
            close(clientfd);
            exit(EXIT_SUCCESS);
        }
        close(clientfd);
        while (waitpid(-1, NULL, WNOHANG) > 0); // clean up child processes
    }
    close(sockfd);
    return EXIT_SUCCESS;
}

long exchange_dh(int client){
    long pub = mod_exp(G, KEY, P);

    // recv client's public key
    long client_pub;
    if (recv(client, &client_pub, sizeof(client_pub), 0) < 0) {
        perror("Error receiving client's public key");
        return  EXIT_FAILURE;
    }

    // send server's public key
    if (send(client, &pub, sizeof(pub), 0) < 0) {
        perror("Error sending server's public key");
        return EXIT_FAILURE;
    }

    // compute shared secret
    long shared_secret = mod_exp(client_pub, KEY, P);
    printf("Shared secret: %ld\n", shared_secret);
    return shared_secret;
}