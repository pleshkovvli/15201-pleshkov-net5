#include "task.h"
#include "../../../agreements.h"

#include <stdlib.h>
#include <string.h>

task_t *make_task(const char *begin, const char *end, uuid_t uuid) {
    task_t *task = malloc(sizeof(task_t));
    if(task == NULL) {
        return task;
    }

    size_t begin_len = strlen(begin);
    size_t end_len = strlen(end);

    task->begin_str = malloc(begin_len + 1);
    if(task->begin_str == NULL) {
        free(task);
        return NULL;
    }

    task->end_str = malloc(end_len + 1);
    if(task->end_str == NULL) {
        free(task->begin_str);
        free(task);
        return NULL;
    }

    strncpy(task->begin_str, begin, begin_len + 1);
    strncpy(task->end_str, end, end_len + 1);

    memcpy(task->uuid, uuid, UUID_SIZE);

    return task;
}

void destroy_task(task_t *task) {
    free(task->begin_str);
    free(task->end_str);
    free(task);
}
