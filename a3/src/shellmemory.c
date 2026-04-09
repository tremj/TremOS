#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lru.h"
#include "pagetable.h"
#include "pcb.h"
#include "shellmemory.h"

struct memory_struct {
    char *var;
    char *value;
};

struct memory_struct shellmemory[VARMEMSIZE];

struct frame frameStore[FRAMESIZE / 3];

struct lru *lru;

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
    for (int i = 0; i < VARMEMSIZE; i++) {
        shellmemory[i].var   = "none";
        shellmemory[i].value = "none";
    }
    for (int i = 0; i < FRAMESIZE / 3; i++) {
        for (int j = 0; j < 3; j++) {
            frameStore[i].lines[j] = NULL;
        }
        frameStore[i].pcb = NULL;
        frameStore[i].index = i;
    }
    lru = init_lru();
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

void mem_set_program_line(int frame, int offset, char *line) {
    frameStore[frame].lines[offset] = strdup(line);
}

char *mem_get_program_line(struct pcb *pcb, int frame, int offset) {
    if (frame < 0 || frame >= FRAMESIZE / 3 || offset < 0 || offset >= 3) {
        return NULL;
    }
    struct frame *frameRef = &frameStore[frame];
    // pcb does not own this frame anymore
    if (frameRef->pcb != pcb) {
        pcb->page_fault = PAGE_FAULT;
        return NULL;
    }
    // enqueue frame into LRU when
    // it has no "neighbours" and it is not the only element in the LRU
    // the prev or next field wouldn't be NULL if the frame wasn't the only element
    // ----
    // in any other case, the frame has already been added to the LRU and
    // must be prioritized to the front to avoid being evicted
    if (frameRef->prev == NULL && frameRef->next == NULL && lru->head != frameRef) {
        enqueue_frame(lru, frameRef);
    } else {
        prioritize_frame(lru, frameRef);
    }
    return frameStore[frame].lines[offset];
}

void free_program_line(int frame, int offset) {
    free(frameStore[frame].lines[offset]);
    frameStore[frame].lines[offset] = NULL;
}

int mem_set_frame(struct pcb *pcb, char **lines) {
    // incrementally allocate frames
    int frameNumber = frame_memory_counter;
    frameStore[frameNumber].pcb = pcb;
    // insert non NULL lines into frame
    for (int i = 0; i < 3 && lines[i] != NULL; i++) {
        mem_set_program_line(frameNumber, i, lines[i]);
    }
    frame_memory_counter++;
    return frameNumber;
}

void mem_update_frame(int frame, char **lines) {
    // free all lines in frame and replace them
    for (int i = 0; i < 3; i++) {
        if (frameStore[frame].lines[i] == NULL) {
            free_program_line(frame, i); // sets line to NULL after
        }
        if (lines[i] != NULL) {
            mem_set_program_line(frame, i, lines[i]);
        }
    }
}

// handles the page fault
void handle_page_fault(struct pcb *pcb) {
    printf("Page fault!");
    int pageTableIndex = pcb->pc / 3;
    int frameIndex = -1;
    // are all frames allocated?
    // if yes, we need to evict a frame from the LRU
    if (frame_memory_counter >= (FRAMESIZE / 3)) {
        struct frame *frame = evict_frame(lru);
        printf(" Victim page contents:\n\n");
        for (int i = 0; i < 3; i++) {
            printf("%s\n", frame->lines[i]);
        }
        printf("\nEnd of victim page contents.\n");
        // frame is now invalid in the old PCB
        // frame is now owned by new PCB
        frame->pcb = pcb;
        frameIndex = frame->index;
        pcb->page_table->table[pageTableIndex] = frameIndex;
    } else {
        printf("\n");
    }

    FILE *p = fopen(pcb->filename, "rt");
    long offset = pcb->page_table->page_offset[pageTableIndex];
    // use the page offset to get next page lines
    fseek(p, offset, SEEK_SET);
    char *lines[3] = {NULL, NULL, NULL};
    size_t line_buf = 0;
    for (int i = 0; i < 3; i++) {
        char *line = NULL;
        ssize_t line_len = getline(&line, &line_buf, p);
        if (line_len == -1) {
            break;
        }
        if (line_len > 0 && line[line_len - 1] == '\n') {
            line[line_len - 1] = '\0';
        }
        lines[i] = strdup(line);
        free(line);
    }
    // set an empty frame or update an evicted frame?
    if (frameIndex == -1) {
        int frame = mem_set_frame(pcb, lines);
        pcb->page_table->table[pageTableIndex] = frame;
    } else {
        mem_update_frame(frameIndex, lines);
    }
    // remove the page fault "flag" to continue execution
    pcb->page_fault = NO_PAGE_FAULT;
}
