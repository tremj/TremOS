struct page_table {
    int *table;
    long *page_offset;
    int size;
};

void free_page_table(struct page_table *pt);