#ifndef PCB_H
#define PCB_H

struct pcb;

struct ready_queue {
    int size;
    struct pcb *head;
    struct pcb *tail;
};

struct ready_queue *init_queue();
void enqueue_process(struct ready_queue *queue, struct pcb *pcb);
struct pcb *dequeue_process(struct ready_queue *queue);
void skip_queue(struct ready_queue *queue, struct pcb *pcb);
void insert_between(struct ready_queue *queue, struct pcb *pcb_insert, struct pcb *pcb_prev, struct pcb *pcb_next);
void free_queue(struct ready_queue *queue);

#endif
