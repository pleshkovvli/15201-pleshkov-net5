#ifndef NET5_TASK_H
#define NET5_TASK_H

#include <time.h>
#include <uuid/uuid.h>

typedef struct task {
    uuid_t uuid;
    time_t timestamp;
    char *begin_str;
    char *end_str;
} task_t;

task_t *make_task(const char *begin, const char *end, uuid_t uuid);
void destroy_task(task_t *task);

#endif //NET5_TASK_H
