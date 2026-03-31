#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * This function allocates memory of the specified size and initializes it to zero.
 * If the allocation fails, it prints an error message and exits the program.
 * @param size the size of memory to allocate
 * @return a pointer to the allocated memory
 * @author Nadun
 */
void* allocate_mem(size_t size){
    void *ptr = malloc(size);
    memset(ptr, 0, size);
    if (ptr == NULL){
        fprintf(stderr, "Memory allocation failed for size %zu\n", size);
        free(ptr);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

/**
 * This function reallocates memory to the specified size. 
 * If the reallocation fails, it prints an error message, frees the original memory, 
 * and exits the program.
 * @param ptr the pointer to the original memory block
 * @param size the new size of memory to allocate
 * @return a pointer to the reallocated memory
 * @note provide size that is larger than the original size to avoid data loss.
 * @author Nadun
 */
void* reallocate_mem(void* ptr, size_t size){
    void* new_ptr = realloc(ptr, size);
    if (new_ptr == NULL){
        fprintf(stderr, "Memory reallocation failed for size %zu\n", size);
        free(ptr);
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}