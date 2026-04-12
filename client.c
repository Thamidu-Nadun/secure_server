#include "util/mem_util.c"
#include <unistd.h>
#include <arpa/inet.h>
#include "util/dh.h"
#include "util/secure.c"

#define HOST "127.0.0.1"
#define PORT 50294
#define BUFFER_SIZE 1024
#define HEADER "LEN: %d %s"
#define KEY 4

long exchange_dh(int client);

int main(){
    /**
     * handling options;
     * login = 0,
     * register = 1,
     * logout = 2,
     * send message = 3
     */
    printf("Choose an option:\n");
    printf("0: LOGIN\n");
    printf("1: REGISTER\n");
    printf("2: LOGOUT\n");
    printf("3: SEND MESSAGE\n");
    
    char *payload = allocate_mem(BUFFER_SIZE);
    int option;
    printf("option> ");
    scanf("%d", &option);
    getchar();
    char username[256], password[256];
    

    switch (option){
    case 0:
        printf("====LOGIN====\n");
        printf("Enter username: ");
        scanf("%s", username);
        printf("Enter password: ");
        scanf("%s", password);
        snprintf(payload, BUFFER_SIZE, "LOGIN: %s %s", username, password);
        break;
    case 1:
        printf("====REGISTER====\n");
        printf("Enter username: ");
        scanf("%s", username);
        printf("Enter password: ");
        scanf("%s", password);
        snprintf(payload, BUFFER_SIZE, "REGISTER: %s %s", username, password);
        break;
    case 2:
        printf("====LOGOUT====\n");
        printf("Enter username: ");
        scanf("%s", username);
        snprintf(payload, BUFFER_SIZE, "LOGOUT: %s", username);
        break;
    case 3:
        printf("====SEND MESSAGE====\n");
        char message[256], token[10];
        printf("Enter token: ");
        scanf("%s", token);
        printf("Enter message: ");
        scanf(" %[^\n]", message);
        snprintf(payload, BUFFER_SIZE, "MSG: token:%s %s", token, message);
        break;
    default:
        printf("Invalid option\n");
        free(payload);
        exit(EXIT_FAILURE);
    }

    // socket setup
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

    // ** Diffie-Hellman key exchange ** //
    long shared_secret = exchange_dh(sockfd);
    if (shared_secret < 0) {
        fprintf(stderr, "Error during Diffie-Hellman key exchange\n");
        free(payload);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char *buffer = allocate_mem(BUFFER_SIZE);
    int payload_len = strlen(payload);

    snprintf(buffer, BUFFER_SIZE, HEADER, payload_len, payload);
    secure_send(sockfd, buffer, strlen(buffer), shared_secret);
    shutdown(sockfd, SHUT_WR);

    memset(buffer, 0, BUFFER_SIZE);
    ssize_t bytes_recv = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_recv < 0){
        perror("error receiving data");
        free(buffer);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    buffer[bytes_recv] = '\0';
    xor_cipher(buffer, shared_secret, strlen(buffer));
    printf("Server response: %s\n", buffer);

    free(buffer);
    close(sockfd);
    return EXIT_SUCCESS;
}

long exchange_dh(int client){
    long pub = mod_exp(G, KEY, P);

    // ** send pub key to server ** //
    send(client, &pub, sizeof(pub), 0);

    // ** recv server's pub key ** //
    long server_pub;
    if (recv(client, &server_pub, sizeof(server_pub), 0) < 0) {
        perror("error receiving server's public key");
        return EXIT_FAILURE;
    }

    // ** compute shared secret ** //
    long shared_secret = mod_exp(server_pub, KEY, P);
    return shared_secret;
}