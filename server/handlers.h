#ifndef NET5_HANDLERS_H
#define NET5_HANDLERS_H

#include "cur_clients/client_type.h"
#include "server.h"

typedef enum handle_res {SUCCESS_RES, FAILURE_RES, UNFINISHED} handle_res_t;

handle_res_t try_handle_unknown(client_t *cur_client);

handle_res_t try_handle_to_ack(const client_t *cur_client);

handle_res_t try_handle_new(server_t *server);

handle_res_t try_handle_more(server_t *server);

handle_res_t try_handle_success(server_t *server);

handle_res_t try_handle_closing(server_t *server);

handle_res_t handle_to_ack(void *buffer);

handle_res_t handle_new(server_t *server);

handle_res_t handle_more(server_t *server);

handle_res_t check_set_uuid(const task_list_t *tasks, client_t *cur_client);

handle_res_t handle_closing(server_t *server);

handle_res_t process_success(success_t *success, const client_t *successor);

handle_res_t send_done(server_t *server);

size_t fill_buf_new(u_char *buffer, task_t *task, const char *hash);

size_t fill_task_buf(u_char *buffer, task_t *task);

int check_uuid(const task_list_t *tasks, client_t *client);

handle_res_t send_work(size_t msg_size, int socket_fd, client_t *cur_client);

size_t fill_buf_more(u_char *buffer, task_t *task);

#endif //NET5_HANDLERS_H
