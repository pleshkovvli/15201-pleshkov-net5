#include "task_maker.h"
#include "protocol.h"
#include "get_task.h"
#include <stdlib.h>
#include <string.h>

#define BEGIN_SUF "AAAAAA"
#define END_SUF "TTTTTT"
#define ALPHABET "ACGT"

void init_task_maker(task_maker_t *task_maker, const char *hash) {
    task_maker->hash = hash;
    task_maker->tasks_list = malloc(sizeof(task_list_t));

    task_maker->tasks_list->tasks = malloc(sizeof(task_t) * MAX_CLIENTS);
    task_maker->tasks_list->amount = 0;

    init_str_gen(task_maker->str_gen);
}

void destroy_task_maker(task_maker_t *task_maker) {
    free(task_maker->tasks_list->tasks);
    free(task_maker->tasks_list);

    destroy_str_gen(task_maker->str_gen);
}

int init_str_gen(str_gen_t *str_gen) {
    str_gen->reverse_table = malloc(sizeof(ushort) * 256);
    str_gen->cur_string = malloc(sizeof(char) * 32);

    if (str_gen->reverse_table == NULL || str_gen->cur_string == NULL) {
        return FAILURE_CODE;
    }

    str_gen->begin_suf = BEGIN_SUF;
    str_gen->end_suf = END_SUF;
    str_gen->alphabet = ALPHABET;

    str_gen->alphabet_length = (ushort) strlen(str_gen->alphabet);

    for (ushort i = 0; i < str_gen->alphabet_length; ++i) {
        str_gen->reverse_table[(u_char) str_gen->alphabet[i]] = i;
    }

    return SUCCESS_CODE;
}

void destroy_str_gen(str_gen_t *str_gen) {
    free(str_gen->reverse_table);
    free(str_gen->cur_string);
}

void remove_task(task_list_t *tasks, uuid_t uuid) {
    ushort uuid_num = 0;

    for (; uuid_num < tasks->amount; ++uuid_num) {
        if (memcmp(tasks->tasks[uuid_num].uuid, uuid, UUID_LEN) == 0) {
            break;
        }
    }

    if(uuid_num == tasks->amount) {
        return;
    }

    --tasks->amount;

    if(uuid_num < tasks->amount) {
        copy_task(&tasks->tasks[uuid_num], &tasks->tasks[tasks->amount]);
    }

}