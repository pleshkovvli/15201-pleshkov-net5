#ifndef NET5_CLIENT_TYPE_H
#define NET5_CLIENT_TYPE_H

#include <zconf.h>
#include <uuid/uuid.h>

#define CLIENT_BUF_SIZE 64

typedef enum {
    UNKNOWN,
    NEW,
    MORE,
    TO_ACK,
    SUCCESS
} client_state;

typedef struct client {
    u_char *buffer;
    uuid_t uuid;
    int got_uuid;
    ushort bytes_read;
    client_state state;
} client_t;

#endif //NET5_CLIENT_TYPE_H
