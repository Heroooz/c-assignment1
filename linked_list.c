#include "linked_list.h"

pthread_mutex_t list_mutex;

// Initialization function
void list_init(Node** head, size_t size) {
    pthread_mutex_init(&list_mutex, NULL);
    pthread_mutex_lock(&list_mutex);
    mem_init(size);
    *head = NULL;
    pthread_mutex_unlock(&list_mutex);
}

// Insertion function: Adds a new node with the specified data to the linked list
void list_insert(Node** head, uint16_t data) {
    pthread_mutex_lock(&list_mutex);
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "Failed to allocate memory for new node.\n");
        pthread_mutex_unlock(&list_mutex);
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
    pthread_mutex_unlock(&list_mutex);
}

// Insertion function: Inserts a new node with the specified data immediately after a given node
void list_insert_after(Node* prev_node, uint16_t data) {
    pthread_mutex_lock(&list_mutex);
    if (prev_node == NULL) {
        fprintf(stderr, "The given previous node cannot be NULL.\n");
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "Failed to allocate memory for new node.\n");
        pthread_mutex_unlock(&list_mutex);
        return;
    }
    new_node->data = data;
    new_node->next = prev_node->next;
    prev_node->next = new_node;
    pthread_mutex_unlock(&list_mutex);
}

// Insertion function: Inserts a new node with the specified data immediately before a given node
void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    pthread_mutex_lock(&list_mutex);
    if (next_node == NULL) {
        fprintf(stderr, "The given next node cannot be NULL.\n");
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "Failed to allocate memory for new node.\n");
        pthread_mutex_unlock(&list_mutex);
        return;
    }
    new_node->data = data;

    if (*head == next_node) {
        new_node->next = *head;
        *head = new_node;
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    Node* current = *head;
    while (current != NULL && current->next != next_node) {
        current = current->next;
    }

    if (current == NULL) {
        fprintf(stderr, "The given next node is not present in the list.\n");
        mem_free(new_node);
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    new_node->next = next_node;
    current->next = new_node;
    pthread_mutex_unlock(&list_mutex);
}

// Deletion function: Removes a node with the specified data from the linked list
void list_delete(Node** head, uint16_t data) {
    pthread_mutex_lock(&list_mutex);
    if (*head == NULL) {
        fprintf(stderr, "The list is empty.\n");
        pthread_mutex_unlock(&list_mutex);
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
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    if (prev == NULL) {
        *head = current->next;
    } else {
        prev->next = current->next;
    }

    mem_free(current);
    pthread_mutex_unlock(&list_mutex);
}

// Search function: Searches for a node with the specified data and returns a pointer to it
Node* list_search(Node** head, uint16_t data) {
    pthread_mutex_lock(&list_mutex);
    Node* current = *head;
    while (current != NULL) {
        if (current->data == data) {
            pthread_mutex_unlock(&list_mutex);
            return current;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&list_mutex);
    return NULL;
}

// Display function: Prints all the elements in the linked list
void list_display(Node** head) {
    pthread_mutex_lock(&list_mutex);
    Node* current = *head;
    printf("[");
    while (current != NULL) {
        printf("%u", current->data);
        current = current->next;
        if (current != NULL) {
            printf(", ");
        }
    }
    printf("]");
    pthread_mutex_unlock(&list_mutex);
}

// Display function: Prints all elements of the list between two nodes
void list_display_range(Node** head, Node* start_node, Node* end_node) {
    pthread_mutex_lock(&list_mutex);
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
    printf("]");
    pthread_mutex_unlock(&list_mutex);
}

// Nodes count function: Returns the count of nodes
int list_count_nodes(Node** head) {
    pthread_mutex_lock(&list_mutex);
    int count = 0;
    Node* current = *head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    pthread_mutex_unlock(&list_mutex);
    return count;
}

// Cleanup function: Frees all the nodes in the linked list
void list_cleanup(Node** head) {
    pthread_mutex_lock(&list_mutex);
    Node* current = *head;
    while (current != NULL) {
        Node* next = current->next;
        mem_free(current);
        current = next;
    }
    *head = NULL;
    pthread_mutex_unlock(&list_mutex);
}
