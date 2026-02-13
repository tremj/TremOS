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

void skip_queue(struct ready_queue *queue, struct pcb *pcb) {
    pcb->next = queue->head;
    queue->head = pcb;
    queue->size++;
}

void insert_between(struct ready_queue *queue, struct pcb *pcb_insert, struct pcb *pcb_prev, struct pcb *pcb_next) {
    if (pcb_prev == NULL) { // head
        skip_queue(queue, pcb_insert);
    } else if (pcb_next == NULL) { // tail
        enqueue_process(queue, pcb_insert);
    } else {
        pcb_insert->next = pcb_next;
        pcb_prev->next = pcb_insert;
        queue->size++;
    }
}

void free_queue(struct ready_queue *queue) {
    while (queue->size > 0) {
        free(dequeue_process(queue));
    }
    free(queue);
}