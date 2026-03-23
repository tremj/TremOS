struct page_table {
    int *table;
    int size;
    int references;
};

void free_page_table(struct page_table *pt);