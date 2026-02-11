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

int program_memory_counter = 0;

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

void free_program_line(int index) {
    free(program_memory[index].line);
    program_memory[index].line = NULL;
}
