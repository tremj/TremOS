#include <stdio.h>

struct pcb {
    int start, length, pid, pc;
    struct pcb *next;
};

struct pcb *init_pcb(FILE *p);
void cleanup_code(struct pcb *pcb);
int compare_pcbs(const void *a, const void *b);