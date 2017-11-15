#ifndef NET5_HANDLERS_H
#define NET5_HANDLERS_H

#include "../task_manager/task_manager.h"
#include "../protocol.h"
#include "success.h"


typedef enum handle_res {SUCCESS_RES, FAILURE_RES, UNFINISHED} handle_res_t;

handle_res_t try_handle_unknown(client_t *cur_client);

handle_res_t try_handle_to_ack(const client_t *cur_client);

handle_res_t try_handle_new(task_manager_t *task_maker, int socket_fd, client_t *cur_client);

handle_res_t try_handle_more(task_manager_t *task_maker, int socket_fd, client_t *cur_client);

handle_res_t try_handle_success(const task_list_t *tasks_list, int sock_num, client_t *cur_client, success_t *success);

handle_res_t try_handle_close(const task_manager_t *task_maker, int sock_fd, client_t *cur_client);

handle_res_t handle_to_ack(void *buffer);

handle_res_t handle_new(task_manager_t *task_maker, int socket_fd, client_t *cur_client);

handle_res_t handle_more(task_manager_t *tasks_manager, int socket_fd, client_t *cur_client);

handle_res_t handle_success(const task_list_t *tasks, client_t *cur_client);

handle_res_t handle_close(const task_manager_t *task_manager, int sock_fd, client_t *cur_client);

handle_res_t check_success(success_t *success, const client_t *successor);

handle_res_t send_done(int sock_fd, client_t *cur_client);

void print_uuid(const u_char *client_uuid);

size_t fill_buf_new(u_char *buffer, task_t *task, const char *hash);

size_t fill_task_buf(u_char *buffer, task_t *task);

int check_uuid(const task_list_t *tasks, client_t *client);

enum handle_res send_work(size_t msg_size, int socket_fd, client_t *cur_client);

size_t fill_buf_more(u_char *buffer, task_t *task);

#endif //NET5_HANDLERS_H
