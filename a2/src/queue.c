#include <stdio.h>
#include <stdlib.h>
#include "queue.h"


struct ready_queue *queue = NULL;

void init_queue() {
    queue = (struct ready_queue *) malloc(sizeof(struct ready_queue));
    queue->size = 0;
    queue->head = NULL;
    queue->tail = NULL;
}

void enqueue_process(struct pcb *pcb) {
    if (queue->size == 0) {
        queue->head = pcb;
        queue->tail = pcb;
    } else {
        queue->tail->next = pcb;
        queue->tail = pcb;
    }

    queue->size++;
}

struct pcb *dequeue_process() {
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