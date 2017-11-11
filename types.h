#ifndef NET5_TYPES_H
#define NET5_TYPES_H

#include <poll.h>
#include <uuid/uuid.h>

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

typedef struct success {
    int happened;
    int num;
    ushort str_length;
} success_t;

typedef struct client {
    u_char *buffer;
    ushort bytes_read;
    client_state state;
} client_t;

typedef struct task {
    task_status status;
    uuid_t uuid;
    time_t timestamp;
    char *begin_str;
    char *end_str;
} task_t;


typedef struct cur_clients {
    struct pollfd *polls;
    client_t *clients;
    nfds_t amount;
} cur_clients_t;

#endif //NET5_TYPES_H
