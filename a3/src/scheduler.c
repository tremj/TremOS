#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "interpreter.h"
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


pthread_mutex_t scheduler_lock;


// collection of 4 functions that allows lock to be
// locked from anywhere that includes the scheduler.h
// header file
void init_scheduler_lock() {
    pthread_mutex_init(&scheduler_lock, NULL);
}

void destroy_scheduler_lock() {
    pthread_mutex_destroy(&scheduler_lock);
}

void lock_scheduler() {
    pthread_mutex_lock(&scheduler_lock);
}

void unlock_scheduler() {
    pthread_mutex_unlock(&scheduler_lock);
}


// create an empty scheduler struct
struct scheduler *init_scheduler(char *scheduler_type, int mt_mode) {
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

    s->mt_mode = mt_mode;

    return s;
}

// shows clearly what the qsort call is actually doing
// function signature taken from stdlib.h
void SJF_AGING_ordering(struct pcb **pcbs, int count, size_t data_size, int (*comparator)(const void *, const void *)) {
    qsort(pcbs, count, data_size, comparator);
}

// returns non-empty scheduler struct with initial exec PCBs in queue
struct scheduler *prepare_scheduler(struct pcb **pcbs, int count, char *scheduler_type, int mt_mode) {
    struct scheduler *s = init_scheduler(scheduler_type, mt_mode);
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


// insert old head of the ready_queue into the queue
// based on the length_score field of the PCB
void SJF_AGING_insert(struct scheduler *s, struct pcb *pcb) {
    struct pcb *prev = NULL;
    struct pcb *curr = s->queue->head;
    while (curr != NULL && pcb->length_score > curr->length_score) {
        prev = curr;
        curr = curr->next;
    }

    insert_between(s->queue, pcb, prev, curr);
}

// queues PCBs in the ready_queue depending on the scheduler type
void queue_pcbs(struct scheduler *s, struct pcb **pcbs, int count) {
    for (int i = 0; i < count; i++) {
        if (s->scheduler_type == SJF || s->scheduler_type == AGING) {
            SJF_AGING_insert(s, pcbs[i]);
        } else {
            enqueue_process(s->queue, pcbs[i]);
        }
    }
}

// add PCB to the front of the ready queue
// useful for batch script prioritizing for background mode
void prioritize_pcb(struct scheduler *s, struct pcb *pcb) {
    skip_queue(s->queue, pcb);
}

// select what scheduling function to run based on scheduler_type field
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

    // reset the memory counter, all PCBs complete & won't be reused
    frame_memory_counter = 0;
}

#define LINE_EXECUTED 0
#define PROCESS_REMOVED 1
#define NO_OP 2

// executes line & tells scheduling algorithm whether
// to clean up the PCB or continue the execution according
// to the scheduling mode
int run_next_instruction(struct pcb *pcb, int mt_mode) {
    char *line = fetch_next_instruction(pcb);
    if (pcb->page_fault == PAGE_FAULT) {
        handle_page_fault(pcb);
        return NO_OP; // figure out what to return
    }
    parseInput(line);
    if (pcb->pc == pcb->length) { // end of program
        return PROCESS_REMOVED;
    }
    return LINE_EXECUTED;
}

// FCFS scheduling algorithm
// keep running job until run_next_instruction returns PROCESS_REMOVED
// then clean up the PCB
void FCFS_scheduling(struct scheduler *s) {
    while (s->queue->size > 0) {
        struct pcb *pcb = dequeue_process(s->queue);

        while (run_next_instruction(pcb, MT_DISABLED) != PROCESS_REMOVED);

        cleanup_pcb(pcb);
    }
}

// ages all jobs in the queue and insert currently
// executed process back into the queue
void age_jobs(struct scheduler *s, struct pcb *pcb) {
    // already more than 1 job in queue
    // it was checked before calling the function
    struct pcb *tmp = s->queue->head;
    for (int i = 0; i < s->queue->size; i++) {
        if (tmp->length_score > 0) {
            tmp->length_score--;
        }
        tmp = tmp->next;
    }

    SJF_AGING_insert(s, pcb);
}

// similar to FCFS in the NO_AGING branch
// if aging is enabled then on each while loop iteration
// all jobs must be aged and reordered
void SJF_scheduling(struct scheduler *s, int enable_aging) {
    while (s->queue->size > 0) {
        struct pcb *pcb = dequeue_process(s->queue);
        if (enable_aging == NO_AGING) {
            while (run_next_instruction(pcb, MT_DISABLED) != PROCESS_REMOVED);
            cleanup_pcb(pcb);
        } else {
            int status = run_next_instruction(pcb, MT_DISABLED);
            if (status == PROCESS_REMOVED) {
                cleanup_pcb(pcb);
            } else {
                age_jobs(s, pcb);
            }
        }
    }
}

// calls run_next_instruction to the amount of instr_per_turn
void RR_run_instructions(struct scheduler *s, int instr_per_turn, struct pcb *pcb, int mt_mode) {
    int status = LINE_EXECUTED;
    for (int i = 0; i < instr_per_turn; i++) {
        status = run_next_instruction(pcb, mt_mode);
        if (status == PROCESS_REMOVED) {
            cleanup_pcb(pcb);
            break;
        } else if (status == NO_OP) {
            break;
        }
    }

    if (mt_mode == MT_ENABLED) {
        lock_scheduler();
    }
    if (status == LINE_EXECUTED || status == NO_OP) {
        enqueue_process(s->queue, pcb);
    }
    if (mt_mode == MT_ENABLED) {
        unlock_scheduler();
    }
}

pthread_t thread1, thread2;

// RR & RR30 thread function that keep on taking from the ready_queue while it is not empty
// ready_queue size can't be 0 in both threads while there are still PCBs to execute
void *RR_thread(void *scheduler) {
    int instr_per_turn = RR_DEFAULT;
    // no need to lock scheduler, field will not be changed
    if (((struct scheduler *)scheduler)->scheduler_type == RR30) {
        instr_per_turn = RR30_INSTR;
    }

    while (1) {
        pthread_mutex_lock(&scheduler_lock);
        if (((struct scheduler *)scheduler)->queue->size == 0) {
            pthread_mutex_unlock(&scheduler_lock);
            break;
        }
        struct pcb *pcb = dequeue_process(((struct scheduler *)scheduler)->queue);
        pthread_mutex_unlock(&scheduler_lock);

        RR_run_instructions(scheduler, instr_per_turn, pcb, MT_ENABLED);
    }

    return NULL;
}

// RR scheduling algorithm, creates thread pool and joins threads when multithreaded is enabled
// if MT is enabled then it skips the entire while loop and waits the join threads
void RR_scheduling(struct scheduler *s, int instr_per_turn) {
    if (s->mt_mode == MT_ENABLED) {
        lock_scheduler();
        pthread_create(&thread1, NULL, RR_thread, s);
        pthread_create(&thread2, NULL, RR_thread, s);
    }

    while (s->mt_mode == MT_DISABLED && s->queue->size > 0) {
        struct pcb *pcb = dequeue_process(s->queue);
        RR_run_instructions(s, instr_per_turn, pcb, s->mt_mode);
    }

    if (s->mt_mode == MT_ENABLED) {
        unlock_scheduler();
        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);
    }
}

// helper function to free all malloc fields of the scheduler struct
void free_scheduler(struct scheduler *s) {
    free_queue(s->queue);
    free(s);
}