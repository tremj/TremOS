#include <stdio.h>

struct pcb {
    int start, length, length_score, pid, pc;
    struct pcb *next;
    struct page_table *page_table;
};

#define SOURCE_CMD_FLAG 1

struct pcb *init_pcb(FILE *p, int bg_mode);
int load_code(struct pcb **pcbs, char *files[], char fileStatus, int size, int sourceCmd);
char *fetch_next_instruction(struct pcb *pcb);
void cleanup_code(struct pcb *pcb);
void cleanup_pcb(struct pcb *pcb);
int compare_pcbs(const void *a, const void *b);
struct pcb *copy_pcb(struct pcb *pcb);