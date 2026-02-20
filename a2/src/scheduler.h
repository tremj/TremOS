#ifndef SCHEDULER_H
#define SCHEDULER_H

struct ready_queue;
struct pcb;

struct scheduler {
    int scheduler_type;
    int mt_mode;
    struct ready_queue *queue;
};

void init_scheduler_lock();
void destroy_scheduler_lock();
void lock_scheduler();
void unlock_scheduler();
struct scheduler *prepare_scheduler(struct pcb **pcbs, int count, char *scheduler_type, int mt_mode);
void queue_pcbs(struct scheduler *s, struct pcb **pcbs, int count);
void prioritize_pcb(struct scheduler *s, struct pcb *pcb);
void run_scheduler(struct scheduler *s);
void free_scheduler(struct scheduler *s);

#endif


