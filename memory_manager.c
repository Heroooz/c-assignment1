#include "memory_manager.h"

typedef struct memory_block {
    void *start;
    void *end;
    struct memory_block *next;
} memory_block;

memory_block *memory_block_init(void *start, void *end, memory_block *next) {
    memory_block *new_block = malloc(sizeof(*new_block));
    new_block->start = start;
    new_block->end = end;
    new_block->next = next;
    return new_block;
}

memory_block *head;
void *memory_pool;
size_t size_of_pool;

pthread_mutex_t memory_mutex;

// Initialization function: creates a memory pool of the given size
void mem_init(size_t size) {
    head = NULL;
    memory_pool = malloc(size);
    size_of_pool = size;
    pthread_mutex_init(&memory_mutex, NULL);
}

// Allocation function: finds the first free block that fits the requested size
void* mem_alloc(size_t size) {
    if (size > size_of_pool) {
        return NULL; // Cannot allocate more than the pool size
    }
    if (size == 0) {
        return memory_pool; // Return the start of the memory pool
    }
    pthread_mutex_lock(&memory_mutex);

    // Insertion first
    if (head == NULL || head->start - memory_pool >= size) {
        memory_block *new_block = memory_block_init(memory_pool, memory_pool + size, head);
        head = new_block;
        pthread_mutex_unlock(&memory_mutex);
        return memory_pool;
    }

    // Insertion between blocks or last
    memory_block *walker = head;
    while (walker != NULL) {
        size_t space = (walker->next) ? walker->next->start - walker->end : memory_pool + size_of_pool - walker->end;
        if (space >= size) {
            memory_block *new_block = memory_block_init(walker->end, walker->end + size, walker->next);
            walker->next = new_block;
            void *return_ptr = walker->end;
            pthread_mutex_unlock(&memory_mutex);
            return return_ptr;
        }
        walker = walker->next;
    }
    pthread_mutex_unlock(&memory_mutex);
    return NULL;
}

// Allocation function: finds the first free block that fits the requested size
void* mem_alloc_without_locks(size_t size) {
    if (size > size_of_pool) {
        return NULL; // Cannot allocate more than the pool size
    }
    if (size == 0) {
        return memory_pool; // Return the start of the memory pool
    }

    // Insertion first
    if (head == NULL || head->start - memory_pool >= size) {
        memory_block *new_block = memory_block_init(memory_pool, memory_pool + size, head);
        head = new_block;
        return memory_pool;
    }

    // Insertion between blocks or last
    memory_block *walker = head;
    while (walker != NULL) {
        size_t space = (walker->next) ? walker->next->start - walker->end : memory_pool + size_of_pool - walker->end;
        if (space >= size) {
            memory_block *new_block = memory_block_init(walker->end, walker->end + size, walker->next);
            walker->next = new_block;
            void *return_ptr = walker->end;
            return return_ptr;
        }
        walker = walker->next;
    }
    return NULL;
}

// Deallocation function: marks a block as free
void mem_free(void* block) {
    pthread_mutex_lock(&memory_mutex);
    if (!head) {
        pthread_mutex_unlock(&memory_mutex);
        return;
    }
    
    if (head->start == block) { 
        memory_block *temp = head;
        head = head->next;
        free(temp);
        pthread_mutex_unlock(&memory_mutex);
        return;
    }
    
    memory_block *walker = head;
    while (walker->next != NULL) {
        if (walker->next->start == block) {
            memory_block *temp = walker->next;
            walker->next = temp->next;
            free(temp);
            pthread_mutex_unlock(&memory_mutex);
            return;
        }
        walker = walker->next;
    }
    pthread_mutex_unlock(&memory_mutex);
    return;
}

// Resize function: changes the size of the memory block, possibly moving it
void* mem_resize(void* block, size_t size) {
    if (size > size_of_pool) {
        return NULL; // Cannot resize to a size larger than the pool
    }

    if (!block) {
        return mem_alloc(size); // Allocate a new block
    }

    if (size == 0) {
        mem_free(block);
        return NULL; // Free the block
    }
    pthread_mutex_lock(&memory_mutex);

    // Find the block to resize
    memory_block *before_node = NULL;
    memory_block *node = head;
    while (node != NULL && node->start != block) {
        before_node = node;
        node = node->next;
    }

    if (!node) {
        pthread_mutex_unlock(&memory_mutex);
        return NULL; // Block not found
    }

    if (before_node) {
        before_node->next = node->next;
    }
    else {
        head = head->next;
    }

    void *newblock = mem_alloc_without_locks(size); // Allocate a new block with the new size

    if (!newblock) {
        if (before_node)
            before_node->next = node;
        else
            head = node;
        pthread_mutex_unlock(&memory_mutex);
        return NULL; // Allocation failed
    }

    // Copy the data from the old block to the new block and free the old block
    size_t old_size = node->end - node->start;
    free(node);
    memcpy(newblock, block, (old_size < size) ? old_size : size);
    pthread_mutex_unlock(&memory_mutex);
    return newblock;
}

// Deinit function: frees the memory pool and resets state
void mem_deinit() {
    memory_block *walker = head;
    while (walker != NULL) {
        memory_block *temp = walker;
        walker = walker->next;
        free(temp);
    }
    free(memory_pool);
    size_of_pool = 0;
    head = NULL;
    pthread_mutex_destroy(&memory_mutex);
}