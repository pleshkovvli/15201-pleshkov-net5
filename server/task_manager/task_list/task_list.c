#include <memory.h>
#include <stdlib.h>
#include "task_list.h"
#include "../../../protocol.h"
#include "../../../agreements.h"

void init_task_list(task_list_t *task_list) {
    task_list->tasks = malloc(sizeof(task_t *) * MAX_TASKS);
    task_list->amount = 0;
}

void destroy_task_list(task_list_t *task_list) {
    for (ushort i = 0; i < task_list->amount; ++i) {
        destroy_task(task_list->tasks[i]);
    }

    free(task_list->tasks);
}

void add_task(task_list_t *list, task_t *task) {
    list->tasks[list->amount] = task;
    ++list->amount;
}

void move_task(task_list_t *to, task_list_t *from, int num) {
    add_task(to, from->tasks[num]);
    --(from->amount);
    from->tasks[num] = from->tasks[from->amount];
}

int remove_task(task_list_t *task_list, uuid_t uuid) {
    for (ushort uuid_num = 0; uuid_num < task_list->amount; ++uuid_num) {
        if (memcmp(task_list->tasks[uuid_num]->uuid, uuid, UUID_LEN) == 0) {
            remove_task_by_num(task_list, uuid_num);
            return TRUE;
        }
    }

    return FALSE;
}

void remove_task_by_num(task_list_t *task_list, ushort num) {
    if(num >= task_list->amount) {
        return;
    }

    task_t **task_array = task_list->tasks;
    destroy_task(task_array[num]);

    ushort cur_amount = --task_list->amount;
    if (num < cur_amount) {
        task_array[num] = task_array[cur_amount];
    }
}

ushort get_task_num(task_list_t *task_list, const void *client_uuid) {
    for (ushort task_num = 0; task_num < task_list->amount; ++task_num) {
        if (memcmp(task_list->tasks[task_num]->uuid, client_uuid, UUID_LEN) == 0) {
            return task_num;
        }
    }

    return task_list->amount;
}
