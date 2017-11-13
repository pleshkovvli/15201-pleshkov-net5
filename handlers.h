#ifndef NET5_HANDLERS_H
#define NET5_HANDLERS_H

#include "task_maker.h"
#include "protocol.h"
#include "get_task.h"

enum to_close {CLOSE_SOCK, CLOSE_TASK, DONT_CLOSE};

enum to_close try_handle_unknown(client_t *cur_client);

enum to_close try_handle_to_ack(const client_t *cur_client);

enum to_close try_handle_new(task_maker_t *task_maker, int socket_fd, client_t *cur_client, int closing);

enum to_close try_handle_more(task_maker_t *task_maker, int socket_fd, client_t *cur_client, int closing);

enum to_close try_handle_success(const task_list_t *tasks_list, int sock_num, client_t *cur_client, success_t *success);

enum to_close try_handle_first(task_maker_t *task_maker, char* hash, int socket_fd, client_t *cur_client, int *first);

enum to_close handle_to_ack(void *buffer);

enum to_close handle_new(task_maker_t *task_maker, int socket_fd, client_t *cur_client);

enum to_close handle_more(task_maker_t *tasks_str, int socket_fd, client_t *cur_client);

enum to_close handle_success(const task_list_t *tasks, client_t *cur_client);

void print_uuid(const u_char *client_uuid);

size_t fill_buf_with_hash(u_char *buffer, task_t *task, const char *hash);

size_t fill_buffer(u_char *buffer, task_t *task);

int check_uuid(const task_list_t *tasks, client_t *client);

enum to_close send_work(size_t msg_size, int socket_fd, client_t *cur_client);

#endif //NET5_HANDLERS_H
