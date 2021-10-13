#include "node.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

//typedef struct Node Node;

/*struct Node {
    Node *left;
    Node *right;
    uint8_t symbol;
    uint64_t frequency;
};*/

Node *node_create(uint8_t symbol, uint64_t frequency) {
    Node *n = (Node *) malloc(sizeof(Node));
    if (n) {
        n->symbol = symbol;
        n->frequency = frequency;
        n->left = NULL;
        n->right = NULL;
    }
    return n;
}

void node_delete(Node **n) {
    if (*n) {
        free(*n);
        *n = NULL;
    }
    return;
}

Node *node_join(Node *left, Node *right) {
    Node *n = node_create('$', (left->frequency + right->frequency));
    n->left = left;
    n->right = right;
    return n;
}

void node_print(Node *n) {
    if (n == NULL) {
        return;
    }
    node_print(n->left);
    node_print(n->right);
    if (n->left == NULL && n->right == NULL) {
        printf("L%u", n->symbol);
    } else {
        printf("I");
    }
    return;
}
