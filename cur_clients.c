#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include "cur_clients.h"

void init_cur_clients(cur_clients_t *cur_clients, struct pollfd *polls) {
    cur_clients->clients = malloc(sizeof(client_t) * MAX_CLIENTS);
    cur_clients->polls = &polls[1];
}

void destroy_cur_clients(cur_clients_t *cur_clients) {
    free(cur_clients->clients);
}

void add_client(cur_clients_t *cur_clients, int socket_fd) {
    nfds_t amount = cur_clients->amount;
    client_t *new_client = &(cur_clients->clients[amount]);
    struct pollfd *poll = &(cur_clients->polls[amount]);

    new_client->state = UNKNOWN;
    new_client->bytes_read = 0;
    new_client->buffer = malloc(CLIENT_BUF_SIZE);

    poll->fd = socket_fd;
    poll->events = POLLIN;
    poll->revents = 0;
}

void remove_client(cur_clients_t *cur_clients, int number) {
    --(cur_clients->amount);
    nfds_t last = cur_clients->amount;

    struct pollfd *polls = cur_clients->polls;
    client_t *clients = cur_clients->clients;

    int socket_fd = polls[number].fd;
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);

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
