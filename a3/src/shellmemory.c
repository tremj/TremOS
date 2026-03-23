#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"

struct memory_struct {
    char *var;
    char *value;
};

struct memory_struct shellmemory[VARMEMSIZE];

struct program_line frameStore[3*FRAMESIZE];

int frame_memory_counter = 0;

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

void mem_init() {
    int i;
    int max = 3*FRAMESIZE;
    for (i = 0; i < VARMEMSIZE; i++){
        shellmemory[i].var   = "none";
        shellmemory[i].value = "none";
        if (i < max) frameStore[i].line = NULL;
    }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
    int i;

    for (i = 0; i < VARMEMSIZE; i++){
        if (strcmp(shellmemory[i].var, var_in) == 0){
            shellmemory[i].value = strdup(value_in);
            return;
        } 
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < VARMEMSIZE; i++){
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

    for (i = 0; i < VARMEMSIZE; i++){
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

void mem_set_program_line(int index, char *line) {
    frameStore[index].line = strdup(line);
}

char *mem_get_program_line(int index) {
    if (index < 0 || index >= 3*FRAMESIZE) {
        return NULL;
    }
    return frameStore[index].line;
}

void free_program_line(int index) {
    free(frameStore[index].line);
    frameStore[index].line = NULL;
}

int mem_set_frame(char **lines) {
    int frameNumber = frame_memory_counter;
//    printf("Alloc frame %d:\n", frameNumber);
//    for (int i = 0; i < 3; i++) {
//        printf("  [%d] = %s\n", i, lines[i]);
//    }
    int startIndex = frameNumber * 3;
    for (int i = 0; i < 3 && lines[i] != NULL; i++) {
        mem_set_program_line(startIndex + i, lines[i]);
    }
    frame_memory_counter++;
    return frameNumber;
}

void mem_free_frame(int frame) {
    int startIndex = frame * 3;
    for (int i = 0; i < 3; i++) {
        int index = startIndex + i;
        if (mem_get_program_line(index) != NULL) {
            free_program_line(index);
        }
    }
}
