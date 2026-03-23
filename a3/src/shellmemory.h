#ifndef PRG_MEM_CNT_H
#define PRG_MEM_CNT_H

extern int frame_memory_counter;

#endif


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
void mem_set_program_line(int index, char *line);
char *mem_get_program_line(int index);
void free_program_line(int index);
int mem_set_frame(char **lines);
void mem_free_frame(int frame);
