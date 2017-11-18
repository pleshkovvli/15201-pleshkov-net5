#ifndef NET5_SOCK_POLLS_H
#define NET5_SOCK_POLLS_H

#include <poll.h>

void init_sock_polls(struct pollfd **socket_polls, int server_socket_fd, uint max_clients);

void destroy_sock_polls(struct pollfd *socket_polls);

#endif //NET5_SOCK_POLLS_H
