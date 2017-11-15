#include "handlers.h"
#include "../utils/memcpy_next.h"
#include "../utils/sock_utils.h"

#include <stdint.h>
#include <openssl/md5.h>
#include <arpa/inet.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

static char new_msg[] = NEW_MSG;
static char more_msg[] = MORE_MSG;
static char ack_msg[] = ACK_MSG;
static char match_msg[] = MATCH_MSG;

void set_uuid(client_t *client, const u_char *client_uuid);

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


handle_res_t try_handle_to_ack(const client_t *cur_client) {
    if (cur_client->bytes_read < MSG_LEN) {
        return UNFINISHED;
    }

    return handle_to_ack(cur_client->buffer);
}

handle_res_t try_handle_success(
        const task_list_t *tasks_list,
        int sock_num,
        client_t *cur_client,
        success_t *success
) {
    if(success->happened) {
        return FAILURE_RES;
    }

    if (cur_client->bytes_read < CONF_LEN) {
        return UNFINISHED;
    }

    handle_res_t result = handle_success(tasks_list, cur_client);
    if (result == SUCCESS_RES) {
        //fprintf(stderr, "SUCCESS CONFIRMED\n");
        success->happened = TRUE;
        success->num = sock_num;
    } else {
        return FAILURE_RES;
    }

    handle_res_t succeed = check_success(success, cur_client);
    return succeed;
}

handle_res_t try_handle_more(task_manager_t *task_maker, int socket_fd, client_t *cur_client) {
    if (cur_client->bytes_read < CONF_LEN) {
        return UNFINISHED;
    }

    return handle_more(task_maker, socket_fd, cur_client);

}

handle_res_t try_handle_new(task_manager_t *task_maker, int socket_fd, client_t *cur_client) {
    if (cur_client->bytes_read < CONF_LEN) {
        return UNFINISHED;
    }

    return handle_new(task_maker, socket_fd, cur_client);
}

handle_res_t try_handle_close(const task_manager_t *task_maker, int sock_fd, client_t *cur_client) {
    if (cur_client->bytes_read < CONF_LEN) {
        return UNFINISHED;
    }

    return handle_close(task_maker, sock_fd, cur_client);
}

handle_res_t handle_close(const task_manager_t *task_manager, int sock_fd, client_t *cur_client) {
    int task_num = check_uuid(task_manager->tasks_going, cur_client);

    if (task_num == FAILURE_CODE) {
        //fprintf(stderr, "UUID check on closing failed\n");
        return FAILURE_RES;
    }

    u_char *uuid = &cur_client->buffer[MSG_LEN];
    set_uuid(cur_client, uuid);

    return send_done(sock_fd, cur_client);

}

handle_res_t send_done(int sock_fd, client_t *cur_client) {
    memcpy(cur_client->buffer, DONE_MSG, MSG_LEN);

    int result = send_all(sock_fd, cur_client->buffer, 0, MSG_LEN);
    if(result == FAILURE_CODE) {
        return FAILURE_RES;
    }

    cur_client->state = TO_ACK;
    cur_client->bytes_read = 0;
    
    return UNFINISHED;
}

handle_res_t try_handle_unknown(client_t *cur_client) {
    if (cur_client->bytes_read < MSG_LEN) {
        return UNFINISHED;
    }

    client_state state = get_status(cur_client->buffer);

    if (state == UNKNOWN) {
        return FAILURE_RES;
    }

    cur_client->state = state;
    return UNFINISHED;
}


handle_res_t handle_to_ack(void *buffer) {
    if (memcmp(buffer, ack_msg, MSG_LEN) == 0) {
        return SUCCESS_RES;
    }

    return FAILURE_RES;
}

handle_res_t handle_success(const task_list_t *tasks, client_t *cur_client) {
    int task_num = check_uuid(tasks, cur_client);

    if (task_num == FAILURE_CODE) {
        //fprintf(stderr, "SUCCESS UUID CHECK FAILED\n");
        return FAILURE_RES;
    }

    u_char *uuid = &cur_client->buffer[MSG_LEN];
    set_uuid(cur_client, uuid);

    return SUCCESS_RES;
}


handle_res_t handle_more(task_manager_t *tasks_manager, int socket_fd, client_t *cur_client) {
    int task_num = check_uuid(tasks_manager->tasks_to_do, cur_client);
    
    if (task_num == FAILURE_CODE) {
        return FAILURE_RES;
    }

    u_char *uuid = &cur_client->buffer[MSG_LEN];
    set_uuid(cur_client, uuid);
    task_t *task = get_task(tasks_manager, uuid);
    
    //fprintf(stderr, "%s--%s\n", task->begin_str, task->end_str);

    size_t msg_size = fill_buf_more(cur_client->buffer, task);

    return send_work(msg_size, socket_fd, cur_client);

}

handle_res_t handle_new(task_manager_t *task_maker, int socket_fd, client_t *cur_client) {
    u_char *uuid = &cur_client->buffer[MSG_LEN];
    set_uuid(cur_client, uuid);
    task_t *task = get_task(task_maker, uuid);
    //fprintf(stderr, "%s--%s\n", task->begin_str, task->end_str);

    size_t msg_size = fill_buf_new(cur_client->buffer, task, task_maker->hash);

    return send_work(msg_size, socket_fd, cur_client);
}

int check_uuid(const task_list_t *tasks, client_t *client) {
    u_char *client_uuid = &(client->buffer[MSG_LEN]);
    //print_uuid(client_uuid);
    //printf("%d\n", tasks->amount);

    for (int uuid_num = 0; uuid_num < tasks->amount; ++uuid_num) {
        //print_uuid(tasks->tasks[uuid_num]->uuid);
        if (memcmp(tasks->tasks[uuid_num]->uuid, client_uuid, UUID_LEN) == 0) {
            return uuid_num;
        }
    }
    return FAILURE_CODE;
}

handle_res_t check_success(success_t *success, const client_t *successor) {
    if ((success->str_len < 1) && successor->bytes_read >= READ_LEN) {
        u_char *buffer = successor->buffer;
        uint16_t len_recv;
        memcpy(&len_recv, &buffer[CONF_LEN], LEN_LEN);
        success->str_len = ntohs(len_recv);
        if(success->str_len < 1) {
            return FAILURE_RES;
        }
    }

    if(success->str_len > 0) {
        int all_read = READ_LEN + success->str_len;
        if (successor->bytes_read >= all_read) {
            successor->buffer[all_read] = '\0';
            memcpy(success->answer, &successor->buffer[READ_LEN], success->str_len + 1);;
            return SUCCESS_RES;
        }
    }

    return UNFINISHED;
}

void set_uuid(client_t *client, const u_char *client_uuid) {
    memcpy(client->uuid, client_uuid, UUID_LEN);
    client->got_uuid = TRUE;
}

void print_uuid(const u_char *client_uuid) {
    printf("UUID: ");
    for(int i = 0; i < 16; i+= 2) {
        printf("%02x", client_uuid[i]);
    }
    printf("\n");
}

handle_res_t send_work(size_t msg_size, int socket_fd, client_t *cur_client) {
    int result = send_all(socket_fd, cur_client->buffer, 0, msg_size);

    if(result == FAILURE_CODE) {
        return FAILURE_RES;
    }
    ///write(0, cur_client->buffer, msg_size);

    cur_client->state = TO_ACK;
    cur_client->bytes_read = 0;

    return UNFINISHED;
}

size_t fill_buf_new(u_char *buffer, task_t *task, const char *hash) {
    size_t written = 0;

    memcpy_next(buffer, DO_MSG, MSG_LEN, &written);
    memcpy_next(buffer, hash, MD5_DIGEST_LENGTH, &written);

    return fill_task_buf(&buffer[written], task) + written;
}

size_t fill_buf_more(u_char *buffer, task_t *task) {
    memcpy(buffer, DO_MSG, MSG_LEN);

    return fill_task_buf(&buffer[MSG_LEN], task) + MSG_LEN;

}

size_t fill_task_buf(u_char *buffer, task_t *task) {
    size_t begin_len = strlen(task->begin_str);
    uint16_t len_to_send = htons((uint16_t) begin_len);

    size_t written = 0;
    memcpy_next(buffer, &len_to_send, LEN_LEN, &written);
    memcpy_next(buffer, task->begin_str, begin_len, &written);

    size_t end_len = strlen(task->end_str);
    len_to_send = htons((uint16_t) end_len);

    memcpy_next(buffer, &len_to_send, LEN_LEN, &written);
    memcpy_next(buffer, task->end_str, end_len, &written);

    return written;
}
