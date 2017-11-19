#include <sys/socket.h>
#include "server.h"
#include "../agreements.h"
#include "sock_polls/sock_polls.h"
#include "../utils/include/sock_utils.h"
#include "handlers.h"

#define MAX_STR_LEN 13
#define MAX_CLIENTS 1024

void run_server(const char *hash, uint16_t port) {
    server_t *server = malloc(sizeof(server_t));
    int res = init_server(server, hash, port);
    if(res == FAILURE_CODE) {
        exit(EXIT_FAILURE);
    }

    while (running(server)) {
        server_poll(server);
        check_server_tasks(server);
        if (check_accept(server) == FAILURE_RES) {
            exit(EXIT_FAILURE);
        }
        check_all_clients(server);
    }

    if(server->success->happened) {
        printf("SUCCESS: answer is %s\n", server->success->answer);
    } else {
        printf("FAILED: max length is exceeded\n");
    }


    destroy_server(server);
    free(server);
}


void check_server_tasks(server_t *server) {
    log_simple(server->logger, "Checking tasks");

    ushort going_before = server->task_manager->tasks_going->amount;
    check_tasks(server->task_manager, TASK_TIMEOUT_SEC);
    ushort going_after = server->task_manager->tasks_going->amount;

    fprintf(server->logger->stream,
            "%s: %d tasks moved in to do\n",
            server->logger->tag,
            going_before - going_after
    );
}

void check_all_clients(server_t *server) {
    cur_clients_t *clients = server->cur_clients;

    for (int client_num = 0; is_num_for_poll(server, client_num); ++client_num) {
        server->cur_poll = &(clients->polls[client_num]);
        server->cur_client = &(clients->clients[client_num]);
        server->client_num = client_num;

        check_one_client(server);
    }
}

int is_num_for_poll(const server_t *server, int client_num) {
    return (client_num < server->cur_clients->amount) && (server->events > 0);
}

void check_one_client(server_t *server) {
    handle_res_t event_res = handle_event(server);
    if (event_res != SUCCESS_RES) {
        return;
    }

    handle_res_t handle_result = UNFINISHED;

    log_simple(server->logger, "Processing client");
    if (server->cur_client->state == UNKNOWN) {
        log_simple(server->logger, "Processing UNKNOWN");
        handle_result = try_handle_unknown(server->cur_client);
    }

    if (handle_result == UNFINISHED) {
        if (server->state == CLOSING && server->cur_client->state != TO_ACK) {
            log_simple(server->logger, "Processing CLOSING");
            handle_result = try_handle_closing(server);
        } else {
            handle_result = try_handle_working(server);
        }
    }

    server->cur_poll->revents = 0;
    process_handle_result(server, handle_result);
}

void server_poll(server_t *server) {
    log_simple(server->logger, "Polling...");
    nfds_t poll_amount = server->cur_clients->amount + 1;
    server->events = poll(server->sock_polls, poll_amount, TIMEOUT_MS);

    if(server->events == 0) {
        log_simple(server->logger, "Empty poll");
    } else {
        fprintf(server->logger->stream, "%s: %d events\n", server->logger->tag, server->events);
    }
}

handle_res_t handle_event(server_t *server) {
    if (server->cur_poll->revents == 0) {
        return UNFINISHED;
    }

    --(server->events);

    if (server->cur_poll->revents != POLLIN) {
        log_simple(server->logger, "Client disconnected");
        handle_client_failure(server);
        return FAILURE_RES;
    }

    ssize_t read = recv(server->cur_poll->fd, server->cur_client->buffer, CLIENT_BUF_SIZE, 0);

    if (read < 0) {
        log_simple(server->logger, "Failed to read from client");
        handle_client_failure(server);
        return FAILURE_RES;
    }

    fprintf(server->logger->stream, "%s: %ld bytes read from client\n", server->logger->tag, read);

    server->cur_client->bytes_read += read;
    return SUCCESS_RES;
}

handle_res_t try_handle_working(server_t *server) {
    switch (server->cur_client->state) {
        case UNKNOWN:
            break;
        case NEW: {
            log_simple(server->logger, "Processing NEW");
            return try_handle_new(server);
        }
        case MORE: {
            log_simple(server->logger, "Processing MORE");
            return try_handle_more(server);
        }
        case SUCCESS: {
            log_simple(server->logger, "Processing SUCCESS");
            return try_handle_success(server);
        }
        case TO_ACK: {
            log_simple(server->logger, "Processing TO_ACK");
            return try_handle_to_ack(server->cur_client);
        }
    }
}


task_t *next_task(server_t *server) {
    task_manager_t *task_manager = server->task_manager;
    task_list_t *going = task_manager->tasks_going;

    ushort task_num = get_task_num(going, server->cur_client->uuid);
    if(task_num < going->amount) {
        remove_task_by_num(going, task_num);
    }

    task_list_t *to_do = task_manager->tasks_to_do;
    task_t *task;

    if(to_do->amount > 0) {
        task = to_do->tasks[to_do->amount - 1];
        --to_do->amount;
    } else {
        if(server->state == LIMITED) {
            return NULL;
        }
        str_gen_t *str_gen = task_manager->str_gen;
        task = make_task(str_gen->begin_str, str_gen->end_str, server->cur_client->uuid);
        if(next_str(str_gen) == FAILURE_CODE) {
            server->state = LIMITED;
        }

    }

    add_task(going, task);

    return task;
}


void process_handle_result(server_t *server, handle_res_t handle_result) {
    if (handle_result != UNFINISHED) {
        log_simple(server->logger, "Removing client");

        if (handle_result == FAILURE_RES) {
            move_task_if_got_uuid(server->task_manager, server->cur_client);
        }
        remove_client(server->cur_clients, server->client_num);
    } else {
        log_simple(server->logger, "Result is UNFINISHED");
    }
}

void handle_client_failure(server_t *server) {
    move_task_if_got_uuid(server->task_manager, server->cur_client);
    remove_client(server->cur_clients, server->client_num);
}

void move_task_if_got_uuid(task_manager_t *task_manager, client_t *cur_client) {
    if (!cur_client->got_uuid) {
        return;
    }

    int uuid_num = check_uuid(task_manager->tasks_going, cur_client);
    if (uuid_num != FAILURE_CODE) {
        move_task(task_manager->tasks_to_do, task_manager->tasks_going, uuid_num);
    }
}

handle_res_t check_accept(server_t *server) {
    short server_revents = server->sock_polls[0].revents;
    if (server_revents == 0) {
        return UNFINISHED;
    }

    --(server->events);
    if (server_revents != POLLIN) {
        log_simple(server->logger, "Server socket failed!!!");
        return FAILURE_RES;
    }

    int client_fd = accept(server->sock_fd, NULL, NULL);
    if(client_fd < 0) {
        log_simple(server->logger, "Accept failed!");
        close(client_fd);
        return FAILURE_RES;
    }
    if(server->cur_clients->amount < server->cur_clients->max_clients) {
        add_client(server->cur_clients, client_fd);
        log_simple(server->logger, "Client accepted");
    } else {
        log_simple(server->logger, "Too much clients!");
        close(client_fd);
    }

    return SUCCESS_RES;
}


int init_server(server_t *server, const char *hash, uint16_t port) {
    server->logger = malloc(sizeof(logger_t));
    init_logger(server->logger, "SERVER", stderr);

    log_simple(server->logger, "Starting server...");

    server->sock_fd = server_socket(port);

    if (server->sock_fd == FAILURE_CODE) {
        log_simple(server->logger, "Failed to create socket");
        free(server->logger);
        return FAILURE_CODE;
    }


    server->cur_clients = malloc(sizeof(cur_clients_t));
    server->success = malloc(sizeof(success_t));
    server->task_manager = malloc(sizeof(task_manager_t));

    init_sock_polls(&server->sock_polls, server->sock_fd, 0);

    init_cur_clients(server->cur_clients, server->sock_polls, MAX_CLIENTS);
    init_success(server->success, MAX_STR_LEN);
    init_task_manager(server->task_manager, hash, MAX_STR_LEN, "ACGT");

    log_simple(server->logger, "Server started");

    return SUCCESS_CODE;
}

void destroy_server(server_t *server) {
    log_simple(server->logger, "Finishing server");

    close(server->sock_fd);

    destroy_success(server->success);
    destroy_task_manager(server->task_manager);
    destroy_cur_clients(server->cur_clients);

    destroy_sock_polls(server->sock_polls);

    free(server->cur_clients);
    free(server->success);
    free(server->task_manager);
    free(server->logger);
}

int running(server_t *server) {
    ushort going_amount = server->task_manager->tasks_going->amount;
    ushort to_do_amount = server->task_manager->tasks_to_do->amount;
    nfds_t clients_amount = server->cur_clients->amount;

    fprintf(stderr,
            "Going=%d, to do=%d, clients=%ld\n",
            going_amount, to_do_amount, clients_amount
    );

    switch (server->state) {
        case WORKING: {
            fprintf(stderr, "WORKING\n");
            return TRUE;
        }
        case CLOSING: {
            fprintf(stderr, "CLOSING\n");
            return (going_amount > 0) || (clients_amount > 0);
        }
        case LIMITED: {
            fprintf(stderr, "LIMITED\n");
            return (to_do_amount > 0) || (going_amount > 0) || (clients_amount > 0);
        }
    }
}
