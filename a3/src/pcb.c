#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpreter.h"
#include "pagetable.h"
#include "pcb.h"
#include "shellmemory.h"


int next_pid = 1;

// function reads a FILE * struct line by line and adds each line to the
// shell memory program line array
struct pcb *init_pcb(FILE *p, int bg_mode) {
    struct pcb *pcb = (struct pcb *) malloc(sizeof(struct pcb));
    if (pcb == NULL) {
        return NULL;
    }
    pcb->pc = 0;
    pcb->length = 0;
    pcb->next = NULL;
    pcb->page_table = (struct page_table *) malloc(sizeof(struct page_table));
    int pt_malloc_size = 2;
    if (bg_mode == BG_ENABLED) {
        pcb->page_table->table = (int *) malloc(pt_malloc_size * sizeof(int));
        pcb->page_table->size = 0;
    }
    pcb->page_table->references = 1;
    size_t line_buf = 0; // will get resized by getline call

    while (1) {
        char *lines[3] = {NULL, NULL, NULL};
        for (int i = 0; i < 3; i++) {
            char *line = NULL;
            ssize_t line_len = getline(&line, &line_buf, p);
            if (line_len == -1) {
                break;
            }
            if (line_len > 0 && line[line_len - 1] == '\n') {
                line[line_len - 1] = '\0';
            }
            pcb->length++;
            lines[i] = strdup(line);
            free(line);
        }

        if (lines[0] == NULL) { // no lines added
            break;
        }

        if (bg_mode == BG_ENABLED) {
            int frame = mem_set_frame(lines);
            pcb->page_table->table[pcb->page_table->size++] = frame;
            if (pcb->page_table->size == pt_malloc_size) {
                pt_malloc_size *= 2;
                void *tmp = (int *) realloc(pcb->page_table->table, pt_malloc_size * sizeof(int));
                if (tmp == NULL) {
                    return NULL;
                } else {
                    pcb->page_table->table = tmp;
                }
            }
        }

        // semi-full frame, file is done
        if (lines[1] == NULL || lines[2] == NULL) {
            break;
        }
    }

    pcb->pid = next_pid++;
    pcb->length_score = pcb->length;
    int total_frames = (pcb->length + 3 - 1) / 3; // ceiling of length / 3
    if (bg_mode == BG_ENABLED) {
        void *tmp = (int *) realloc(pcb->page_table->table, pcb->page_table->size * sizeof(int));
        if (tmp == NULL) {
            return NULL;
        } else {
            pcb->page_table->table = tmp;
        }
    } else {
        pcb->page_table->table = (int *) malloc(sizeof(int) * total_frames);
    }
    pcb->page_table->size = total_frames;
    return pcb;
}

int load_code(struct pcb **pcbs, char *files[], char fileStatus, int size, int sourceCmd) {
    // loading order
    int order[3] = {-1, -1, -1};
    if (sourceCmd == SOURCE_CMD_FLAG) {
        order[0] = 0;
    } else {
        int i = 0;
        if (size == 3 && (fileStatus & (0b010 | 0b001)) == 0) { // f3 is unique
            order[i] = 2;
            i++;
        }
        if (size >= 2 && (fileStatus & (0b100 | 0b001)) == 0) { // f2 is unique
            order[i] = 1;
            i++;
        }
        order[i] = 0; // always include f1
    }

    // get all file objects
    FILE *f_objs[3] = {NULL, NULL, NULL};
    int i = 0;
    while (i < 3 && files[i] != NULL) {
        int skip_file = 0;
        if (i == 1 && (fileStatus & (0b100 | 0b001)) != 0) {
            skip_file = 1;
        } else if (i == 2 && (fileStatus & (0b010 | 0b001)) != 0) {
            skip_file = 1;
        }

        if (skip_file == 0) {
            f_objs[i] = fopen(files[i], "rt");
            if (f_objs[i] == NULL) {
                return -1;
            }
        }
        i++;
    }

    // start loading code
    int total = i;
    i = 0;
    int page_table_index[3] = {0, 0, 0};
    int done = 0;
    while (done != total) {
        int program = order[i];
        if (program == -1) {
            i = (i + 1) % 3;
            continue;
        }

        char *lines[3] = {NULL, NULL, NULL};
        size_t line_buf = 0;
        for (int j = 0; j < 3; j++) {
            char *line = NULL;
            ssize_t line_len = getline(&line, &line_buf, f_objs[program]);
            if (line_len == -1) {
                done++;
                order[i] = -1;
                break;
            }
            if (line_len > 0 && line[line_len - 1] == '\n') {
                line[line_len - 1] = '\0';
            }
            lines[j] = strdup(line);
            free(line);
        }

        if (lines[0] == NULL) { // never read a line from file
            i = (i + 1) % 3;
            continue;
        }

        int frame = mem_set_frame(lines);
        pcbs[program]->page_table->table[page_table_index[program]] = frame;
        page_table_index[program]++;
        i = (i + 1) % 3;
    }

    i = 0;
    while (i < 3 && f_objs[i] != NULL) {
        fclose(f_objs[i]);
        i++;
    }

    return 0;
}

char *fetch_next_instruction(struct pcb *pcb) {
    int frame = pcb->page_table->table[pcb->pc / 3];
    int offset = pcb->pc % 3;
    pcb->pc++;
    return mem_get_program_line(frame*3 + offset);
}

// cleaning up PCB code lines stored in program line memory
void cleanup_code(struct pcb *pcb) {
    for (int i = 0; i < pcb->page_table->size; i++) {
        mem_free_frame(pcb->page_table->table[i]);
    }
}

// acts like a free(pcb) call but frees all lines in the program
// memory array in memory
void cleanup_pcb(struct pcb *pcb) {
    if (pcb->page_table->references == 1) {
        cleanup_code(pcb);
        free_page_table(pcb->page_table);
    } else {
        pcb->page_table->references--;
    }
    free(pcb);
}

// comparator helper function for initial sorting
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

// returns a copy of a pcb
struct pcb *copy_pcb(struct pcb *pcb) {
    struct pcb *new_pcb = (struct pcb *)malloc(sizeof(struct pcb));
    if (new_pcb == NULL) {
        return NULL;
    }

    new_pcb->start = pcb->start;
    new_pcb->length = pcb->length;
    new_pcb->length_score = pcb->length_score;
    new_pcb->pid = next_pid++;
    new_pcb->pc = pcb->pc;
    new_pcb->page_table = pcb->page_table;
    new_pcb->page_table->references++;

    return new_pcb;
}