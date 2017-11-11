#include <openssl/md5.h>
#include "server.h"

static char register_msg[] = "_NEW";
static char more_msg[] = "MORE";
static char match_msg[] = "MTCH";
static char ack_msg[] = "_ACK";

static const uint16_t PORT = 3112;

int try_handle_unknown(client_t *cur_client);

int try_handle_new(tasks_srt_t *tasks_srt, int socket_fd, client_t *cur_client);

int try_handle_more(tasks_srt_t *tasks_srt_gen, int socket_fd, client_t *cur_client);

int try_handle_success(const task_list_t *tasks_list, int sock_num, client_t *cur_client,
                       poll_clients_t *poll_clients, int *success, int *success_num);

void try_handle_to_ack(const client_t *cur_client);

int run_server(const char *hash) {
    int server_socket_fd = server_socket(PORT);

    if (server_socket_fd < 0) {
        fprintf(stderr, "Error on socket creation. %s\n", strerror(server_socket_fd));
        exit(EXIT_FAILURE);
    }


    tasks_srt_t tasks_srt_gen;
    tasks_srt_gen.hash = hash;

    task_list_t *tasks_list = &tasks_srt_gen.tasks_list;
    tasks_list->tasks = malloc(sizeof(task_t) * MAX_CLIENTS);
    tasks_list->amount = 0;

    struct pollfd socket_polls[MAX_CLIENTS + 1];
    socket_polls[0].fd = server_socket_fd;
    socket_polls[0].events = POLLIN;

    poll_clients_t poll_clients;

    init_poll_clients(&poll_clients, socket_polls);

    if (init_string_gen(&tasks_srt_gen.str_gen) == FAILURE_CODE) {
        exit(1);
    }

    int success = FALSE;
    int success_num = -1;

    ushort success_len = 0;

    while (1) {
        int events = poll(socket_polls, poll_clients.amount + 1, TIMEOUT);

        for (int i = 0; i < tasks_list->amount; ++i) {
            if ((time(NULL) - tasks_list->tasks[i].timestamp) > TIME_TO_DO) {
                tasks_list->tasks[i].status = TO_DO;
            }
        }

        if (success && (poll_clients.polls[success_num].revents == POLLIN)) {
            client_t *successor = &(poll_clients.clients[success_num]);
            struct pollfd *success_poll = &(poll_clients.polls[success_num]);

            ssize_t read = recv(success_poll->fd, successor->buffer, CLIENT_BUF_SIZE, 0);

            if (read < 0) {
                remove_client(success_num, &poll_clients);
                success = FALSE;
                success_num = -1;
                continue;
            }

            if ((success_len < 1) && successor->bytes_read >= READ_LEN) {
                u_char *buffer = successor->buffer;
                memcpy(&success_len, &buffer[CONF_LEN], 2);
                success_len = ntohs(success_len);
            }

            int all_read = READ_LEN + success_len;
            if (successor->bytes_read >= all_read) {
                successor->buffer[all_read] = '\0';
                printf("%s\n", &(successor->buffer[READ_LEN]));
                exit(0);
            }
        }

        if (socket_polls[0].revents == POLLIN) {
            --events;
            int client_fd = accept(server_socket_fd, NULL, NULL);
            add_client(&poll_clients, client_fd);
        }

        for (int sock_num = 0; sock_num < poll_clients.amount && events > 0; ++sock_num) {
            struct pollfd *cur_poll = &socket_polls[sock_num];
            if (cur_poll->events == 0) {
                continue;
            }

            --events;
            if (cur_poll->events != POLLIN) {
                remove_client(sock_num, &poll_clients);
                continue;
            }

            client_t *cur_client = &(poll_clients.clients[sock_num]);

            ssize_t read = recv(cur_poll->fd, cur_client->buffer, CLIENT_BUF_SIZE, 0);

            if (read < 0) {
                remove_client(sock_num, &poll_clients);
                continue;
            }

            cur_client->bytes_read += read;

            int error_check = SUCCESS_CODE;
            if(cur_client->state == UNKNOWN) {
                error_check = try_handle_unknown(cur_client);
            }
            switch (cur_client->state) {
                case UNKNOWN:
                    break;
                case NEW: {
                    error_check = try_handle_new(&tasks_srt_gen, cur_poll->fd, cur_client);
                }
                    break;
                case MORE: {
                    error_check = try_handle_more(&tasks_srt_gen, cur_poll->fd, cur_client);
                }
                    break;
                case SUCCESS: {
                    try_handle_success(tasks_list, sock_num, cur_client, &poll_clients, &success, &success_num);
                }
                    break;
                case TO_ACK: {
                    try_handle_to_ack(cur_client);
                }
                    break;
            }

            if(error_check == FAILURE_CODE) {
                remove_client(sock_num, &poll_clients);
            }
        }
    }
}

void try_handle_to_ack(const client_t *cur_client) {
    if (cur_client->bytes_read >= MSG_LEN) {
        handle_to_ack(cur_client->buffer);
    }
}

int try_handle_success(const task_list_t *tasks_list, int sock_num, client_t *cur_client,
                       poll_clients_t *poll_clients, int *success, int *success_num) {
    if ((cur_client->bytes_read >= CONF_LEN)) {
        int result = handle_success(tasks_list, cur_client);
        if (result == FAILURE_CODE) {
            remove_client(sock_num, poll_clients);
            (*success) = FALSE;
            (*success_num) = 0;
        } else {
            (*success) = TRUE;
            (*success_num) = sock_num;
        }
    }
}

int try_handle_more(tasks_srt_t *tasks_srt_gen, int socket_fd, client_t *cur_client) {
    if (cur_client->bytes_read < CONF_LEN) {
        return SUCCESS_CODE;
    }

    return handle_more(tasks_srt_gen, socket_fd, cur_client);

}

int try_handle_new(tasks_srt_t *tasks_srt, int socket_fd, client_t *cur_client) {
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

void init_poll_clients(poll_clients_t *poll_clients, struct pollfd *socket_polls) {
    poll_clients->clients = malloc(sizeof(client_t) * MAX_CLIENTS);
    poll_clients->polls = &socket_polls[1];
}

int server_socket(uint16_t port) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0) {
        perror("Failed to open socket.");
    }

    struct sockaddr_in address;
    socklen_t address_size = sizeof(struct sockaddr_in);

    memset(&address, 0, address_size);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    int result_code = bind(socket_fd, (struct sockaddr *) &address, address_size);
    if (result_code != SUCCESS_CODE) {
        return result_code;
    }

    result_code = listen(socket_fd, 32);
    if (result_code != SUCCESS_CODE) {
        return result_code;
    }

    return socket_fd;
}

int init_string_gen(string_gen_t *str_gen) {
    str_gen->reverse_table = malloc(sizeof(ushort) * 256);
    str_gen->current_string = malloc(sizeof(char) * 32);

    if (str_gen->reverse_table == NULL || str_gen->current_string == NULL) {
        return FAILURE_CODE;
    }

    str_gen->begin_string_suf = "AAAAAA";
    str_gen->end_string_suf = "TTTTTT";
    str_gen->alphabet = "ACGT";

    str_gen->alphabet_length = (ushort) strlen(str_gen->alphabet);

    for (ushort i = 0; i < (*str_gen).alphabet_length; ++i) {
        str_gen->reverse_table[(u_char) str_gen->alphabet[i]] = i;
    }

    return SUCCESS_CODE;
}

int handle_success(const task_list_t *tasks, client_t *cur_client) {
    if (check_uuid(tasks, cur_client) == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
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


int handle_more(tasks_srt_t *tasks_str, int socket_fd, client_t *cur_client) {
    int task_num = check_uuid(&tasks_str->tasks_list, cur_client);
    if (task_num == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    task_t *task = get_task(tasks_str, &cur_client->buffer[MSG_LEN]);
    size_t msg_size = fill_buffer(cur_client->buffer, task);

    return send_work(msg_size, socket_fd, cur_client);

}

int send_work(size_t msg_size, int socket_fd, client_t *cur_client) {
    if (send(socket_fd, cur_client->buffer, msg_size, 0) < msg_size) {
        return FAILURE_CODE;
    }
    cur_client->state = TO_ACK;
    cur_client->bytes_read = 0;

    return SUCCESS_CODE;
}


size_t fill_buf_with_hash(u_char *buffer, task_t *task, const char *hash);

int handle_new(tasks_srt_t *tasks_str, int socket_fd, client_t *cur_client) {

    task_t *task = get_task(tasks_str, &cur_client->buffer[MSG_LEN]);
    size_t msg_size = fill_buf_with_hash(cur_client->buffer, task, tasks_str->hash);

    return send_work(msg_size, socket_fd, cur_client);
}

task_t *get_task(tasks_srt_t *tasks_str, void *client_uuid) {
    int i;
    int amount = tasks_str->tasks_list.amount;
    task_t *tasks = tasks_str->tasks_list.tasks;

    int task_number = 0;
    for(;task_number < amount; ++task_number) {
        if(memcmp(tasks[task_number].uuid, client_uuid, UUID_LEN) == 0) {
            tasks[task_number].status = IN_PROGRESS;
            break;
        }
    }
    for (i = 0; i < amount; ++i) {
        if (tasks[i].status == TO_DO) {
            break;
        }
    }

    task_t *task;


    task = &tasks[task_number];
    if (i == amount) {
        get_next(&tasks_str->str_gen);
        fill_task(task, &(tasks_str->str_gen));
    } else {
        task_t past_task = tasks[i];
        task->status = past_task.status;
        size_t b_len = strlen(past_task.begin);
        memcpy(task->begin, past_task.begin, b_len);
        task->begin[b_len] = '\0';
        size_t e_len = strlen(past_task.end);
        memcpy(task->end, past_task.end, e_len);
        task->begin[e_len] = '\0';
    }
    
    if(task_number == amount) {
        ++(tasks_str->tasks_list.amount);
    }

    task->status = IN_PROGRESS;
    task->timestamp = time(NULL);
    memcpy(tasks[amount].uuid, client_uuid, UUID_LEN);
    return task;
}

client_state get_status(void *buffer) {
    if (memcmp(buffer, register_msg, MSG_LEN) == 0) {
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

int handle_to_ack(void *buffer) {
    if (memcmp(buffer, ack_msg, MSG_LEN) == 0) {
        return SUCCESS_CODE;
    }
    return FAILURE_CODE;
}


void memcpy_next(void *buffer, const void *src, size_t n, size_t *written);

void fill_task(task_t *task, string_gen_t *str_gen) {

    char *cur_string = str_gen->current_string;
    size_t length = strlen(cur_string);

    size_t written = 0;
    memcpy_next(task->begin, cur_string, length, &written);
    memcpy_next(task->begin, str_gen->begin_string_suf, SUF_LEN, &written);

    written = 0;
    memcpy_next(task->end, cur_string, length, &written);
    memcpy_next(task->end, str_gen->end_string_suf, SUF_LEN, &written);
}

size_t fill_buf_with_hash(u_char *buffer, task_t *task, const char *hash) {
    memcpy(buffer, hash, MD5_DIGEST_LENGTH);

    return fill_buffer(&buffer[MD5_DIGEST_LENGTH], task) + MD5_DIGEST_LENGTH;
}

size_t fill_buffer(u_char *buffer, task_t *task) {
    size_t length = strlen(task->begin);

    uint16_t len_to_send = htons((uint16_t) length);

    size_t written = 0;
    memcpy_next(buffer, &len_to_send, LEN_LEN, &written);

    memcpy_next(buffer, task->begin, length, &written);
    memcpy_next(buffer, task->end, length, &written);

    return written;
}

void memcpy_next(void *buffer, const void *src, size_t n, size_t *written) {
    memcpy(&buffer[*written], src, n);
    *written += n;
}

void get_next(string_gen_t *str_gen) {
    char *cur_string = str_gen->current_string;

    ushort length = (ushort) strlen(cur_string);
    enum next_action add = next_string(str_gen, length, 0);

    if (add == ADD) {
        cur_string[length] = str_gen->alphabet[0];
        cur_string[length + 1] = '\0';
    }
}

enum next_action {
    ADD, NOT_ADD
};

enum next_action next_string(string_gen_t *str_gen, ushort length, int index) {
    if (index == length - 1) {
        return change_sym(str_gen, index);

    }

    int result = next_string(str_gen, length, index + 1);

    if (result == ADD) {
        return change_sym(str_gen, index);
    }

    return NOT_ADD;

}

enum next_action change_sym(string_gen_t *str_gen, int index) {
    char *string = str_gen->current_string;
    const char *alphabet = str_gen->alphabet;
    const int alphabet_length = str_gen->alphabet_length;
    const ushort *reverse_table = str_gen->reverse_table;

    char cur_sym = string[index];
    ushort cur_position = reverse_table[cur_sym];

    int next_position = (cur_position + 1) % alphabet_length;

    string[index] = alphabet[next_position];
    return (next_position == 0) ? ADD : NOT_ADD;
}

void remove_client(int number, poll_clients_t *poll_clients) {
    --(poll_clients->amount);
    nfds_t last = poll_clients->amount;


    struct pollfd *polls = poll_clients->polls;

    int socket_fd = polls[number].fd;
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);

    client_t *clients = poll_clients->clients;

    free(clients[number].buffer);

    if (number < last) {
        polls[number].fd = polls[last].fd;
        polls[number].events = polls[last].events;
        polls[number].revents = polls[last].revents;

        clients[number].bytes_read = clients[last].bytes_read;
        clients[number].buffer = clients[last].buffer;
        clients[number].state = clients[last].state;
    }
}
