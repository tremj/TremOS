#include "shellmemory.h"

struct ready_queue {
    int size;
    struct pcb *head;
    struct pcb *tail;
};

#ifndef READY_QUEUE
#define READY_QUEUE

extern struct ready_queue *queue;

#endif

void init_queue();
void enqueue_process(struct pcb *pcb);
struct pcb *dequeue_process();