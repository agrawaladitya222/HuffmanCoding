#include "pq.h"

#include "node.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct PriorityQueue {
    uint32_t head;
    uint32_t tail;
    uint32_t size;
    uint32_t maxcap;
    Node **nodes;
} PriorityQueue;

uint32_t next(uint32_t pos, uint32_t capacity) {
    return ((pos + 1) % capacity);
}

uint32_t prev(uint32_t pos, uint32_t capacity) {
    return ((pos + capacity - 1) % capacity);
}

PriorityQueue *pq_create(uint32_t capacity) {
    PriorityQueue *pq = (PriorityQueue *) malloc(sizeof(PriorityQueue));
    if (pq) {
        pq->head = 0;
        pq->tail = 0;
        pq->size = 0;
        pq->maxcap = capacity;
        pq->nodes = (Node **) calloc(capacity, sizeof(Node));
        if (!pq->nodes) {
            free(pq);
            pq = NULL;
        }
    }
    return pq;
}

void pq_delete(PriorityQueue **q) {
    if (*q && (*q)->nodes) {
        free((*q)->nodes);
        free(*q);
        *q = NULL;
    }
    return;
}

bool pq_empty(PriorityQueue *q) {
    return (q->size == 0);
}

bool pq_full(PriorityQueue *q) {
    return ((q->size) == (q->maxcap));
}

uint32_t pq_size(PriorityQueue *q) {
    return q->size;
}

bool enqueue(PriorityQueue *q, Node *n) {
    if (pq_full(q)) {
        return false;
    }

    uint32_t slot = q->tail;
    // while (prev(slot) node freq > slot node freq) && (slot != head)
    while ((slot != q->head) && (q->nodes[prev(slot, q->maxcap)]->frequency > n->frequency)) {
        q->nodes[slot] = q->nodes[prev(slot, q->maxcap)]; // slot node = prev(slot) node
        slot -= 1;
    }
    q->nodes[slot] = n;
    q->size += 1;
    q->tail = next(q->tail, q->maxcap);

    return true;
}

bool dequeue(PriorityQueue *q, Node **n) {
    if (pq_empty(q)) {
        return false;
    }

    *n = q->nodes[q->head];
    q->head = next(q->head, q->maxcap);
    q->size -= 1;
    return true;
}

void pq_print(PriorityQueue *q) {
    uint32_t index = q->head;
    if (!pq_empty(q)) {
        printf("%" PRIu64 ", ", q->nodes[index]->frequency);
        index = next(index, q->maxcap);

        while (index != q->tail) {
            printf("%" PRIu64 ", ", q->nodes[index]->frequency);
            index = next(index, q->maxcap);
        }
    } else {
        printf("queue empty");
    }
    printf("\n");
    return;
}

/* main function for testing only
int main(void) {
    PriorityQueue *pq = pq_create(5);
    printf("pq made\n");
    Node *a = node_create('a', 8);
    Node *b = node_create('b', 6);
    Node *c = node_create('c', 4);
    Node *d = node_create('d', 2);
    Node *e = node_create('e', 1);
    printf("nodes made\n");
    node_print(a);
    node_print(b);
    node_print(c);
    node_print(d);
    node_print(e);

    enqueue(pq,a);
    enqueue(pq,b);
    enqueue(pq,c);
    enqueue(pq,d);
    enqueue(pq,e);
    printf("done enqueue'ing\n");
    if( pq_full(pq) ){
	printf("queue full\n");
    }
    pq_print(pq);

    Node *dummy = NULL;
    for(int y=0; y<5; y+=1){
	dequeue(pq, &dummy);
	node_print(dummy);
    }
    printf("done dequeue'ing\n");

    pq_print(pq);
    if( pq_empty(pq) ) {
	printf("queue empty\n");
    }

    node_delete(&a);
    node_delete(&b);
    node_delete(&c);
    node_delete(&d);
    node_delete(&e);
    //node_delete(&dummy);
    pq_delete(&pq);
    return 0;
}
*/
