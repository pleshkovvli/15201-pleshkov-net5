#include "task_manager.h"
#include "../protocol.h"
#include "../server/handlers.h"
#include "task_list.h"
#include "../server/server.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void init_task_manager(task_manager_t *task_maker, const char *hash) {
    task_maker->hash = hash;

    task_maker->tasks_to_do = malloc(sizeof(task_list_t));
    init_task_list(task_maker->tasks_to_do);

    task_maker->tasks_going = malloc(sizeof(task_list_t));
    init_task_list(task_maker->tasks_going);

    task_maker->str_gen = malloc(sizeof(str_gen_t));
    init_str_gen(task_maker->str_gen, NULL);
}

void destroy_task_manager(task_manager_t *task_maker) {
    destroy_task_list(task_maker->tasks_to_do);
    free(task_maker->tasks_to_do);

    destroy_task_list(task_maker->tasks_going);
    free(task_maker->tasks_going);

    destroy_str_gen(task_maker->str_gen);
    free(task_maker->str_gen);
}


task_t *get_task(task_manager_t *tasks_manager, void *client_uuid) {
    task_list_t *going = tasks_manager->tasks_going;

    ushort task_num = get_task_num(going, client_uuid);
    if(task_num < going->amount) {
        remove_task_by_num(going, task_num);
    }

    task_list_t *to_do = tasks_manager->tasks_to_do;
    task_t *task;

    if(to_do->amount > 0) {
        task = to_do->tasks[to_do->amount - 1];
        --to_do->amount;
    } else {
        str_gen_t *str_gen = tasks_manager->str_gen;
        task = make_task(str_gen->begin_str, str_gen->end_str, client_uuid);
        next_str(str_gen);
    }

    add_task(going, task);

    return task;
}

void check_tasks(const task_manager_t *task_manager) {
    time_t cur_time = time(NULL);
    task_list_t *going = task_manager->tasks_going;
    task_list_t *to_do = task_manager->tasks_to_do;

    for (int i = 0; i < going->amount; ++i) {
        time_t cur_timestamp = going->tasks[i]->timestamp;
        if ((cur_time - cur_timestamp) > TIME_TO_DO) {
            move_task(to_do, going, i);
            --i;
        }
    }
}
