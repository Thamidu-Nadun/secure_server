#ifndef SECURE_H
#define SECURE_H

#include <unistd.h>
#include <openssl/sha.h>

long mod_exp(long base, long exp, long mod) {
    long result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = (result * base) % mod;
        }
        exp = exp >> 1; // exp = exp / 2
        base = (base * base) % mod;
    }
    return result;
}

void xor_cipher(unsigned char* data, unsigned char key, int len){
    for (int i = 0; i < len; i++) {
        data[i] ^= key;
    }
}

/**
 * encrypt data using XOR cipher and send it securely over the socket.
 * Write N bytes of BUF to FD. Return the number written, or -1.
 * This function is a cancellation point and therefore not marked with __THROW.
 */
int secure_send(int sockfd, char* buff, int len, long shared_secret){
    unsigned char key = (unsigned char)(shared_secret & 0xFF); 
    xor_cipher((unsigned char*)buff, shared_secret, len);
    return write(sockfd, buff, len);
}


void sha256(const char* str, char outputBuffer[65]) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char *)str, strlen(str), hash);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = '\0';
}

/**
 * Create a salted hash of the password
 */
void create_password_hash(const char* password, const char* salt, char* outp) {
    char salted_password[256];
    snprintf(salted_password, sizeof(salted_password), "%s%s", password, salt);
    sha256(salted_password, outp);
}

#endif