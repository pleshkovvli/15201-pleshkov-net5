#ifndef NET5_TASK_LIST_H
#define NET5_TASK_LIST_H

#include "task.h"
#include <uuid/uuid.h>

#define MAX_TASKS 2048

typedef struct task_list {
    task_t **tasks;
    ushort amount;
} task_list_t;

void init_task_list(task_list_t *task_list);
void destroy_task_list(task_list_t *task_list);

ushort get_task_num(task_list_t *task_list, const void *client_uuid);

void add_task(task_list_t *list, task_t *task);
void move_task(task_list_t *to, task_list_t *from, int num);
int remove_task(task_list_t *task_list, uuid_t uuid);
void remove_task_by_num(task_list_t *task_list, ushort num);

#endif //NET5_TASK_LIST_H
