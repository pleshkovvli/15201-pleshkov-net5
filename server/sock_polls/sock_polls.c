#include <stdlib.h>
#include "sock_polls.h"

void init_sock_polls(struct pollfd **socket_polls, int server_socket_fd, uint max_clients) {
    *socket_polls = malloc(sizeof(struct pollfd) * (max_clients + 1));

    (*socket_polls)[0].fd = server_socket_fd;
    (*socket_polls)[0].events = POLLIN;
    (*socket_polls)[0].revents = 0;
}

void destroy_sock_polls(struct pollfd *socket_polls) {
    free(socket_polls);
}