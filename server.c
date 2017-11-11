#include "server.h"

void run_server(const char *hash, uint16_t port) {
    int server_socket_fd = server_socket(port);

    if (server_socket_fd == FAILURE_CODE) {
        exit(EXIT_FAILURE);
    }

    task_maker_t task_maker;
    init_task_maker(&task_maker, hash);

    struct pollfd *socket_polls = malloc(sizeof(struct pollfd) * (MAX_CLIENTS + 1));

    socket_polls[0].fd = server_socket_fd;
    socket_polls[0].events = POLLIN;
    socket_polls[0].revents = 0;

    cur_clients_t cur_clients;

    init_cur_clients(&cur_clients, socket_polls);

    success_t success;
    init_success(&success);

    while (1) {
        int events = poll(socket_polls, cur_clients.amount + 1, TIMEOUT);

        check_tasks(task_maker.tasks_list);

        if (events == 0) {
            continue;
        }

        if (success.happened) {
            int result = deal_with_success(&cur_clients, &success);
            if (result == SUCCESS_CODE) {
                break;
            }
        }

        if (socket_polls[0].revents == POLLIN) {
            --events;
            int client_fd = accept(server_socket_fd, NULL, NULL);
            add_client(&cur_clients, client_fd);
        }

        for (int sock_num = 0; sock_num < cur_clients.amount && events > 0; ++sock_num) {
            struct pollfd *cur_poll = &socket_polls[sock_num];
            if (cur_poll->events == 0) {
                continue;
            }

            --events;
            if (cur_poll->events != POLLIN) {
                remove_client(&cur_clients, sock_num);
                continue;
            }

            client_t *cur_client = &(cur_clients.clients[sock_num]);

            ssize_t read = recv(cur_poll->fd, cur_client->buffer, CLIENT_BUF_SIZE, 0);

            if (read < 0) {
                remove_client(&cur_clients, sock_num);
                continue;
            }

            cur_client->bytes_read += read;

            int error_check = SUCCESS_CODE;

            if (cur_client->state == UNKNOWN) {
                error_check = try_handle_unknown(cur_client);
            }

            switch (cur_client->state) {
                case UNKNOWN:
                    break;
                case NEW: {
                    error_check = try_handle_new(&task_maker, cur_poll->fd, cur_client);
                }
                    break;
                case MORE: {
                    error_check = try_handle_more(&task_maker, cur_poll->fd, cur_client);
                }
                    break;
                case SUCCESS: {
                    error_check = try_handle_success(
                            task_maker.tasks_list, sock_num, cur_client, &success);
                }
                    break;
                case TO_ACK: {
                    try_handle_to_ack(cur_client);
                }
                    break;
            }

            if (error_check == FAILURE_CODE) {
                remove_client(&cur_clients, sock_num);
            }
        }
    }

    destroy_task_maker(&task_maker);
    destroy_cur_clients(&cur_clients);
}

int deal_with_success(cur_clients_t *cur_clients, success_t *success) {
    struct pollfd *success_poll = &(cur_clients->polls[success->num]);
    client_t *successor = &(cur_clients->clients[success->num]);

    if (success_poll->revents == 0) {
        return FAILURE_CODE;
    }

    if (success_poll->revents != POLLIN) {
        remove_client(cur_clients, success->num);
        success->happened = FALSE;
        success->num = -1;
        return FAILURE_CODE;
    }

    ssize_t read = recv(success_poll->fd, successor->buffer, CLIENT_BUF_SIZE, 0);

    if (read < 0) {
        remove_client(cur_clients, success->num);
        success->happened = FALSE;
        success->num = -1;
        return FAILURE_CODE;
    }

    if ((success->str_length < 1) && successor->bytes_read >= READ_LEN) {
        u_char *buffer = successor->buffer;
        memcpy(&success->str_length, &buffer[CONF_LEN], LEN_LEN);
        success->str_length = ntohs((*success).str_length);
    }

    int all_read = READ_LEN + success->str_length;
    if (successor->bytes_read >= all_read) {
        successor->buffer[all_read] = '\0';
        return SUCCESS_CODE;
    }

    return FAILURE_CODE;
}

void check_tasks(const task_list_t *task_list) {
    time_t cur_time = time(NULL);
    task_t *tasks = task_list->tasks;

    for (int i = 0; i < task_list->amount; ++i) {
        time_t cur_timestamp = tasks[i].timestamp;
        if ((cur_time - cur_timestamp) > TIME_TO_DO) {
            tasks[i].status = TO_DO;
        }
    }
}

void init_success(success_t *success) {
    success->happened = FALSE;
    success->num = -1;
    success->str_length = 0;
}

int server_socket(uint16_t port) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0) {
        perror("Open socket error.");
        return FAILURE_CODE;
    }

    struct sockaddr_in address;
    socklen_t addr_size = sizeof(struct sockaddr_in);

    memset(&address, 0, addr_size);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    int result_code = bind(
            socket_fd,
            (struct sockaddr *) &address,
            addr_size
    );

    if (result_code != SUCCESS_CODE) {
        perror("Bind socket error.");
        return FAILURE_CODE;
    }

    result_code = listen(socket_fd, 32);

    if (result_code != SUCCESS_CODE) {
        perror("Listen socket error.");
        return FAILURE_CODE;
    }

    return socket_fd;
}

