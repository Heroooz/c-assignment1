#include "common_defs.h"
#include "memory_manager.c"

void test_mem_init_deinit() {
    printf_yellow("Testing mem_init and mem_deinit...\n");

    // Test initialization with a valid size
    mem_init(1024);  // Initialize memory manager with 1024 bytes
    printf_green("Memory initialized with 1024 bytes.\n");

    // Test de-initialization
    mem_deinit();
    printf_green("Memory deinitialized successfully.\n");
}

void test_mem_alloc_free() {
    printf_yellow("Testing mem_alloc and mem_free...\n");

    mem_init(1024);  // Initialize with 1024 bytes

    // Allocate a small block
    void *block = mem_alloc(128);
    my_assert(block != NULL);  // Ensure allocation succeeds
    printf_green("Allocated 128 bytes successfully.\n");

    // Allocate a larger block
    void *block2 = mem_alloc(256);
    my_assert(block2 != NULL);  // Ensure allocation succeeds
    printf_green("Allocated 256 bytes successfully.\n");

    // Free the first block
    mem_free(block);
    printf_green("Freed 128-byte block successfully.\n");

    // Free the second block
    mem_free(block2);
    printf_green("Freed 256-byte block successfully.\n");

    mem_deinit();  // Cleanup
}

void test_mem_alloc_insufficient_space() {
    printf_yellow("Testing mem_alloc with insufficient space...\n");

    mem_init(512);  // Initialize with 512 bytes

    // Allocate a block larger than available space
    void *block = mem_alloc(1024);
    my_assert(block == NULL);  // Allocation should fail
    printf_green("Allocation failed as expected due to insufficient space.\n");

    mem_deinit();  // Cleanup
}

void test_mem_free_null() {
    printf_yellow("Testing mem_free with NULL...\n");

    mem_init(1024);  // Initialize with 1024 bytes

    // Freeing a NULL pointer should not crash the program
    mem_free(NULL);
    printf_green("Freeing NULL did not cause any issues.\n");

    mem_deinit();  // Cleanup
}

void test_mem_resize_shrink() {
    printf_yellow("Testing mem_resize (shrink)...\n");

    mem_init(1024);  // Initialize with 1024 bytes

    void *block = mem_alloc(256);
    my_assert(block != NULL);  // Ensure allocation succeeds
    printf_green("Allocated 256 bytes successfully.\n");

    // Resize the block to a smaller size
    block = mem_resize(block, 128);
    my_assert(block != NULL);  // Ensure resize succeeds
    printf_green("Resized block to 128 bytes successfully.\n");

    mem_free(block);
    mem_deinit();  // Cleanup
}

void test_mem_resize_expand() {
    printf_yellow("Testing mem_resize (expand)...\n");

    mem_init(1024);  // Initialize with 1024 bytes

    void *block = mem_alloc(256);
    my_assert(block != NULL);  // Ensure allocation succeeds
    printf_green("Allocated 256 bytes successfully.\n");

    // Resize the block to a larger size
    block = mem_resize(block, 512);
    my_assert(block != NULL);  // Ensure resize succeeds
    printf_green("Resized block to 512 bytes successfully.\n");

    mem_free(block);
    mem_deinit();  // Cleanup
}

void test_mem_resize_insufficient_space() {
    printf_yellow("Testing mem_resize with insufficient space...\n");

    mem_init(512);  // Initialize with 512 bytes

    void *block = mem_alloc(256);
    my_assert(block != NULL);  // Ensure allocation succeeds
    printf_green("Allocated 256 bytes successfully.\n");

    // Try resizing beyond available memory
    void *new_block = mem_resize(block, 1024);
    my_assert(new_block == NULL);  // Resize should fail
    printf_green("Resize failed as expected due to insufficient space.\n");

    mem_free(block);
    mem_deinit();  // Cleanup
}

void test_mem_alloc_all_memory() {
    printf_yellow("Testing mem_alloc to consume all memory...\n");

    mem_init(512);  // Initialize with 512 bytes

    // Allocate exactly 512 bytes
    void *block = mem_alloc(512);
    my_assert(block != NULL);  // Ensure allocation succeeds
    printf_green("Allocated exactly 512 bytes successfully.\n");

    // Attempt to allocate more memory, which should fail
    void *block2 = mem_alloc(1);
    my_assert(block2 == NULL);  // Allocation should fail
    printf_green("Allocation failed as expected when out of memory.\n");

    mem_free(block);
    mem_deinit();  // Cleanup
}

void test_mem_alloc_zero_size() {
    printf_yellow("Testing mem_alloc with zero size...\n");

    mem_init(1024);  // Initialize with 1024 bytes

    // Allocate zero bytes, which should return NULL or handle gracefully
    void *block = mem_alloc(0);
    my_assert(block == NULL);  // Should fail or return NULL
    printf_green("Allocation of zero bytes handled correctly.\n");

    mem_deinit();  // Cleanup
}

void test_mem_resize_null_pointer() {
    printf_yellow("Testing mem_resize with a NULL pointer...\n");

    mem_init(1024);  // Initialize with 1024 bytes

    // Resizing a NULL pointer should behave like mem_alloc
    void *block = mem_resize(NULL, 128);
    my_assert(block != NULL);  // Should allocate a new block
    printf_green("Resized NULL pointer resulted in a successful allocation.\n");

    mem_free(block);
    mem_deinit();  // Cleanup
}

void test_mem_free_after_deinit() {
    printf_yellow("Testing mem_free after mem_deinit...\n");

    mem_init(1024);  // Initialize with 1024 bytes

    void *block = mem_alloc(256);
    my_assert(block != NULL);  // Ensure allocation succeeds

    mem_deinit();  // Deinitialize memory manager

    // Freeing after deinit should not crash or cause undefined behavior
    mem_free(block);
    printf_green("Freed block after deinit without any issues.\n");
}

void test_double_free() {
    printf_yellow("Testing double free...\n");

    mem_init(1024);  // Initialize with 1024 bytes

    // Allocate a block
    void *block = mem_alloc(128);
    my_assert(block != NULL);  // Ensure allocation succeeds

    // Free the block once
    mem_free(block);
    printf_green("Freed block successfully.\n");

    // Free the block again
    mem_free(block);
    printf_green("Double free handled gracefully.\n");

    mem_deinit();  // Cleanup
}

void test_memory_fragmentation() {
    printf_yellow("Testing memory fragmentation...\n");

    mem_init(1024);  // Initialize with 1024 bytes

    // Allocate and free multiple blocks to fragment the memory
    void *block1 = mem_alloc(128);
    void *block2 = mem_alloc(256);
    void *block3 = mem_alloc(128);

    my_assert(block1 != NULL && block2 != NULL && block3 != NULL);

    // Free two blocks to create gaps
    mem_free(block1);
    mem_free(block3);
    printf_green("Freed two blocks to create fragmentation.\n");

    // Try to allocate a block that should fit into the fragmented space
    void *block4 = mem_alloc(128);
    my_assert(block4 != NULL);
    printf_green("Successfully allocated block in fragmented memory.\n");

    // Cleanup
    mem_free(block2);
    mem_free(block4);
    mem_deinit();
}

void test_large_alloc_exceeding_size() {
    printf_yellow("Testing large allocation exceeding memory size...\n");

    mem_init(1024);  // Initialize with 1024 bytes

    // Attempt to allocate a very large block, more than available memory
    void *block = mem_alloc(2048);
    my_assert(block == NULL);  // Allocation should fail
    printf_green(
        "Large allocation exceeding memory size failed as expected.\n");

    mem_deinit();  // Cleanup
}

void test_alloc_boundary_conditions() {
    printf_yellow("Testing allocation at boundary conditions...\n");

    mem_init(1024);  // Initialize with 1024 bytes

    // Allocate exactly all available memory
    void *block1 = mem_alloc(1024);
    my_assert(block1 != NULL);
    printf_green("Successfully allocated memory at boundary (1024 bytes).\n");

    // Free the memory
    mem_free(block1);

    // Try allocating a small amount at the boundary
    void *block2 = mem_alloc(1024 - 1);
    my_assert(block2 != NULL);
    printf_green(
        "Successfully allocated one byte less than total memory (1023 "
        "bytes).\n");

    // Try allocating the remaining 1 byte
    void *block3 = mem_alloc(1);
    my_assert(block3 != NULL);
    printf_green("Successfully allocated remaining 1 byte.\n");

    // Cleanup
    mem_free(block2);
    mem_free(block3);
    mem_deinit();
}

void test_mem_resize_to_zero() {
    printf_yellow("Testing mem_resize to zero...\n");

    mem_init(1024);  // Initialize with 1024 bytes

    // Allocate a block
    void *block = mem_alloc(256);
    my_assert(block != NULL);  // Ensure allocation succeeds

    // Resize the block to zero
    void *resized_block = mem_resize(block, 0);
    my_assert(resized_block ==
              NULL);  // Should either return NULL or deallocate
    printf_green("Resized block to zero bytes successfully.\n");

    // Clean up (if resize didn't free, free manually)
    if (resized_block != NULL) {
        mem_free(resized_block);
    }

    mem_deinit();  // Cleanup
}

void test_free_invalid_pointer() {
    printf_yellow("Testing free of invalid pointer...\n");

    mem_init(1024);  // Initialize with 1024 bytes

    // Allocate a block
    void *block = mem_alloc(256);
    my_assert(block != NULL);  // Ensure allocation succeeds

    // Attempt to free a pointer that was never allocated
    int dummy;
    mem_free(&dummy);
    printf_green("Freeing invalid pointer handled gracefully.\n");

    // Cleanup
    mem_free(block);
    mem_deinit();
}

void test_memory_overwrite() {
    printf_yellow("Testing memory overwrite protection...\n");

    mem_init(1024);  // Initialize with 1024 bytes

    // Allocate two blocks
    char *block1 = (char *)mem_alloc(128);
    char *block2 = (char *)mem_alloc(128);

    my_assert(block1 != NULL && block2 != NULL);  // Ensure allocations succeed

    // Write data to the first block
    for (int i = 0; i < 128; i++) {
        block1[i] = 'A';
    }

    // Verify that the second block is untouched
    for (int i = 0; i < 128; i++) {
        my_assert(block2[i] == 0);  // Assuming memory is initialized to zero
    }
    printf_green(
        "Memory overwrite protection successful, block2 remained intact.\n");

    // Cleanup
    mem_free(block1);
    mem_free(block2);
    mem_deinit();
}

int main() {
    // Run all tests
    test_mem_init_deinit();
    test_mem_alloc_free();
    test_mem_alloc_insufficient_space();
    test_mem_free_null();
    test_mem_resize_shrink();
    test_mem_resize_expand();
    test_mem_resize_insufficient_space();
    test_mem_alloc_all_memory();
    test_mem_alloc_zero_size();
    test_mem_resize_null_pointer();
    test_mem_free_after_deinit();

    // Additional tests
    test_double_free();
    test_memory_fragmentation();
    test_large_alloc_exceeding_size();
    test_alloc_boundary_conditions();
    test_mem_resize_to_zero();
    test_free_invalid_pointer();
    test_memory_overwrite();

    printf_green("All tests completed successfully.\n");
    return 0;
}