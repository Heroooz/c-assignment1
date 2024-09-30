#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "memory_manager.h"

unsigned char* start;
unsigned char* end;
static void* memory_pool = NULL;
size_t size_of_pool;

void set_bit(unsigned char* arr, size_t index) {
    arr[index / 8] |= (1 << (index % 8));
}

void clear_bit(unsigned char* arr, size_t index) {
    arr[index / 8] &= ~(1 << (index % 8));
}

bool get_bit(unsigned char* arr, size_t index) {
    return (arr[index / 8] & (1 << (index % 8)));
}

// Initialization function: creates a memory pool of the given size
void mem_init(size_t size) {
    memory_pool = malloc(size);  // Allocate memory pool
    start = calloc((size + 7) / 8, 1);  // Allocate bit array for start
    end = calloc((size + 7) / 8, 1);    // Allocate bit array for end
    size_of_pool = size;
}

// Allocation function: finds the first free block that fits the requested size
void* mem_alloc(size_t size) {
    if (size == 0 || size > size_of_pool) {
        return NULL;  // Cannot allocate zero bytes or more than the pool size
    }

    // Find the first free block that fits the requested size
    for (size_t i = 0; i <= size_of_pool - size; i++) {
        bool block_free = true;
        for (size_t j = 0; j < size; j++) {
            if (get_bit(start, i + j)) {
                block_free = false;
                break;
            }
        }

        if (block_free) {
            // Mark the block as used
            for (size_t j = 0; j < size; j++) {
                set_bit(start, i + j);
            }
            return (void*)((unsigned char*)memory_pool + i);
        }
    }

    return NULL;  // No free block found
}


// Deallocation function: marks a block as free
void mem_free(void* block) {
    size_t offset = (unsigned char*)block - (unsigned char*)memory_pool;
    if (block == NULL || offset >= size_of_pool) {
        return;  // Nothing to do || Invalid block
    }

    // Mark the block as free
    for (size_t i = offset; i < size_of_pool && get_bit(start, i); i++) {
        clear_bit(start, i);
    }
}

// Resize function: changes the size of the memory block, possibly moving it
void* mem_resize(void* block, size_t size) {
    if (block == NULL) {
        return mem_alloc(size);  // Allocate a new block
    }

    size_t offset = (unsigned char*)block - (unsigned char*)memory_pool;
    if (offset >= size_of_pool) {
        return NULL;  // Invalid block
    }

    if (size == 0) {
        mem_free(block);
        return NULL;  // Free the block
    }

    // Find the size of the current block
    size_t current_size = 0;
    for (size_t i = offset; i < size_of_pool && start[i] == 1; i++) {
        current_size++;
    }
    if (current_size >= size) {
        return block;  // No need to resize
    }

    // Allocate a new block with the new size
    void* new_block = mem_alloc(size);
    if (new_block == NULL) {
        return NULL;  // Allocation failed
    }

    // Copy the contents of the old block to the new block
    memcpy(new_block, block, current_size);

    // Free the old block
    mem_free(block);

    return new_block;
}

// Deinit function: frees the memory pool and resets state
void mem_deinit() {
    free(end);
    free(start);
    free(memory_pool);
    memory_pool = NULL;
    size_of_pool = 0;
}