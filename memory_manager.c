#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// make test_mmanager
// make run_test_mmanager
// make clean

typedef struct BlockHeader {
    size_t size;            // Size of the block (including header)
    bool free;              // Whether the block is free or allocated
    struct BlockHeader* next; // Pointer to the next block in the memory pool
} BlockHeader;

static void* memory_pool = NULL;
static BlockHeader* free_list = NULL;  // List of free/available blocks

// Align memory to a multiple of this size (for better memory usage)
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

// Initialization function: creates a memory pool of the given size
void mem_init(size_t size) {
    size = ALIGN(size);  // Align the size
    memory_pool = malloc(size);  // Allocate memory pool
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
    size = ALIGN(size);  // Align the size
    BlockHeader* current = free_list;
    BlockHeader* previous = NULL;

    // Traverse the free list to find the first block that fits
    while (current != NULL) {
        if (current->free && current->size >= size + sizeof(BlockHeader)) {
            // Split the block if it's too large
            if (current->size > size + sizeof(BlockHeader)) {
                BlockHeader* new_block = (BlockHeader*)((char*)current + sizeof(BlockHeader) + size);
                new_block->size = current->size - size - sizeof(BlockHeader);
                new_block->free = true;
                new_block->next = current->next;
                
                current->next = new_block;
                current->size = size + sizeof(BlockHeader);
            }
            current->free = false;
            return (char*)current + sizeof(BlockHeader);  // Return pointer to the usable memory
        }
        previous = current;
        current = current->next;
    }

    // If we reach here, no suitable block was found
    return NULL;
}

// Deallocation function: marks a block as free
void mem_free(void* block) {
    if (block == NULL) {
        return;
    }

    BlockHeader* header = (BlockHeader*)((char*)block - sizeof(BlockHeader));
    header->free = true;

    // Coalesce free blocks to prevent fragmentation
    BlockHeader* current = free_list;
    while (current != NULL && current->next != NULL) {
        if (current->free && current->next->free) {
            current->size += current->next->size + sizeof(BlockHeader);
            current->next = current->next->next;
        }
        current = current->next;
    }
}

// Resize function: changes the size of the memory block, possibly moving it
void* mem_resize(void* block, size_t size) {
    if (block == NULL) {
        return mem_alloc(size);  // If block is NULL, allocate a new block
    }

    BlockHeader* header = (BlockHeader*)((char*)block - sizeof(BlockHeader));
    size = ALIGN(size);

    // If the new size fits in the existing block, keep it
    if (header->size >= size + sizeof(BlockHeader)) {
        return block;
    }

    // Otherwise, allocate a new block and copy the data
    void* new_block = mem_alloc(size);
    if (new_block != NULL) {
        memcpy(new_block, block, header->size - sizeof(BlockHeader));
        mem_free(block);
    }
    return new_block;
}

// Deinit function: frees the memory pool and resets state
void mem_deinit() {
    free(memory_pool);
    memory_pool = NULL;
    free_list = NULL;
}
