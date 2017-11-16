#ifndef NET5_SERVER_HANDLING_H
#define NET5_SERVER_HANDLING_H

#include "../../types.h"
#include "client_type.h"

typedef struct cur_clients {
    struct pollfd *polls;
    client_t *clients;
    nfds_t amount;
    uint max_clients;
} cur_clients_t;


void init_cur_clients(cur_clients_t *cur_clients, struct pollfd *polls, uint max_clients);
void destroy_cur_clients(cur_clients_t *cur_clients);
void add_client(cur_clients_t *cur_clients, int socket_fd);
void remove_client(cur_clients_t *cur_clients, int number);

#endif //NET5_SERVER_HANDLING_H
