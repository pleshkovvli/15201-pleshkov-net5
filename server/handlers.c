#include <arpa/inet.h>
#include <memory.h>
#include <openssl/md5.h>
#include "../protocol.h"
#include "cur_clients/client_type.h"
#include "handlers.h"
#include "../agreements.h"
#include "../utils/include/sock_utils.h"
#include "../utils/include/memcpy_next.h"

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

handle_res_t handle_to_ack(void *buffer) {
    if (memcmp(buffer, ack_msg, MSG_LEN) == 0) {
        return SUCCESS_RES;
    }

    return FAILURE_RES;
}

handle_res_t try_handle_success(server_t *server) {
    if(server->success->happened) {
        return FAILURE_RES;
    }

    if (server->cur_client->bytes_read < CONF_LEN) {
        return UNFINISHED;
    }

    handle_res_t result = check_set_uuid(server->task_manager->tasks_going, server->cur_client);
    if (result == SUCCESS_RES) {
        server->success->happened = TRUE;
    } else {
        return FAILURE_RES;
    }

    handle_res_t process_res = process_success(server->success, server->cur_client);

    if(process_res == SUCCESS_CODE) {
        send_done(server);
    }

    return process_res;
}

handle_res_t check_set_uuid(const task_list_t *tasks, client_t *cur_client) {
    int check_res = check_uuid(tasks, cur_client);

    if (check_res == FAILURE_CODE) {
        return FAILURE_RES;
    }

    u_char *uuid = &cur_client->buffer[MSG_LEN];
    set_uuid(cur_client, uuid);

    return SUCCESS_RES;
}

handle_res_t process_success(success_t *success, const client_t *successor) {
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

handle_res_t try_handle_more(server_t *server) {
    if (server->cur_client->bytes_read < CONF_LEN) {
        return UNFINISHED;
    }

    return handle_more(server);

}

handle_res_t handle_more(server_t *server) {
    client_t *client = server->cur_client;

    int result = check_set_uuid(server->task_manager->tasks_to_do, client);
    if(result == FAILURE_RES) {
        return FAILURE_RES;

    }

    task_t *task = next_task(server);
    if(task == NULL) {
        return handle_closing(server);
    }
    
    size_t msg_size = fill_buf_more(client->buffer, task);

    return send_work(msg_size, server->cur_poll->fd, client);

}

handle_res_t try_handle_new(server_t *server) {
    if (server->cur_client->bytes_read < CONF_LEN) {
        return UNFINISHED;
    }

    return handle_new(server);
}

handle_res_t handle_new(server_t *server) {
    client_t *client = server->cur_client;
    
    u_char *uuid = &client->buffer[MSG_LEN];
    set_uuid(client, uuid);
    
    task_t *task = next_task(server);
    if(task == NULL) {
        return handle_closing(server);
    }
    
    size_t msg_size = fill_buf_new(client->buffer, task, server->task_manager->hash);

    return send_work(msg_size, server->cur_poll->fd, client);
}


handle_res_t try_handle_closing(server_t *server) {
    if (server->cur_client->bytes_read < CONF_LEN) {
        return UNFINISHED;
    }

    return handle_closing(server);
}

handle_res_t handle_closing(server_t *server) {
    client_t *client = server->cur_client;
    
    int task_num = check_uuid(server->task_manager->tasks_going, client);

    if (task_num == FAILURE_CODE) {
        return FAILURE_RES;
    }

    u_char *uuid = &client->buffer[MSG_LEN];
    set_uuid(client, uuid);

    return send_done(server);

}

handle_res_t send_done(server_t *server) {
    client_t *client = server->cur_client;
    memcpy(client->buffer, DONE_MSG, MSG_LEN);

    int result = send_all(server->cur_poll->fd, client->buffer, 0, MSG_LEN);
    if(result == FAILURE_CODE) {
        return FAILURE_RES;
    }

    client->state = TO_ACK;
    client->bytes_read = 0;
    
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

int check_uuid(const task_list_t *tasks, client_t *client) { ;
    for (int uuid_num = 0; uuid_num < tasks->amount; ++uuid_num) {
        if (memcmp(tasks->tasks[uuid_num]->uuid, client->uuid, UUID_LEN) == 0) {
            return uuid_num;
        }
    }

    return FAILURE_CODE;
}


void set_uuid(client_t *client, const u_char *client_uuid) {
    memcpy(client->uuid, client_uuid, UUID_LEN);
    client->got_uuid = TRUE;
}

handle_res_t send_work(size_t msg_size, int socket_fd, client_t *cur_client) {
    int result = send_all(socket_fd, cur_client->buffer, 0, msg_size);

    if(result == FAILURE_CODE) {
        return FAILURE_RES;
    }

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
