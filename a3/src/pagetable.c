#include <stdlib.h>
#include "pagetable.h"

void free_page_table(struct page_table *pt) {
    free(pt->table);
    free(pt);
}