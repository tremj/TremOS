#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcb.h"
#include "queue.h"
#include "scheduler.h"
#include "shell.h"
#include "shellmemory.h"

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
        s->scheduler_type = 0;
    } else {
        s->scheduler_type = 0;
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

    free(s->queue);
    free(s);
}

void run_next_instruction(struct scheduler *s) {
    struct pcb *p = s->queue->head;
    if (p->start + p->length == p->pc) { // end of program
        cleanup_code(p); // cleanup
        dequeue_process(s->queue);
    } else {
        char *line = mem_get_program_line(p->pc++);
        parseInput(line);
    }
}

void FCFS_scheduling(struct scheduler *s) {
    while (s->queue->size > 0) {
        run_next_instruction(s);
    }
    program_memory_counter = 0;
}

void SJF_scheduling(struct scheduler *s, int enable_aging) {
    // TODO
    return;
}

void RR_scheduling(struct scheduler *s, int instr_per_turn) {
    // TODO
    return;
}