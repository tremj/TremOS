#define MEM_SIZE 1000
struct pcb {
    pid_t pid;
    int start, length, pc;
    struct pcb *next;
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
struct pcb *load_program(FILE *p);
