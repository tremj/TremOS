#define FCFS 0
#define SJF 1
#define RR 2
#define AGING 3
#define RR30 4

#define NO_AGING 0
#define ENABLE_AGING 1

#define RR_DEFAULT 2
#define RR30_INSTR 30

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


