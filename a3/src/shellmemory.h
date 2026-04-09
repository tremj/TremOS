#ifndef PRG_MEM_CNT_H
#define PRG_MEM_CNT_H

extern int frame_memory_counter;

#endif

#define INVALID_FRAME 0
#define VALID_FRAME 1

struct frame {
    char *lines[3];
    struct pcb *pcb;
    struct frame *prev;
    struct frame *next;
    int index;
};

struct program_line {
    char *line;
};

struct memory_return {
    int status;
    char *res;
};
void mem_init();
struct memory_return *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
void mem_set_program_line(int frame, int index, char *line);
char *mem_get_program_line(struct pcb *pcb, int frame, int offset);
void free_program_line(int frame, int offset);
int mem_set_frame(struct pcb *pcb, char **lines);
void handle_page_fault(struct pcb *pcb);
