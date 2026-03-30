#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void* allocate_mem(size_t size){
    void *ptr = malloc(size);
    memset(ptr, 0, size);
    if (ptr == NULL){
        fprintf(stderr, "Memory allocation failed for size %zu\n", size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void* reallocate_mem(void* ptr, size_t size){
    void* new_ptr = realloc(ptr, size);
    if (new_ptr == NULL){
        fprintf(stderr, "Memory reallocation failed for size %zu\n", size);
        free(ptr);
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}