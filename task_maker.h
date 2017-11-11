#ifndef NET5_TASK_MAKER_H
#define NET5_TASK_MAKER_H

#include "types.h"

#define MAX_CLIENTS 64

typedef struct task_list {
    task_t *tasks;
    ushort amount;
} task_list_t;

typedef struct str_gen {
    char *cur_string;
    const char *begin_suf;
    const char *end_suf;
    const char *alphabet;
    ushort alphabet_length;
    ushort *reverse_table;
} str_gen_t;

typedef struct task_maker {
    task_list_t *tasks_list;
    str_gen_t *str_gen;
    const char *hash;
} task_maker_t;

void init_task_maker(task_maker_t *task_maker, const char *hash);
void destroy_task_maker(task_maker_t *task_maker);
int init_str_gen(str_gen_t *str_gen);
void destroy_str_gen(str_gen_t *str_gen);


#endif //NET5_TASK_MAKER_H
