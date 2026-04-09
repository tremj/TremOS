struct frame;
struct lru;

struct lru {
    struct frame *head;
    struct frame *tail;
    int size;
};

struct lru *init_lru();
void enqueue_frame(struct lru *lru, struct frame *frame);
struct frame *evict_frame(struct lru *lru);
void prioritize_frame(struct lru *lru, struct frame *frame);