#include <stdlib.h>
#include "task_manager.h"
#include "../../agreements.h"

void init_task_manager(task_manager_t *task_manager, const char *hash) {
    task_manager->hash = hash;
    task_manager->valid = TRUE;

    task_manager->tasks_to_do = malloc(sizeof(task_list_t));
    init_task_list(task_manager->tasks_to_do);

    task_manager->tasks_going = malloc(sizeof(task_list_t));
    init_task_list(task_manager->tasks_going);

    task_manager->str_gen = malloc(sizeof(str_gen_t));
    init_str_gen(task_manager->str_gen, "ACGT", 0);
}

void destroy_task_manager(task_manager_t *task_maker) {
    destroy_task_list(task_maker->tasks_to_do);
    free(task_maker->tasks_to_do);

    destroy_task_list(task_maker->tasks_going);
    free(task_maker->tasks_going);

    destroy_str_gen(task_maker->str_gen);
    free(task_maker->str_gen);
}

void check_tasks(const task_manager_t *task_manager, ushort sec_to_do) {
    time_t cur_time = time(NULL);
    task_list_t *going = task_manager->tasks_going;
    task_list_t *to_do = task_manager->tasks_to_do;

    for (int task_num = 0; task_num < going->amount; ++task_num) {
        time_t cur_timestamp = going->tasks[task_num]->timestamp;
        if ((cur_time - cur_timestamp) > sec_to_do) {
            move_task(to_do, going, task_num);
            --task_num;
        }
    }
}
