#include <stdio.h>
#include <stdlib.h>

void generate_token(char* cp, unsigned long size) {
    for (unsigned long i = 0; i < size - 1; i++){
        sprintf(cp + (i*2), "%02x", rand() % 256);
    }
    cp[size - 1] = '\0';
}