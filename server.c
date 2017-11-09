#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <memory.h>
#include <poll.h>
#include "server.h"
#include <uuid/uuid.h>
#include <zconf.h>

#define SUCCESS_CODE 0
#define MAX_CLIENTS 64
static const uint16_t PORT = 3112;

#define TRUE 1
#define FALSE 0

#define MSG_LEN 4
#define UUID_LEN 16
#define LEN_LEN 2
#define CONF_LEN (MSG_LEN + UUID_LEN)
#define READ_LEN (CONF_LEN + LEN_LEN)

static char register_msg[] = "RGST";
static char more_msg[] = "MORE";
static char success_msg[] = "SCSS";
static char ack_msg[] = "_ACK";

static uint64_t DIAPASONE = 3000;

typedef enum {
    UNKNOWN,
    REGISTERED,
    MORE,
    SUCCESS,
    TO_ACK,
    SUCCESS_CONF,
    SUCCESS_READ
} client_state;

typedef enum {
    IN_PROGRESS, TO_DO
} task_status;

typedef struct client {
    u_char buffer[128];
    short bytes_read;
    client_state state;
} client_t;

typedef struct task {
    uuid_t uuid;
    char begin[32];
    char end[32];
    task_status status;
} task_t;

int next(char *string,
         const char *alphabet,
         int alphabet_length,
         const ushort *reverse_table
);

int next_string(
        char *string,
        ushort length,
        int index,
        const char *alphabet,
        int alphabet_length,
        const ushort *reverse_table
);

void fill_buffer(
        u_char *buffer,
        char *begin_task,
        char *end_task,
        const char *begin_string_suf,
        const char *end_string_suf,
        const char *current_string,
        size_t length
);


void init_client(client_t *client, struct pollfd *client_poll, int socket_fd) {
    client->state = UNKNOWN;
    client->bytes_read = 0;
    client_poll->fd = socket_fd;
    client_poll->events = POLLIN;
}

void remove_socket(
        nfds_t *number_of_sockets,
        int number,
        struct pollfd *socket_polls
);

void handle_to_ack(
        struct pollfd *socket_polls,
        const client_t *cur_client,
        nfds_t *number_of_sockets,
        int sock_num
);

void handle_unknown(
        struct pollfd *socket_polls,
        client_t *cur_client,
        nfds_t *number_of_sockets,
        int sock_num,
        int *success,
        int *success_num
);

void handle_registred(
        task_t *tasks,
        int *tasks_number,
        const char *begin_string_suf,
        const char *end_string_suf,
        const char *alphabet,
        ushort alphabet_length,
        char *current_string,
        const ushort *reverse_table,
        const struct pollfd *cur_poll,
        client_t *cur_client
);

void handle_more(
        const client_t *clients,
        const task_t *tasks,
        int tasks_number,
        struct pollfd *socket_polls,
        nfds_t *number_of_sockets,
        const char *begin_string_suf,
        const char *end_string_suf,
        const char *alphabet,
        ushort alphabet_length,
        char *current_string,
        const ushort *reverse_table,
        int sock_num,
        struct pollfd *cur_poll,
        client_t *cur_client
);

void handle_success(
        const client_t *clients,
        const task_t *tasks,
        int tasks_number,
        struct pollfd *socket_polls,
        nfds_t *number_of_sockets,
        int sock_num,
        struct pollfd *cur_poll,
        client_t *cur_client
);

int run_server() {
    int server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0) {
        perror("Failed to open socket.");
        exit(1);
    }

    struct sockaddr_in server_address;

    socklen_t address_size = sizeof(struct sockaddr_in);
    memset(&server_address, 0, address_size);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);

    const struct sockaddr *address = (struct sockaddr *) &server_address;
    int result_code = bind(server_socket_fd, address, address_size);
    if (result_code != SUCCESS_CODE) {
        perror("Failed to bind socket.");
        exit(1);
    }

    result_code = listen(server_socket_fd, 32);
    if (result_code != SUCCESS_CODE) {
        perror("Failed to make socket listening");
        exit(1);
    }

    client_t clients[MAX_CLIENTS];
    task_t tasks[MAX_CLIENTS];
    int tasks_number = 0;

    struct pollfd socket_polls[MAX_CLIENTS + 1];
    socket_polls[0].fd = server_socket_fd;
    socket_polls[0].events = POLLIN;

    nfds_t number_of_sockets = 1;

    char *begin_string_suf = "AAAAAA";
    char *end_string_suf = "TTTTTT";


    char alphabet[] = "ACGT";
    ushort alphabet_length = (ushort) strlen(alphabet);
    char current_string[32] = "A";

    ushort reverse_table[256];

    for (ushort i = 0; i < alphabet_length; ++i) {
        reverse_table[(u_char) alphabet[i]] = i;
    }

    int success = FALSE;
    int success_num = 0;


    ushort success_len = 0;

    while (1) {
        int events = poll(socket_polls, number_of_sockets, -1);

        if (success && (socket_polls[success_num].revents == POLLIN)) {
            client_t successor = clients[success_num];
            if ((successor.state == SUCCESS) && (successor.bytes_read >= CONF_LEN)) {
                int uuid_num = 0;
                for (; uuid_num < tasks_number; ++uuid_num) {
                    if (memcmp(
                            tasks[uuid_num].uuid,
                            &clients[uuid_num].buffer[MSG_LEN],
                            UUID_LEN
                    ) == 0) {
                        break;
                    }
                }
                if (uuid_num == tasks_number) {
                    success = FALSE;
                    remove_socket(&number_of_sockets, success_num, socket_polls);
                } else {
                    successor.state = SUCCESS_CONF;
                }
            }
            if ((successor.state == SUCCESS_CONF) && (successor.bytes_read >= READ_LEN)) {
                u_char *buffer = successor.buffer;
                uint16_t s_len = (((uint16_t) buffer[CONF_LEN]) << 8) + buffer[CONF_LEN + 1];
                success_len = ntohs(s_len);
                successor.state = SUCCESS_READ;
            }

            int all_read = READ_LEN + success_len;
            if ((successor.state == SUCCESS_READ) && (successor.bytes_read >= all_read)) {
                successor.buffer[all_read] = 0;
                printf("%s\n", &successor.buffer[READ_LEN]);
                exit(0);
            }
        }

        for (int sock_num = 0; sock_num < number_of_sockets && events > 0; ++sock_num) {
            struct pollfd *cur_poll = &socket_polls[sock_num];
            if (cur_poll->events == 0) {
                continue;
            }

            --events;
            if (cur_poll->events != POLLIN) {
                remove_socket(&number_of_sockets, sock_num, socket_polls);
                continue;
            }

            if (sock_num == 0) {
                int client_fd = accept(server_socket_fd, NULL, NULL);
                init_client(
                        &clients[number_of_sockets],
                        &socket_polls[number_of_sockets],
                        client_fd
                );
                ++number_of_sockets;
                continue;
            }

            client_t *cur_client = &clients[sock_num];

            ssize_t read = recv(cur_poll->fd, cur_client->buffer, CONF_LEN, 0);
            if (read < 0) {
                remove_socket(&number_of_sockets, sock_num, socket_polls);
                continue;
            }

            cur_client->bytes_read += read;

            switch (cur_client->state) {
                case UNKNOWN: {
                    handle_unknown(
                            socket_polls,
                            cur_client,
                            &number_of_sockets,
                            sock_num,
                            &success,
                            &success_num
                    );
                }
                case REGISTERED: {
                    handle_registred(
                            tasks,
                            &tasks_number,
                            begin_string_suf,
                            end_string_suf,
                            alphabet,
                            alphabet_length,
                            current_string,
                            reverse_table,
                            cur_poll,
                            cur_client
                    );
                }
                    break;
                case MORE: {
                    handle_more(
                            clients,
                            tasks,
                            tasks_number,
                            socket_polls,
                            &number_of_sockets,
                            begin_string_suf,
                            end_string_suf,
                            alphabet,
                            alphabet_length,
                            current_string,
                            reverse_table,
                            sock_num,
                            cur_poll,
                            cur_client

                    );
                }
                    break;
                case SUCCESS: {
                    handle_success(
                            clients,
                            tasks,
                            tasks_number,
                            socket_polls,
                            &number_of_sockets,
                            sock_num,
                            cur_poll,
                            cur_client
                    );
                }
                    break;
                case TO_ACK: {
                    handle_to_ack(socket_polls, clients, &number_of_sockets, sock_num);
                }
                    break;
                case SUCCESS_CONF: {
                }
                    break;
                case SUCCESS_READ: {
                }
                    break;
            }
        }
    }
}

void handle_success(
        const client_t *clients,
        const task_t *tasks,
        int tasks_number,
        struct pollfd *socket_polls,
        nfds_t *number_of_sockets,
        int sock_num,
        struct pollfd *cur_poll,
        client_t *cur_client
) {
    if ((cur_client->bytes_read < CONF_LEN)) {
        return;
    }

    int j = 0;
    for (; j < tasks_number; ++j) {
        if (memcmp(tasks[j].uuid, &clients[j].buffer[MSG_LEN], UUID_LEN) == 0) {
            break;
        }
    }
    if (j == tasks_number) {
        fprintf(stderr, "Unknown message type");
        remove_socket(number_of_sockets, sock_num, socket_polls);
    } else {
        cur_client->state = SUCCESS_CONF;
    }
}

void handle_more(
        const client_t *clients,
        const task_t *tasks,
        int tasks_number,
        struct pollfd *socket_polls,
        nfds_t *number_of_sockets,
        const char *begin_string_suf,
        const char *end_string_suf,
        const char *alphabet,
        ushort alphabet_length,
        char *current_string,
        const ushort *reverse_table,
        int sock_num,
        struct pollfd *cur_poll,
        client_t *cur_client
) {
    if ((cur_client->bytes_read < CONF_LEN)) {
        return;
    }

    int j = 0;
    for (; j < tasks_number; ++j) {
        if (memcmp(tasks[j].uuid, &clients[j].buffer[MSG_LEN], UUID_LEN) == 0) {
            break;
        }
    }

    if (j == tasks_number) {
        fprintf(stderr, "Unknown message type");
        remove_socket(number_of_sockets, sock_num, socket_polls);
        return;
    }

    next(current_string, alphabet, alphabet_length, reverse_table);

    size_t length = strlen(current_string);
    fill_buffer(
            cur_client->buffer,
            tasks[tasks_number].begin,
            tasks[tasks_number].end,
            begin_string_suf,
            end_string_suf,
            current_string,
            length
    );
    send(cur_poll->fd, cur_client->buffer, 2 * length + 12, 0);
    cur_client->state = TO_ACK;
    cur_client->bytes_read = 0;
}

void handle_registred(
        task_t *tasks,
        int *tasks_number,
        const char *begin_string_suf,
        const char *end_string_suf,
        const char *alphabet,
        ushort alphabet_length,
        char *current_string,
        const ushort *reverse_table,
        const struct pollfd *cur_poll,
        client_t *cur_client
) {
    if ((cur_client->bytes_read < CONF_LEN)) {
        return;
    }

    const int t_number = *tasks_number;

    memcpy(tasks[t_number].uuid, &(cur_client->buffer[MSG_LEN]), UUID_LEN);
    next(current_string, alphabet, alphabet_length, reverse_table);
    size_t length = strlen(current_string);

    fill_buffer(
            cur_client->buffer,
            tasks[t_number].begin,
            tasks[t_number].end,
            begin_string_suf,
            end_string_suf,
            current_string,
            length
    );
    tasks[t_number].status = IN_PROGRESS;

    ++(*tasks_number);

    send(cur_poll->fd, cur_client->buffer, 2 * length + 12, 0);
    cur_client->state = TO_ACK;
    cur_client->bytes_read = 0;
}

void handle_unknown(
        struct pollfd *socket_polls,
        client_t *cur_client,
        nfds_t *number_of_sockets,
        int sock_num,
        int *success,
        int *success_num
) {
    if (cur_client->bytes_read < MSG_LEN) {
        return;
    }

    if (memcpy(cur_client->buffer, register_msg, MSG_LEN) == 0) {
        cur_client->state = REGISTERED;
        return;
    }

    if (memcmp(cur_client->buffer, more_msg, MSG_LEN) == 0) {
        cur_client->state = MORE;
        return;
    }

    if (memcmp(cur_client->buffer, success_msg, MSG_LEN) == 0) {
        cur_client->state = SUCCESS;
        (*success) = TRUE;
        (*success_num) = sock_num;
        return;
    }

    fprintf(stderr, "Unknown message type");
    remove_socket(number_of_sockets, sock_num, socket_polls);
}

void handle_to_ack(
        struct pollfd *socket_polls,
        const client_t *cur_client,
        nfds_t *number_of_sockets,
        int sock_num
) {
    if (cur_client->bytes_read >= MSG_LEN) {
        return;
    }
    if (memcmp(cur_client->buffer, ack_msg, MSG_LEN) == 0) {
        fprintf(stderr, "Ack accepted");
    } else {
        fprintf(stderr, "Ack failed");
    }
    remove_socket(number_of_sockets, sock_num, socket_polls);

}

void fill_buffer(
        u_char *buffer,
        char *begin_task,
        char *end_task,
        const char *begin_string_suf,
        const char *end_string_suf,
        const char *current_string,
        size_t length
) {
    uint16_t len = htons((uint16_t) length);
    buffer[0] = (u_char) (len >> 16 & 0xFF);
    buffer[1] = (u_char) (len & 0xFF);
    memcpy(&buffer[2], current_string, length);
    memcpy(&buffer[length + 2], begin_string_suf, 6);
    memcpy(&buffer[length + 6 + 2], current_string, length);
    memcpy(&buffer[length + 6 + 2 + length], end_string_suf, 6);
    memcpy(begin_task, &buffer[2], length + 6);
    memcpy(end_task, &buffer[length + 6 + 2], length + 6);
}

int next(char *string,
         const char *alphabet,
         int alphabet_length,
         const ushort *reverse_table
) {
    int i = 0;
    ushort length = (ushort) strlen(string);
    for (; i < length; ++i) {
        if (string[i] != alphabet[alphabet_length - 1]) {
            break;
        }
    }
    if (i == length) {
        for (; i <= length; ++i) {
            string[i] = alphabet[0];
        }
        string[length + 1] = '\0';
    } else {
        next_string(string, length, 0, alphabet, alphabet_length, reverse_table);
    }
}

int next_string(
        char *string,
        ushort length,
        int index,
        const char *alphabet,
        int alphabet_length,
        const ushort *reverse_table
) {
    if (index == length - 1) {
        if (string[index] == alphabet[alphabet_length - 1]) {
            string[index] = alphabet[0];
            return 1;
        } else {
            string[index] = alphabet[reverse_table[string[index]] + 1];
            return 0;
        }
    } else {
        int result = next_string(
                string,
                length,
                index + 1,
                alphabet,
                alphabet_length,
                reverse_table
        );

        if (result == 1) {
            if (string[index] == alphabet[alphabet_length - 1]) {
                string[index] = alphabet[0];
                return 1;
            } else {
                string[index] = alphabet[reverse_table[string[index]] + 1];
                return 0;
            }
        } else {
            return 0;
        }
    }

}

void remove_socket(
        nfds_t *number_of_sockets,
        int number,
        struct pollfd *socket_polls
) {
    --(*number_of_sockets);
    nfds_t last = *number_of_sockets;
    shutdown(socket_polls[number].fd, SHUT_RDWR);
    close(socket_polls[number].fd);
    if (number < last) {
        socket_polls[number].fd = socket_polls[last].fd;
        socket_polls[number].events = socket_polls[last].events;
        socket_polls[number].revents = socket_polls[last].revents;
    }
}
