#ifndef NET5_SOCK_POLLS_H
#define NET5_SOCK_POLLS_H

void init_sock_polls(struct pollfd **socket_polls, int server_socket_fd);

void destroy_sock_polls(struct pollfd *socket_polls);

#endif //NET5_SOCK_POLLS_H
