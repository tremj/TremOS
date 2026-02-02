#define MEM_SIZE 1000
struct memory_return {
    int status;
    char *res;
};
void mem_init();
struct memory_return *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
