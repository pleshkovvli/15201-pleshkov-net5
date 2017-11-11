#include "get_task.h"
#include "protocol.h"
#include <string.h>

task_t *get_task(task_maker_t *tasks_maker, void *client_uuid) {
    int amount = tasks_maker->tasks_list->amount;
    task_t *tasks = tasks_maker->tasks_list->tasks;

    int task_num = get_task_num(client_uuid, amount, tasks);

    task_t *task = &tasks[task_num];
    renew_task(task, client_uuid);

    int to_do_num = get_todo_num(tasks, amount);

    if (task_num == amount) {
        ++(tasks_maker->tasks_list->amount);
    }

    if (to_do_num == amount) {
        get_next_str(tasks_maker->str_gen);
        fill_task(task, tasks_maker->str_gen);
    } else {
        task_t *past_task = &tasks[to_do_num];

        strncpy(task->begin_str, past_task->begin_str, 32);
        strncpy(task->end_str, past_task->end_str, 32);

        task_t *last_task = &tasks[amount - 1];

        copy_task(past_task, last_task);

        --(tasks_maker->tasks_list->amount);
    }

    return task;
}

int get_task_num(const void *client_uuid, int amount, const task_t *tasks) {
    for (int task_num = 0; task_num < amount; ++task_num) {
        if (memcmp(tasks[task_num].uuid, client_uuid, UUID_LEN) == 0) {
            return task_num;
        }
    }

    return amount;
}

void renew_task(task_t *task, const void *client_uuid) {
    task->status = IN_PROGRESS;
    task->timestamp = time(NULL);
    memcpy(task->uuid, client_uuid, UUID_LEN);
}

int get_todo_num(const task_t *tasks, int amount) {
    for (int to_do_num = 0; to_do_num < amount; ++to_do_num) {
        if (tasks[to_do_num].status == TO_DO) {
            return to_do_num;
        }
    }

    return amount;
}

void fill_task(task_t *task, str_gen_t *str_gen) {
    char *cur_string = str_gen->cur_string;
    size_t length = strlen(cur_string);

    size_t written = 0;
    memcpy_next(task->begin_str, cur_string, length, &written);
    memcpy_next(task->begin_str, str_gen->begin_suf, SUF_LEN, &written);

    written = 0;
    memcpy_next(task->end_str, cur_string, length, &written);
    memcpy_next(task->end_str, str_gen->end_suf, SUF_LEN, &written);
}

void copy_task(task_t *dest, const task_t *src) {
    dest->status = src->status;
    dest->timestamp = src->timestamp;
    memcpy(dest->uuid, src->uuid, UUID_LEN);
    strncpy(dest->begin_str, src->begin_str, 32);
    strncpy(dest->end_str, src->end_str, 32);
}

void memcpy_next(void *buffer, const void *src, size_t n, size_t *written) {
    memcpy(&buffer[*written], src, n);
    *written += n;
}

void get_next_str(str_gen_t *str_gen) {
    char *cur_string = str_gen->cur_string;

    ushort length = (ushort) strlen(cur_string);
    enum next_action add = next_string(str_gen, length, 0);

    if (add == ADD) {
        cur_string[length] = str_gen->alphabet[0];
        cur_string[length + 1] = '\0';
    }
}

enum next_action next_string(str_gen_t *str_gen, ushort length, int index) {
    if (index == length - 1) {
        return change_sym(str_gen, index);

    }

    int result = next_string(str_gen, length, index + 1);

    if (result == ADD) {
        return change_sym(str_gen, index);
    }

    return NOT_ADD;

}

enum next_action change_sym(str_gen_t *str_gen, int index) {
    char *string = str_gen->cur_string;
    const char *alphabet = str_gen->alphabet;
    const int alphabet_length = str_gen->alphabet_length;
    const ushort *reverse_table = str_gen->reverse_table;

    char cur_sym = string[index];
    ushort cur_position = reverse_table[cur_sym];

    int next_position = (cur_position + 1) % alphabet_length;

    string[index] = alphabet[next_position];
    return (next_position == 0) ? ADD : NOT_ADD;
}
