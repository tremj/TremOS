#include <stdlib.h>
#include "lru.h"
#include "pcb.h"
#include "shellmemory.h"


// simple implementation of a doubly linked list specifically for a frame struct
// function bodies modified from implementations in queue.c

// create an empty queue
struct lru *init_lru() {
    struct lru *lru = (struct lru *)malloc(sizeof(struct lru));
    if (lru == NULL) {
        return NULL;
    }

    lru->size = 0;
    lru->head = NULL;
    lru->tail = NULL;

    return lru;
}

// typical enqueuing function for doubly linked list
void enqueue_frame(struct lru *lru, struct frame *frame) {
    frame->prev = NULL;
    frame->next = NULL;
    if (lru->size == 0) {
        lru->head = frame;
        lru->tail = frame;
    } else {
        frame->prev = lru->tail;
        lru->tail->next = frame;
        lru->tail = frame;
    }

    lru->size++;
}

// typical dequeue function for doubly linked list
struct frame *evict_frame(struct lru *lru) {
    if (lru->size == 0) {
        return NULL;
    }

    struct frame *tmp = lru->head;
    lru->head = tmp->next;
    if (lru->head != NULL) {
        lru->head->prev = NULL;
    }

    lru->size--;

    if (lru->size == 0) {
        lru->tail = NULL;
    }

    tmp->prev = NULL;
    tmp->next = NULL;
    return tmp;
}

// remove frame from middle of LRU and enqueue it
void prioritize_frame(struct lru *lru, struct frame *frame) {
    if (lru->size > 0) {
        if (frame == NULL || lru->head == frame) {
            return;
        }

        if (frame->prev != NULL) {
            frame->prev->next = frame->next;
        }

        if (frame->next != NULL) {
            frame->next->prev = frame->prev;
        }

        if (lru->tail == frame) {
            lru->tail = frame->prev;
        }
        lru->size--;
    }
    enqueue_frame(lru, frame);
}