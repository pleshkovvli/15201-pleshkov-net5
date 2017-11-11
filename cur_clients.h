#ifndef NET5_SERVER_HANDLING_H
#define NET5_SERVER_HANDLING_H

#include "types.h"

#define MAX_CLIENTS 64
#define CLIENT_BUF_SIZE 64

void init_cur_clients(cur_clients_t *cur_clients, struct pollfd *polls);
void destroy_cur_clients(cur_clients_t *cur_clients);
void add_client(cur_clients_t *cur_clients, int socket_fd);
void remove_client(cur_clients_t *cur_clients, int number);

#endif //NET5_SERVER_HANDLING_H
