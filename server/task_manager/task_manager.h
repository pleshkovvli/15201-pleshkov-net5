#ifndef NET5_TASK_MAKER_H
#define NET5_TASK_MAKER_H

#include "str_gen/i-str_gen.h"
#include "task_list/task_list.h"

typedef struct task_manager {
    task_list_t *tasks_to_do;
    task_list_t *tasks_going;
    str_gen_t *str_gen;
    const char *hash;
} task_manager_t;

void init_task_manager(task_manager_t *task_manager, const char *hash, ushort max_str_len, const char *abc);
void destroy_task_manager(task_manager_t *task_maker);

void check_tasks(const task_manager_t *task_manager, ushort sec_to_do);

#endif //NET5_TASK_MAKER_H
