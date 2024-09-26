#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <stdbool.h>
#include <stdint.h> 

#include "common_defs.h"
#include "memory_manager.h"
#include "linked_list.h"

// Initialization function
void list_init(Node** head, size_t size) {
    mem_init(size + size * sizeof(BlockHeader) / sizeof(Node));
    *head = NULL;
}

// Insertion function: Adds a new node with the specified data to the linked list
void list_insert(Node** head, uint16_t data) {
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "Failed to allocate memory for new node.\n");
        return;
    }
    new_node->data = data;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        Node* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
}

// Insertion function: Inserts a new node with the specified data immediately after a given node
void list_insert_after(Node* prev_node, uint16_t data) {
    if (prev_node == NULL) {
        fprintf(stderr, "The given previous node cannot be NULL.\n");
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "Failed to allocate memory for new node.\n");
        return;
    }
    new_node->data = data;
    new_node->next = prev_node->next;
    prev_node->next = new_node;
}

// Insertion function: Inserts a new node with the specified data immediately before a given node
void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    if (next_node == NULL) {
        fprintf(stderr, "The given next node cannot be NULL.\n");
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "Failed to allocate memory for new node.\n");
        return;
    }
    new_node->data = data;

    if (*head == next_node) {
        new_node->next = *head;
        *head = new_node;
        return;
    }

    Node* current = *head;
    while (current != NULL && current->next != next_node) {
        current = current->next;
    }

    if (current == NULL) {
        fprintf(stderr, "The given next node is not present in the list.\n");
        mem_free(new_node);
        return;
    }

    new_node->next = next_node;
    current->next = new_node;
}

// Deletion function: Removes a node with the specified data from the linked list
void list_delete(Node** head, uint16_t data) {
    if (*head == NULL) {
        fprintf(stderr, "The list is empty.\n");
        return;
    }

    Node* current = *head;
    Node* prev = NULL;

    while (current != NULL && current->data != data) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        fprintf(stderr, "Node with data %u not found.\n", data);
        return;
    }

    if (prev == NULL) {
        *head = current->next;
    } else {
        prev->next = current->next;
    }

    mem_free(current);
}

// Search function: Searches for a node with the specified data and returns a pointer to it
Node* list_search(Node** head, uint16_t data) {
    Node* current = *head;
    while (current != NULL) {
        if (current->data == data) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Display function: Prints all the elements in the linked list
void list_display(Node** head) {
    Node* current = *head;
    printf("[");
    while (current != NULL) {
        printf("%u", current->data);
        current = current->next;
        if (current != NULL) {
            printf(", ");
        }
    }
    printf("]\n");
}

// Display function: Prints all elements of the list between two nodes
void list_display_range(Node** head, Node* start_node, Node* end_node) {
    Node* current = *head;
    bool in_range = (start_node == NULL);

    printf("[");
    while (current != NULL) {
        if (current == start_node) {
            in_range = true;
        }
        if (in_range) {
            printf("%u", current->data);
            if (current == end_node) {
                break;
            }
            if (current->next != NULL) {
                printf(", ");
            }
        }
        current = current->next;
    }
    printf("]\n");
}

// Nodes count function: Returns the count of nodes
int list_count_nodes(Node** head) {
    int count = 0;
    Node* current = *head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

// Cleanup function: Frees all the nodes in the linked list
void list_cleanup(Node** head) {
    Node* current = *head;
    while (current != NULL) {
        Node* next = current->next;
        mem_free(current);
        current = next;
    }
    *head = NULL;
}
