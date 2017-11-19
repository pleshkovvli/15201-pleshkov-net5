#ifndef NET5_SERVER_H
#define NET5_SERVER_H

#include "i-server.h"
#include "cur_clients/client_type.h"
#include "cur_clients/cur_clients.h"
#include "task_manager/task_manager.h"
#include "success/success.h"
#include "../utils/include/logger.h"

#define TIMEOUT_MS 3000
#define TASK_TIMEOUT_SEC 40

typedef enum server_state {WORKING, CLOSING, LIMITED} server_state_t;
typedef enum handle_res {SUCCESS_RES, FAILURE_RES, UNFINISHED} handle_res_t;

typedef struct server {
    int sock_fd;

    struct pollfd *sock_polls;
    int events;

    client_t *cur_client;
    struct pollfd *cur_poll;
    int client_num;

    cur_clients_t *cur_clients;
    task_manager_t *task_manager;
    success_t *success;
    server_state_t state;

    logger_t *logger;
} server_t;

task_t *next_task(struct server *server);

handle_res_t handle_event(server_t *server);

void server_poll(server_t *server);

void check_server_tasks(server_t *server);

void check_all_clients(server_t *server);

void check_one_client(server_t *server);

int is_num_for_poll(const server_t *server, int client_num);

handle_res_t check_accept(server_t *server);

void handle_client_failure(server_t *server);

void move_task_if_got_uuid(task_manager_t *task_manager, client_t *cur_client);

void process_handle_result(server_t *server, handle_res_t handle_result);

handle_res_t try_handle_working(server_t *server);

int init_server(server_t *server, const char *hash, uint16_t port);

void destroy_server(server_t *server);

int running(server_t *server);

#endif //NET5_SERVER_H
