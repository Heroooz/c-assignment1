#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mem_init(size_t size);

void* mem_alloc(size_t size);

void mem_free(void* block);

void* mem_resize(void* block, size_t size);

void mem_deinit();

typedef struct BlockHeader {
    size_t size;            // Size of the block (including header)
    bool free;              // Whether the block is free or allocated
    struct BlockHeader* next; // Pointer to the next block in the memory pool
} BlockHeader;

#endif