#ifndef NET5_HANDLERS_H
#define NET5_HANDLERS_H

#include "task_maker.h"
#include "protocol.h"
#include "get_task.h"

int try_handle_unknown(client_t *cur_client);

int try_handle_to_ack(const client_t *cur_client);

int try_handle_new(task_maker_t *tasks_srt, int socket_fd, client_t *cur_client);

int try_handle_more(task_maker_t *tasks_srt_gen, int socket_fd, client_t *cur_client);

int try_handle_success(const task_list_t *tasks_list, int sock_num, client_t *cur_client, success_t *success);

int handle_to_ack(void *buffer);

int handle_new(task_maker_t *task_maker, int socket_fd, client_t *cur_client);

int handle_more(task_maker_t *tasks_str, int socket_fd, client_t *cur_client);

int handle_success(const task_list_t *tasks, client_t *cur_client);

size_t fill_buf_with_hash(u_char *buffer, task_t *task, const char *hash);

size_t fill_buffer(u_char *buffer, task_t *task);

int check_uuid(const task_list_t *tasks, client_t *client);

int send_work(size_t msg_size, int socket_fd, client_t *cur_client);

#endif //NET5_HANDLERS_H
