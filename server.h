#ifndef NET5_SERVER_H
#define NET5_SERVER_H

#include <sys/socket.h>
#include <stdio.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <memory.h>
#include <poll.h>
#include <uuid/uuid.h>
#include <zconf.h>

#define SUCCESS_CODE 0
#define FAILURE_CODE (-1)
#define MAX_CLIENTS 64

#define TIMEOUT 3
#define TIME_TO_DO 30

#define TRUE 1
#define FALSE 0

#define MSG_LEN 4
#define UUID_LEN 16
#define LEN_LEN 2
#define SUF_LEN 6
#define CONF_LEN (MSG_LEN + UUID_LEN)
#define READ_LEN (CONF_LEN + LEN_LEN)

#define CLIENT_BUF_SIZE 64

typedef enum {
    UNKNOWN,
    NEW,
    MORE,
    TO_ACK,
    SUCCESS
} client_state;

typedef enum {
    IN_PROGRESS, TO_DO
} task_status;

typedef struct client {
    u_char *buffer;
    short bytes_read;
    client_state state;
} client_t;

typedef struct task {
    uuid_t uuid;
    time_t timestamp;
    char begin[32];
    char end[32];
    task_status status;
} task_t;

typedef struct task_list {
    task_t *tasks;
    int amount;
} task_list_t;

typedef struct string_gen {
    char *current_string;
    const char *begin_string_suf;
    const char *end_string_suf;
    const char *alphabet;
    int alphabet_length;
    ushort *reverse_table;
} string_gen_t;

typedef struct tasks_srt {
    task_list_t tasks_list;
    string_gen_t str_gen;
    const char *hash;
} tasks_srt_t;

typedef struct poll_clients {
    struct pollfd *polls;
    client_t *clients;
    nfds_t amount;
} poll_clients_t;

void get_next(string_gen_t *str_gen);

enum next_action next_string(string_gen_t *str_gen, ushort length, int index);

size_t fill_buffer(u_char *buffer, task_t *task);


void add_client(poll_clients_t *poll_clients, int socket_fd) {
    nfds_t client_num = poll_clients->amount;
    client_t *client = &(poll_clients->clients[client_num]);
    struct pollfd *poll = &(poll_clients->polls[client_num]);

    client->state = UNKNOWN;
    client->bytes_read = 0;
    client->buffer = malloc(CLIENT_BUF_SIZE);

    poll->fd = socket_fd;
    poll->events = POLLIN;
    poll->revents = 0;
}


void fill_task(task_t *task, string_gen_t *str_gen);

void remove_client(int number, poll_clients_t *poll_clients);

int handle_to_ack(void *buffer);

client_state get_status(void *buffer);

int handle_new(tasks_srt_t *tasks_str, int socket_fd, client_t *cur_client);

int handle_more(tasks_srt_t *tasks_str, int socket_fd, client_t *cur_client);

int handle_success(const task_list_t *tasks, client_t *cur_client);

int check_uuid(const task_list_t *tasks, client_t *client);

enum next_action change_sym(string_gen_t *str_gen, int index);

int init_string_gen(string_gen_t *str_gen);

int server_socket(uint16_t port);

void init_poll_clients(poll_clients_t *poll_clients, struct pollfd *socket_polls);


int send_work(size_t msg_size, int socket_fd, client_t *cur_client);

task_t *get_task(tasks_srt_t *tasks_str, void *client_uuid);

#endif //NET5_SERVER_H
