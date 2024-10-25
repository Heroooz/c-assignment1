#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#include "memory_manager.h"

unsigned char* start;
unsigned char* end;
static void* memory_pool = NULL;
size_t size_of_pool;
pthread_mutex_t memory_mutex;

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
    pthread_mutex_init(&memory_mutex, NULL);
    pthread_mutex_lock(&memory_mutex);
    memory_pool = malloc(size);  // Allocate memory pool
    start = calloc((size + 7) / 8, 1);  // Allocate bit array for start
    end = calloc((size + 7) / 8, 1);    // Allocate bit array for end
    size_of_pool = size;
    pthread_mutex_unlock(&memory_mutex);
}

// Allocation function: finds the first free block that fits the requested size
void* mem_alloc(size_t size) {
    pthread_mutex_lock(&memory_mutex);
    if (size == 0) {
        pthread_mutex_unlock(&memory_mutex);
        return memory_pool;  // Return the start of the memory pool
    }

    size_t amount_of_empty_space = 0;
    bool block_free = true;
    // Find the first free block that fits the requested size
    for (size_t i = 0; i < size_of_pool; i++) {
        if (get_bit(start, i)) {
            block_free = false;
        }

        if (block_free){
            amount_of_empty_space++;
        }
        else{
            amount_of_empty_space = 0;
        }

        if (amount_of_empty_space == size) {
            // Mark the block as used
            set_bit(end, i);
            set_bit(start, i + 1 - size);
            pthread_mutex_unlock(&memory_mutex);
            return (void*)((unsigned char*)memory_pool + i + 1 - size);
        }

        if (get_bit(end, i)) {
            block_free = true;
        }
    }

    pthread_mutex_unlock(&memory_mutex);
    return NULL;  // No free block found
}

// Deallocation function: marks a block as free
void mem_free(void* block) {
    pthread_mutex_lock(&memory_mutex);
    size_t offset = (unsigned char*)block - (unsigned char*)memory_pool;
    if (block == NULL || offset >= size_of_pool) {
        pthread_mutex_unlock(&memory_mutex);
        return;  // Nothing to do || Invalid block
    }

    // Mark the block as free
    for (size_t i = offset; i < size_of_pool && get_bit(start, i); i++) {
        clear_bit(start, i);
    }

    pthread_mutex_unlock(&memory_mutex);
}

// Resize function: changes the size of the memory block, possibly moving it
void* mem_resize(void* block, size_t size) {
    pthread_mutex_lock(&memory_mutex);
    if (block == NULL) {
        void* new_block = mem_alloc(size);
        pthread_mutex_unlock(&memory_mutex);
        return new_block;  // Allocate a new block
    }

    size_t offset = (unsigned char*)block - (unsigned char*)memory_pool;
    if (offset >= size_of_pool) {
        pthread_mutex_unlock(&memory_mutex);
        return NULL;  // Invalid block
    }

    if (size == 0) {
        mem_free(block);
        pthread_mutex_unlock(&memory_mutex);
        return NULL;  // Free the block
    }

    // Find the size of the current block
    size_t current_size = 0;
    for (size_t i = offset; i < size_of_pool && get_bit(start, i); i++) {
        current_size++;
    }
    if (current_size >= size) {
        pthread_mutex_unlock(&memory_mutex);
        return block;  // No need to resize
    }

    // Allocate a new block with the new size
    void* new_block = mem_alloc(size);
    if (new_block == NULL) {
        pthread_mutex_unlock(&memory_mutex);
        return NULL;  // Allocation failed
    }

    // Copy the contents of the old block to the new block
    memcpy(new_block, block, current_size);

    // Free the old block
    mem_free(block);

    pthread_mutex_unlock(&memory_mutex);
    return new_block;
}

// Deinit function: frees the memory pool and resets state
void mem_deinit() {
    pthread_mutex_lock(&memory_mutex);
    free(end);
    free(start);
    free(memory_pool);
    memory_pool = NULL;
    size_of_pool = 0;
    pthread_mutex_unlock(&memory_mutex);
    pthread_mutex_destroy(&memory_mutex);
}