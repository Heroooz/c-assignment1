#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common_defs.h"
#include "memory_manager.h"

typedef struct block {
    int *block;
    size_t size;
} block;

void test_memory_validity_(block *block_array, size_t nrOf_blocks) {
    printf_yellow("  Validating block(s)\n");

    printf_yellow("  Checking if allocation was successful\n");
    for (size_t i = 0; i < nrOf_blocks; i++) {
        if (!(block_array[i].block) && block_array[i].size != 0) {
            printf_red("  -Block at index %ld allocation failed!\n", i);
            my_assert(false);
        }
    }
    printf_green("  -Successfull\n");

    printf_yellow("  Filling all blocks and checking expected values\n");

    size_t counter = 0;
    for (size_t i = 0; i < nrOf_blocks; i++) {
        for (size_t j = 0; j < block_array[i].size; j++) {
            block_array[i].block[j] = counter++;
        }
    }
    counter = 0;
    for (size_t i = 0; i < nrOf_blocks; i++) {
        for (size_t j = 0; j < block_array[i].size; j++) {
            if (block_array[i].block[j] != counter) {
                printf_red(
                    "  -Block %ld did not contain the expected value at index "
                    "%ld, expected vale: %ld, actual value: %d, most likely "
                    "due to memory-block overlap\n",
                    j, i, counter, block_array[i].block[j]);
                my_assert(false);
            }
            counter++;
        }
    }
    printf_green("  -Successfull validation passed\n");
}

void block_free_all(block *blocks, size_t nrOf_blocks) {
    for (size_t i = 0; i < nrOf_blocks; i++)
        if (blocks[i].size) mem_free(blocks[i].block);
}

void block_init(block *block, size_t size) {
    block->block = mem_alloc(size * sizeof(*(block->block)));
    block->size = size;
}

size_t block_free(block *block) {
    mem_free(block->block);
    size_t temp = block->size;
    block->size = 0;
    return temp;
}

block block_resize(block block_, size_t size) {
    block new_block;
    new_block.block = mem_resize(block_.block, size * sizeof(*(block_.block)));
    new_block.size = size;
    return new_block;
}

void mem_free_all(int **blocks, size_t nrOf_blocks) {
    for (size_t i = 0; i < nrOf_blocks; i++) mem_free(blocks[i]);
}

void test_init() {
    printf_yellow("Testing mem_init...\n");
    static const size_t block_size = 128;
    static const size_t nrOf_blocks = 3;
    // Initialize with block_size * 3) bytes of memory
    // Try allocating to check if init was successful (block_size * 4 bytes)

    block blocks[nrOf_blocks];
    mem_init(sizeof(*(blocks[0].block)) * block_size * nrOf_blocks);
    for (size_t i = 0; i < nrOf_blocks; i++) block_init(&blocks[i], block_size);

    // Test that allocation was successfull
    test_memory_validity_(blocks, 3);

    block_free_all(blocks, nrOf_blocks);
    mem_deinit();
    printf_green("mem_init passed.\n");
}

void test_alloc_and_free() {
    srand(0);
    printf_yellow("Testing mem_alloc and mem_free...\n");
    static const size_t nrOf_blocks = 16;
    static const size_t block_size = 32;

    block blocks[nrOf_blocks];
    static const size_t memory_size =
        nrOf_blocks * sizeof(*(blocks[0].block)) * block_size;

    printf_yellow("  Trying to create %ld blocks of size %ld\n", nrOf_blocks,
                  block_size);
    mem_init(memory_size);
    for (size_t i = 0; i < nrOf_blocks; i++) {
        block_init(&blocks[i], block_size);
        my_assert(blocks[i].block != NULL);
    }
    printf_green("  -Successfull\n");

    test_memory_validity_(blocks, nrOf_blocks);

    printf_yellow("  Trying allocation one over size limit\n");
    void *extra_block = mem_alloc(1);
    my_assert(extra_block == NULL &&
              "  -Allocation over size limit did not fail when it should have");
    printf_green("  -Passed\n");

    printf_yellow("  Freeing all blocks and attempting reallocation\n");
    for (size_t i = 0; i < nrOf_blocks; i++) block_free(&blocks[i]);

    block_init(&blocks[0], block_size * nrOf_blocks);
    my_assert(blocks[0].block != NULL &&
              "  -Memory reallocation failed, mem_free likely did not free all "
              "memory properly");
    printf_green("  -Successfull\n");

    printf_yellow("  Filling reallocated space\n");
    for (size_t i = 0; i < block_size * nrOf_blocks; i++)
        blocks[0].block[i] = i;
    printf_green("  -Successfull\n");

    block_free(&blocks[0]);

    printf_yellow("  Testing allocation of size 0\n");
    my_assert(mem_alloc(0) != NULL && "mem_alloc(0) did not return NULL");
    printf_green("  -Successfull\n");

    printf_yellow("  Testing allocation of randomly sized blocks\n");
    size_t remaining_space = nrOf_blocks * block_size;
    for (size_t i = 0; i < (nrOf_blocks - 1); i++) {
        size_t current_block_size = block_size - rand() % (block_size / 5);
        block_init(&blocks[i], current_block_size);
        remaining_space -= current_block_size;
    }
    block_init(&blocks[nrOf_blocks - 1], remaining_space);
    remaining_space = 0;
    test_memory_validity_(blocks, nrOf_blocks);
    printf_green("  -Successfull\n");

    printf_yellow("  Testing validity of memory after freeing some blocks\n");
    remaining_space += block_free(&blocks[nrOf_blocks / 3]);
    remaining_space += block_free(&blocks[nrOf_blocks / 3 + 1]);
    remaining_space += block_free(&blocks[nrOf_blocks / 3 + 2]);

    test_memory_validity_(blocks, nrOf_blocks);
    printf_green("  -Successfull\n");

    printf_yellow(
        "  Testing validity of memory after refilling leftover space\n");

    block_init(&blocks[nrOf_blocks / 3], remaining_space);

    test_memory_validity_(blocks, nrOf_blocks);
    printf_green("  -Successfull\n");

    block_free_all(blocks, nrOf_blocks);

    mem_deinit();
    printf_green("mem_alloc and mem_free passed.\n");
}

void test_resize() {
    printf_yellow("Testing mem_resize...\n");
    static const size_t block_size = 16;
    static const size_t nrOf_blocks = 16;
    block blocks[nrOf_blocks];
    assert(nrOf_blocks > 1 && "Number of blocks cannot be less than 2!");
    static const size_t memory_size =
        sizeof(*(blocks[0].block)) * block_size * nrOf_blocks;
    mem_init(memory_size);

    printf_yellow(
        "  Testing resize from half memory size to full memory size\n");
    block_init(&blocks[0], nrOf_blocks / 2);
    my_assert(blocks[0].block != NULL && "Failed to allocate block");

    blocks[0] = block_resize(blocks[0], nrOf_blocks * block_size);
    my_assert(blocks[0].block != NULL && "Block resize to full memory failed");

    printf_green("  -Successfull\n");

    printf_yellow("  Testing resize to size 0\n");
    blocks[0] = block_resize(blocks[0], 0);
    my_assert(blocks[0].block == NULL &&
              "Block resize to 0 did not return NULL");
    printf_green("  -Successfull\n");

    printf_yellow("  Testing if resize to 0 properly freed memory\n");
    block_init(&blocks[0], nrOf_blocks * block_size);
    my_assert(
        blocks[0].block != NULL &&
        "block allocation failed, mem_resize to 0 might not have freed memory");
    block_free(&blocks[0]);
    printf_green("  -Successfull\n");

    printf_yellow("  Testing resize with invalid pointer\n");
    blocks[0] = block_resize(blocks[0], block_size);
    my_assert(blocks[0].block == NULL &&
              "Block resize with invalid pointer did not return NULL");
    printf_green("  -Successfull\n");

    printf_yellow("  Testing resize with NULL block\n");
    blocks[0] = block_resize((block){NULL, 0}, block_size);
    my_assert(blocks[0].block != NULL &&
              "Block resize with NULL block did not allocate new block");
    block_free(&blocks[0]);
    printf_green("  -Successfull\n");

    printf_yellow(
        "  Testing resize with an empty block inbetween a bunch of other "
        "blocks\n");

    for (size_t i = 0; i < nrOf_blocks; i++) block_init(&blocks[i], block_size);

    size_t resized_block_index = nrOf_blocks / 2 - 1;
    size_t removed_block_index = nrOf_blocks / 2;
    block_free(&blocks[removed_block_index]);

    block fail_block =
        block_resize(blocks[resized_block_index], block_size * 2 + 1);
    my_assert(fail_block.block == NULL &&
              "Block resize to invalid size succeded when it shouldn't");

    blocks[resized_block_index] =
        block_resize(blocks[resized_block_index], block_size * 2);
    my_assert(blocks[resized_block_index].block != NULL &&
              "Block resize to valid size failed");

    printf_yellow("  Testing if resize to new place copies data correctly\n");
    block_free(&blocks[resized_block_index]);

    for (size_t i = 0; i < blocks[0].size; i++) blocks[0].block[i] = i;

    block resized_block_0 = block_resize(blocks[0], block_size * 2);

    for (size_t i = 0; i < blocks[0].size; i++)
        my_assert(resized_block_0.block[i] == blocks[0].block[i] &&
                  "resize does not copy data correctly");
    test_memory_validity_(blocks, nrOf_blocks);

    printf_green("  -Successfull\n");

    block_free_all(blocks, nrOf_blocks);

    mem_deinit();
    printf_green("mem_resize passed.\n");
}

void test_allocation_exceeding_memory_size() {
    printf_yellow("Testing allocations exceeding memory pool size\n");
    static const size_t nrOf_blocks = 8;
    assert(nrOf_blocks > 2);

    static const size_t block_size = 16;
    int *blocks[nrOf_blocks];
    static const size_t memory_size =
        sizeof(**blocks) * (nrOf_blocks - 1) * block_size;

    mem_init(memory_size);  // Initialize with memory_size of memory
    blocks[0] = mem_alloc(memory_size + 1);
    my_assert(blocks[0] == NULL &&
              "Successfully allocated more memory than memory pool size, this "
              "is bad");

    blocks[0] = mem_alloc(memory_size);
    my_assert(blocks[0] != NULL &&
              "failed to allocate exact memory in memory pool");

    blocks[1] = mem_alloc(1);  // This should fail, no space left
    my_assert(blocks[1] == NULL &&
              "Allocated one more than total memory pool, not good");

    mem_free(blocks[0]);
    for (size_t i = 0; i < nrOf_blocks; i++)
        blocks[i] = mem_alloc(sizeof(**blocks) * block_size);

    my_assert(blocks[nrOf_blocks - 1] == NULL &&
              "Succeded with allocation beyond memory capacity, bad");

    mem_free_all(blocks, nrOf_blocks);

    mem_deinit();
    printf_green("Allocations exceeding pool size test passed.\n");
}

void test_double_free() {
    printf_yellow("Testing double deallocation...\n");
    mem_init(1024);  // Initialize with 1KB of memory

    void *block1 = mem_alloc(512);  // Allocate a block of 100 bytes
    my_assert(block1 != NULL);      // Ensure the block was allocated

    void *block2 = mem_alloc(512);  // Allocate a block of 100 bytes
    my_assert(block2 != NULL);      // Ensure the block was allocated

    mem_free(block1);  // Free the block for the first time
    mem_free(block1);  // Attempt to free the block a second time

    my_assert(mem_alloc(1024) == NULL &&
              "Double mem_free of the same block freed another block");
    block1 = mem_alloc(512);
    my_assert(block1 != NULL &&
              "Failed to alocate memory again after double free");

    mem_free(block1);
    mem_free(block2);
    mem_free(block2);

    printf_green(
        "Double deallocation test passed (if no crash and handled "
        "gracefully).\n");
    mem_deinit();  // Cleanup memory
}

void test_memory_fragmentation() {
    printf_yellow("Testing memory fragmentation handling...\n");

    static const size_t nrOf_blocks = 4;
    static const size_t block_size = 8;
    int *blocks[nrOf_blocks];

    static const size_t memory_size =
        sizeof(**blocks) * block_size * nrOf_blocks;

    mem_init(memory_size);  // Initialize with 2048 bytes

    for (size_t i = 0; i < nrOf_blocks; i++)
        blocks[i] = mem_alloc(sizeof(**blocks) * block_size);

    mem_free(
        blocks[2]);  // Free third block, leaving a fragmented hole after block2
    blocks[2] = mem_alloc(sizeof(**blocks) *
                          block_size);  // Should fit into the space of block3
    assert(blocks[2] != NULL);

    mem_free(blocks[0]);
    mem_free(blocks[2]);
    mem_free(blocks[3]);

    // should fit after second block
    blocks[3] = mem_alloc(sizeof(**blocks) * block_size * 2);
    // should fit before second block
    blocks[0] = mem_alloc(sizeof(**blocks) * block_size);

    mem_free_all(blocks, nrOf_blocks);

    mem_deinit();
    printf_green("Memory fragmentation test passed.\n");
}
int main() {
    test_init();
    test_alloc_and_free();
    test_resize();
    test_allocation_exceeding_memory_size();
    test_double_free();
    test_memory_fragmentation();
    printf_green("All tests passed successfully!\n");
    return 0;
}