#ifndef SCHEDULER_H
#define SCHEDULER_H

struct ready_queue;
struct pcb;

struct scheduler {
    int scheduler_type;
    struct ready_queue *queue;
};

struct scheduler *prepare_scheduler(struct pcb **pcbs, int count, char *scheduler_type);
void queue_pcbs(struct scheduler *s, struct pcb **pcbs, int count);
void prioritize_pcb(struct scheduler *s, struct pcb *pcb);
void run_scheduler(struct scheduler *s);
void free_scheduler(struct scheduler *s);

#endif


