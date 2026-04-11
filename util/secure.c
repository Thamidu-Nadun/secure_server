#include <unistd.h>

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

void xor_cipher(char* data, const char key, int len){
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
    xor_cipher(buff, shared_secret, len);
    return write(sockfd, buff, len);
}