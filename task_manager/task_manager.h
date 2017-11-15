#ifndef NET5_TASK_MAKER_H
#define NET5_TASK_MAKER_H

#include "../types.h"
#include "str_gen.h"
#include "task_list.h"

#define MAX_CLIENTS 64

typedef struct task_manager {
    task_list_t *tasks_to_do;
    task_list_t *tasks_going;
    str_gen_t *str_gen;
    const char *hash;
} task_manager_t;

void init_task_manager(task_manager_t *task_maker, const char *hash);
void destroy_task_manager(task_manager_t *task_maker);

task_t *get_task(task_manager_t *tasks_manager, void *client_uuid);

void check_tasks(const task_manager_t *task_manager);

#endif //NET5_TASK_MAKER_H
