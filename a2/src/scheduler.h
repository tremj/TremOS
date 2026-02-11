#ifndef SCHEDULER_H
#define SCHEDULER_H

struct ready_queue;
struct pcb;

struct scheduler {
    int scheduler_type;
    struct ready_queue *queue;
};

void run_scheduler(struct pcb **pcbs, int count, char *scheduler_type);

#endif


