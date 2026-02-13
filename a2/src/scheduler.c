#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcb.h"
#include "queue.h"
#include "scheduler.h"
#include "shell.h"
#include "shellmemory.h"

#define FCFS 0
#define SJF 1
#define RR 2
#define AGING 3
#define RR30 4

#define NO_AGING 0
#define ENABLE_AGING 1

#define RR_DEFAULT 2
#define RR30_INSTR 30

void FCFS_scheduling(struct scheduler *s);
void SJF_scheduling(struct scheduler *s, int enable_aging);
void RR_scheduling(struct scheduler *s, int instr_per_turn);


struct scheduler *init_scheduler(char *scheduler_type) {
    struct scheduler *s = (struct scheduler *) malloc(sizeof(struct scheduler));
    if (s == NULL) {
        return NULL;
    }
    s->queue = init_queue();
    if (s->queue == NULL) {
        return NULL;
    }

    if (strcmp(scheduler_type, "FCFS") == 0) {
        s->scheduler_type = FCFS;
    } else if (strcmp(scheduler_type, "SJF") == 0) {
        s->scheduler_type = SJF;
    } else if (strcmp(scheduler_type, "RR") == 0) {
        s->scheduler_type = RR;
    } else if (strcmp(scheduler_type, "AGING") == 0) {
        s->scheduler_type = AGING;
    } else if (strcmp(scheduler_type, "RR30") == 0) {
        s->scheduler_type = RR30;
    } else {
        return NULL;
    }

    return s;
}

void SJF_AGING_ordering(struct pcb **pcbs, int count, size_t data_size, int (*comparator)(const void *, const void *)) {
    qsort(pcbs, count, data_size, comparator);
}

struct scheduler *prepare_scheduler(struct pcb **pcbs, int count, char *scheduler_type) {
    struct scheduler *s = init_scheduler(scheduler_type);
    if (s == NULL) {
        return NULL;
    }

    if (s->scheduler_type == SJF || s->scheduler_type == AGING) {
        SJF_AGING_ordering(pcbs, count, sizeof(struct pcb *), compare_pcbs);
    }

    for (int i = 0; i < count; i++) {
        enqueue_process(s->queue, pcbs[i]);
    }

    return s;
}

void SJF_AGING_insert(struct scheduler *s, struct pcb *pcb) {
    struct pcb *prev = NULL;
    struct pcb *curr = s->queue->head;
    while (curr != NULL && pcb->length_score > curr->length_score) {
        prev = curr;
        curr = curr->next;
    }

    insert_between(s->queue, pcb, prev, curr);
}

void queue_pcbs(struct scheduler *s, struct pcb **pcbs, int count) {
    for (int i = 0; i < count; i++) {
        if (s->scheduler_type == SJF || s->scheduler_type == AGING) {
            SJF_AGING_insert(s, pcbs[i]);
        } else {
            enqueue_process(s->queue, pcbs[i]);
        }
    }
}

void prioritize_pcb(struct scheduler *s, struct pcb *pcb) {
    skip_queue(s->queue, pcb);
}

void run_scheduler(struct scheduler *s) {
    int enable_aging = NO_AGING;
    int instr_per_turn = RR_DEFAULT;
    switch (s->scheduler_type) {
        case FCFS:
            FCFS_scheduling(s);
            break;
        case AGING:
            enable_aging = ENABLE_AGING;
        case SJF:
            SJF_scheduling(s, enable_aging);
            break;
        case RR30:
            instr_per_turn = RR30_INSTR;
        case RR:
            RR_scheduling(s, instr_per_turn);
        default:
            break;
    }

    program_memory_counter = 0;
}

#define LINE_EXECUTED 0
#define PROCESS_REMOVED 1

int run_next_instruction(struct scheduler *s) {
    struct pcb *pcb = dequeue_process(s->queue);
    char *line = mem_get_program_line(pcb->pc++);
    parseInput(line);
    if (pcb->start + pcb->length == pcb->pc) { // end of program
        cleanup_code(pcb);
        free(pcb);
        return PROCESS_REMOVED;
    }
    skip_queue(s->queue, pcb);
    return LINE_EXECUTED;
}

void FCFS_scheduling(struct scheduler *s) {
    while (s->queue->size > 0) {
        run_next_instruction(s);
    }
}

void age_jobs(struct scheduler *s) {
    // already more than 1 job in queue
    // it was checked before calling the function
    struct pcb *tmp = s->queue->head->next;
    for (int i = 1; i < s->queue->size; i++) {
        if (tmp->length_score > 0) {
            tmp->length_score--;
        }
        tmp = tmp->next;
    }

    struct pcb *pcb = dequeue_process(s->queue);

    SJF_AGING_insert(s, pcb);
}

void SJF_scheduling(struct scheduler *s, int enable_aging) {
    while (s->queue->size > 0) {
        int status = run_next_instruction(s);
        if (status != PROCESS_REMOVED && enable_aging == ENABLE_AGING && s->queue->size > 1) {
            age_jobs(s);
        }
    }
}

void rotate_jobs(struct scheduler *s) {
    struct pcb *p = dequeue_process(s->queue);
    enqueue_process(s->queue, p);
}

void RR_scheduling(struct scheduler *s, int instr_per_turn) {
    int status;
    while (s->queue->size > 0) {
        for (int i = 0; i < instr_per_turn; i++) {
            status = run_next_instruction(s);
            if (status == PROCESS_REMOVED) {
                break;
            }
        }

        if (status != PROCESS_REMOVED && s->queue->size > 1) {
            rotate_jobs(s);
        }
    }
}

void free_scheduler(struct scheduler *s) {
    free_queue(s->queue);
    free(s);
}