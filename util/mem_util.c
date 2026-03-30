#include <stdlib.h>
#include <stdio.h>

void* allocate_mem(size_t size){
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Memory allocation failed for size %zu\n", size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}