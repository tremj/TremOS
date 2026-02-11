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

struct scheduler *prepare_scheduler(struct pcb **pcbs, int count, char *scheduler_type) {
    struct scheduler *s = init_scheduler(scheduler_type);
    if (s == NULL) {
        return NULL;
    }

    if (s->scheduler_type == SJF || s->scheduler_type == AGING) {
        qsort(pcbs, count, sizeof(struct pcb *), compare_pcbs);
    }

    for (int i = 0; i < count; i++) {
        enqueue_process(s->queue, pcbs[i]);
    }

    return s;
}


void run_scheduler(struct pcb **pcbs, int count, char *scheduler_type) {
    struct scheduler *s = prepare_scheduler(pcbs, count, scheduler_type);
    if (s == NULL) {
        printf("Something went wrong!!\n");
        return;
    }

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

    free(s->queue);
    free(s);
}

#define LINE_EXECUTED 0
#define PROCESS_REMOVED 1

int run_next_instruction(struct scheduler *s) {
    struct pcb *p = s->queue->head;
    if (p->start + p->length == p->pc) { // end of program
        cleanup_code(p);
        dequeue_process(s->queue);
        return PROCESS_REMOVED;
    } else {
        char *line = mem_get_program_line(p->pc++);
        parseInput(line);
        return LINE_EXECUTED;
    }
}

void FCFS_scheduling(struct scheduler *s) {
    while (s->queue->size > 0) {
        run_next_instruction(s);
    }
}

void age_jobs(struct scheduler *s) {
    struct pcb *curr_exec_pcb = s->queue->head;
    struct pcb *tmp = s->queue->head->next;
    for (int i = 1; i < s->queue->size; i++) {
        if (tmp->length_score > 0) {
            tmp->length_score--;
        }
        tmp = tmp->next;
    }

    struct pcb *next_pcb = curr_exec_pcb->next;
    if (next_pcb != NULL && next_pcb->length_score < curr_exec_pcb->length_score) {
        dequeue_process(s->queue);
        enqueue_process(s->queue, curr_exec_pcb);
    }
}

void SJF_scheduling(struct scheduler *s, int enable_aging) {
    while (s->queue->size > 0) {
        run_next_instruction(s);
        if (enable_aging == ENABLE_AGING && s->queue->size > 1) {
            age_jobs(s);
        }
    }
}

void rotate_jobs(struct scheduler *s) {
    struct pcb *p = dequeue_process(s->queue);
    enqueue_process(s->queue, p);
}

void RR_scheduling(struct scheduler *s, int instr_per_turn) {
    while (s->queue->size > 0) {
        for (int i = 0; i < instr_per_turn; i++) {
            int status = run_next_instruction(s);
            if (status == PROCESS_REMOVED) {
                break;
            }
        }

        if (s->queue->size > 1) {
            rotate_jobs(s);
        }
    }
}