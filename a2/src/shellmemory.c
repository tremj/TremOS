#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"

struct memory_struct {
    char *var;
    char *value;
};

struct memory_struct shellmemory[MEM_SIZE];

struct program_line program_memory[MEM_SIZE];

int program_memory_counter;

// Helper functions
int match(char *model, char *var) {
    int i, len = strlen(var), matchCount = 0;
    for (i = 0; i < len; i++) {
        if (model[i] == var[i]) matchCount++;
    }
    if (matchCount == len) {
        return 1;
    } else return 0;
}

// Shell memory functions

void mem_init(){
    int i;
    for (i = 0; i < MEM_SIZE; i++){		
        shellmemory[i].var   = "none";
        shellmemory[i].value = "none";
        program_memory[i].line = NULL;
    }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++){
        if (strcmp(shellmemory[i].var, var_in) == 0){
            shellmemory[i].value = strdup(value_in);
            return;
        } 
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < MEM_SIZE; i++){
        if (strcmp(shellmemory[i].var, "none") == 0){
            shellmemory[i].var   = strdup(var_in);
            shellmemory[i].value = strdup(value_in);
            return;
        } 
    }

    return;
}

//get value based on input key
struct memory_return *mem_get_value(char *var_in) {
    int i;
    struct memory_return *result = (struct memory_return *)malloc(sizeof(struct memory_return));

    for (i = 0; i < MEM_SIZE; i++){
        if (strcmp(shellmemory[i].var, var_in) == 0){
	    result->res = strdup(shellmemory[i].value);
	    result->status = 0;
	    return result;
        } 
    }
    result->res = strdup("Variable does not exist");
    result->status = -1;
    return result;
}

void mem_set_program_line(char *line) {
    for (int i = program_memory_counter; i < MEM_SIZE; i++) {
        if (program_memory[i].line == NULL) {
            program_memory[i].line = strdup(line);
            program_memory_counter = i + 1;
            return;
        }
    }
}

char *mem_get_program_line(int index) {
    if (index < 0 || index >= MEM_SIZE) {
        return NULL;
    }
    return program_memory[index].line;
}

// load program into memory
struct pcb *load_program(FILE *p) {
    struct pcb *pcb = (struct pcb *) malloc(sizeof(struct pcb));
    pcb->start = program_memory_counter;
    pcb->pc = 0;
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

    return pcb;
}
