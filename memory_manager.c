#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "memory_manager.h"

// make test_mmanager
// make run_test_mmanager
// make clean

static void* memory_pool = NULL;
static BlockHeader* free_list = NULL;  // List of free/available blocks

// Initialization function: creates a memory pool of the given size
void mem_init(size_t size) {
    memory_pool = malloc(size + 4 * sizeof(BlockHeader));  // Allocate memory pool
    if (memory_pool == NULL) {
        fprintf(stderr, "Failed to initialize memory pool.\n");
        exit(1);
    }

    // Create the first block, which covers the entire pool and is free
    free_list = (BlockHeader*)memory_pool;
    free_list->size = size;
    free_list->free = true;
    free_list->next = NULL;
}

// Allocation function: finds the first free block that fits the requested size
void* mem_alloc(size_t size) {
    if (size == 0) {
        return NULL;  // No need to allocate zero bytes
    }

    BlockHeader* current = free_list;

    // Find the first free block that fits the requested size
    while (current != NULL) {
        if (current->free && current->size >= size) {
            // Split the block if it's larger than the requested size
            if (current->size > size + sizeof(BlockHeader)) {
                BlockHeader* new_block = (BlockHeader*)((char*)current + size + sizeof(BlockHeader));
                new_block->size = current->size - size - sizeof(BlockHeader);
                new_block->free = true;
                new_block->next = current->next;
                current->size = size + sizeof(BlockHeader);
                current->next = new_block;
            }
            current->free = false;
            return (void*)(current + 1);  // Return a pointer to the block data
        }
        current = current->next;
    }

    return NULL;  // No free block found
}

// Deallocation function: marks a block as free
void mem_free(void* block) {
    if (block == NULL) {
        return;  // Nothing to do
    }

    BlockHeader* header = (BlockHeader*)((char*)block - sizeof(BlockHeader));
    header->free = true;

    // Merge adjacent free blocks
    BlockHeader* current = free_list;
    while (current != NULL) {
        if (current->free) {
            while (current->next != NULL && current->next->free) {
                current->size += current->next->size + sizeof(BlockHeader);
                current->next = current->next->next;
            }
        }
        current = current->next;
    }
}

// Resize function: changes the size of the memory block, possibly moving it
void* mem_resize(void* block, size_t size) {
    if (block == NULL) {
        return mem_alloc(size);  // Allocate a new block
    }

    if (size == 0) {
        mem_free(block);
        return NULL;  // Free the block
    }

    BlockHeader* header = (BlockHeader*)((char*)block - sizeof(BlockHeader));
    //size = ALIGN(size);  // Align the size
    if (header->size >= size) {
        return block;  // No need to resize
    }

    // Allocate a new block with the new size
    void* new_block = mem_alloc(size);
    if (new_block == NULL) {
        return NULL;  // Allocation failed
    }

    // Copy the contents of the old block to the new block
    memcpy(new_block, block, header->size - sizeof(BlockHeader));
    // Free the old block
    mem_free(block);
    return new_block;
}

// Deinit function: frees the memory pool and resets state
void mem_deinit() {
    free(memory_pool);
    memory_pool = NULL;
    free_list = NULL;
}
