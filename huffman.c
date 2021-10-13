#include "huffman.h"

#include "code.h"
#include "defines.h"
#include "node.h"
#include "pq.h"
#include "stack.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void post_trav_del(Node **root) {
    if ((*root)->left == NULL && (*root)->right == NULL) {
        node_delete(root);
    } else {
        post_trav_del(&((*root)->left));
        post_trav_del(&((*root)->right));
        node_delete(root);
    }
    return;
}

Node *build_tree(uint64_t hist[static ALPHABET]) {
    PriorityQueue *pq = pq_create(ALPHABET);
    for (uint16_t i = 0; i < ALPHABET; i += 1) { //enqueue individual symbols
        if (hist[i] != 0) {
            enqueue(pq, node_create(i, hist[i]));
        }
    }

    while (pq_size(pq) > 1) { // build tree leaving root node in pq
        Node *left;
        Node *right;
        dequeue(pq, &left);
        dequeue(pq, &right);
        enqueue(pq, node_join(left, right));
    }

    Node *root;
    dequeue(pq, &root);
    pq_delete(&pq);
    return root;
}

void build_codes(Node *root, Code table[static ALPHABET]) {
    static Code c = { 0, { 0 } };
    if (root == NULL) {
        return;
    }
    if (root->left == NULL && root->right == NULL) {
        table[root->symbol] = c;
        //fprintf(stderr, "table[%c] = ", root->symbol);
        //code_print(&c);
    } else {
        code_push_bit(&c, 0);
        build_codes(root->left, table);
        uint8_t dummy; // dummy variable to hold popped bit
        code_pop_bit(&c, &dummy);
        code_push_bit(&c, 1);
        build_codes(root->right, table);
        code_pop_bit(&c, &dummy);
    }
    return;
}

Node *rebuild_tree(uint16_t nbytes, uint8_t tree[static nbytes]) {
    Stack *s = stack_create(nbytes);
    for (int i = 0; i < nbytes; i += 1) {
        if (tree[i] == 'L') {
            i += 1;
            Node *n = node_create(tree[i], 0);
            stack_push(s, n);
        } else if (tree[i] == 'I') {
            Node *right;
            Node *left;
            stack_pop(s, &right);
            stack_pop(s, &left);
            Node *joined = node_join(left, right);
            stack_push(s, joined);
        }
    }
    Node *root;
    if (stack_size(s) != 1) {
        fprintf(stderr, "error with rebuild tree, should be last item on stack\n");
    }
    stack_pop(s, &root);
    stack_delete(&s);
    return root;
}
void delete_tree(Node **root) {
    if (*root) {
        post_trav_del(root);
    }
    return;
}
