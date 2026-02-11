#include <stdio.h>
#include <stdlib.h>
#include "pcb.h"
#include "shellmemory.h"


int next_pid = 1;


struct pcb *init_pcb(FILE *p) {
    struct pcb *pcb = (struct pcb *) malloc(sizeof(struct pcb));
    if (pcb == NULL) {
        return NULL;
    }
    pcb->start = program_memory_counter;
    pcb->pc = pcb->start;
    pcb->length = 0;
    pcb->next = NULL;
    size_t line_buf = 0; // will get resized by getline call

    while (1) {
        char *line = NULL;
        ssize_t line_len = getline(&line, &line_buf, p);
        if (line_len == -1) {
            break;
        }
        if (line_len > 0 && line[line_len - 1] == '\n') {
            line[line_len - 1] = '\0';
        }

        mem_set_program_line(line);
        pcb->length++;
        free(line);
    }

    pcb->pid = next_pid++;
    pcb->length_score = pcb->length;
    return pcb;
}

void cleanup_code(struct pcb *pcb) {
    for (int i = pcb->start; i < pcb->start + pcb->length; i++) {
        free_program_line(i);
    }
}

int compare_pcbs(const void *a, const void *b) {
    const struct pcb *pcbA = *(const struct pcb **)a;
    const struct pcb *pcbB = *(const struct pcb **)b;

    if (pcbA->length < pcbB->length) {
        return -1;
    } else if (pcbA->length == pcbB->length) {
        return 0;
    } else {
        return 1;
    }
}