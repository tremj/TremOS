#include <stdio.h>
#include <stdlib.h>
#include "pcb.h"
#include "queue.h"
#include "shellmemory.h"


struct ready_queue *init_queue() {
    struct ready_queue *queue = (struct ready_queue *)malloc(sizeof(struct ready_queue));
    if (queue == NULL) {
        return NULL;
    }

    queue->size = 0;
    queue->head = NULL;
    queue->tail = NULL;

    return queue;
}

void enqueue_process(struct ready_queue *queue, struct pcb *pcb) {
    if (queue->size == 0) {
        queue->head = pcb;
        queue->tail = pcb;
    } else {
        queue->tail->next = pcb;
        queue->tail = pcb;
    }

    queue->size++;
}

struct pcb *dequeue_process(struct ready_queue *queue) {
    if (queue->size == 0) {
        return NULL;
    } else {
        struct pcb *tmp = queue->head;
        queue->head = queue->head->next;
        queue->size--;
        if (queue->size == 0) {
            queue->tail = NULL;
        }
        tmp->next = NULL;
        return tmp;
    }
}