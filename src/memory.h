
#ifndef MEMORY_H
#define MEMORY_H

typedef struct app_memory_t {
    memory_arena_t *perm_arena;
    memory_arena_t *temp_arena;
} app_memory_t;

#endif //MEMORY_H
