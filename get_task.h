#ifndef NET5_GET_TASK_H
#define NET5_GET_TASK_H

#include "types.h"
#include "task_maker.h"

task_t *get_task(task_maker_t *tasks_maker, void *client_uuid);

int get_task_num(const void *client_uuid, int amount, const task_t *tasks);

int get_todo_num(const task_t *tasks, int amount);

void copy_task(task_t *dest, const task_t *src);

void renew_task(task_t *task, const void *client_uuid);

void get_next_str(str_gen_t *str_gen);

void memcpy_next(void *buffer, const void *src, size_t n, size_t *written);

void fill_task(task_t *task, str_gen_t *str_gen);

enum next_action {
    ADD, NOT_ADD
};

enum next_action next_string(str_gen_t *str_gen, ushort length, int index);

enum next_action change_sym(str_gen_t *str_gen, int index);

#endif //NET5_GET_TASK_H
