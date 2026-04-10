// Test: recursive (self-referential) struct — e.g., a linked-list node.
//
// This tests that c2pulse can handle struct types with self-referential
// pointer fields, needed for pointer-based linked lists, trees, etc.

typedef struct node {
    int data;
    struct node *next;
} node;

void set_data(node *n, int x) {
    n->data = x;
}
