#include "handlers.h"

#include <stdint.h>
#include <openssl/md5.h>
#include <arpa/inet.h>
#include <memory.h>

static char new_msg[] = NEW_MSG;
static char more_msg[] = MORE_MSG;
static char ack_msg[] = ACK_MSG;
static char match_msg[] = MATCH_MSG;

client_state get_status(void *buffer) {
    if (memcmp(buffer, new_msg, MSG_LEN) == 0) {
        return NEW;
    }

    if (memcmp(buffer, more_msg, MSG_LEN) == 0) {
        return MORE;
    }

    if (memcmp(buffer, match_msg, MSG_LEN) == 0) {
        return SUCCESS;
    }

    return UNKNOWN;
}


int try_handle_to_ack(const client_t *cur_client) {
    if (cur_client->bytes_read >= MSG_LEN) {
        return handle_to_ack(cur_client->buffer);
    }
}

int try_handle_success(
        const task_list_t *tasks_list,
        int sock_num,
        client_t *cur_client,
        success_t *success
) {
    if (success->happened || cur_client->bytes_read < CONF_LEN) {
        return SUCCESS_CODE;
    }

    int result = handle_success(tasks_list, cur_client);
    if (result == SUCCESS_CODE) {
        success->happened = TRUE;
        success->num = sock_num;
    }

    return result;
}

int try_handle_more(task_maker_t *tasks_srt_gen, int socket_fd, client_t *cur_client) {
    if (cur_client->bytes_read < CONF_LEN) {
        return SUCCESS_CODE;
    }

    return handle_more(tasks_srt_gen, socket_fd, cur_client);

}

int try_handle_new(task_maker_t *tasks_srt, int socket_fd, client_t *cur_client) {
    if (cur_client->bytes_read < CONF_LEN) {
        return SUCCESS_CODE;
    }

    return handle_new(tasks_srt, socket_fd, cur_client);
}

int try_handle_unknown(client_t *cur_client) {
    if (cur_client->bytes_read < MSG_LEN) {
        return SUCCESS_CODE;
    }

    client_state state = get_status(cur_client->buffer);

    if (state == UNKNOWN) {
        return FAILURE_CODE;
    }

    cur_client->state = state;
    return SUCCESS_CODE;
}


int handle_to_ack(void *buffer) {
    if (memcmp(buffer, ack_msg, MSG_LEN) == 0) {
        return SUCCESS_CODE;
    }
    return FAILURE_CODE;
}

int handle_success(const task_list_t *tasks, client_t *cur_client) {
    if (check_uuid(tasks, cur_client) == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int handle_more(task_maker_t *tasks_str, int socket_fd, client_t *cur_client) {
    int task_num = check_uuid(tasks_str->tasks_list, cur_client);
    if (task_num == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    task_t *task = get_task(tasks_str, &cur_client->buffer[MSG_LEN]);
    size_t msg_size = fill_buffer(cur_client->buffer, task);

    return send_work(msg_size, socket_fd, cur_client);

}

int handle_new(task_maker_t *task_maker, int socket_fd, client_t *cur_client) {
    task_t *task = get_task(task_maker, &cur_client->buffer[MSG_LEN]);
    size_t msg_size = fill_buf_with_hash(cur_client->buffer, task, task_maker->hash);

    return send_work(msg_size, socket_fd, cur_client);
}

int check_uuid(const task_list_t *tasks, client_t *client) {
    u_char *client_uuid = &(client->buffer[MSG_LEN]);

    for (int uuid_num = 0; uuid_num < tasks->amount; ++uuid_num) {
        if (memcmp(tasks->tasks[uuid_num].uuid, client_uuid, UUID_LEN) == 0) {
            return uuid_num;
        }
    }
    return FAILURE_CODE;
}


int send_work(size_t msg_size, int socket_fd, client_t *cur_client) {
    if (send(socket_fd, cur_client->buffer, msg_size, 0) < msg_size) {
        return FAILURE_CODE;
    }
    cur_client->state = TO_ACK;
    cur_client->bytes_read = 0;

    return SUCCESS_CODE;
}

size_t fill_buf_with_hash(u_char *buffer, task_t *task, const char *hash) {
    memcpy(buffer, hash, MD5_DIGEST_LENGTH);

    return fill_buffer(&buffer[MD5_DIGEST_LENGTH], task) + MD5_DIGEST_LENGTH;
}

size_t fill_buffer(u_char *buffer, task_t *task) {
    size_t length = strlen(task->begin_str);

    uint16_t len_to_send = htons((uint16_t) length);

    size_t written = 0;
    memcpy_next(buffer, &len_to_send, LEN_LEN, &written);

    memcpy_next(buffer, task->begin_str, length, &written);
    memcpy_next(buffer, task->end_str, length, &written);

    return written;
}
