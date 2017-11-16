#ifndef NET5_SOCK_UTILS_H
#define NET5_SOCK_UTILS_H

#include <stddef.h>
#include <stdint.h>

int server_socket(uint16_t port);
int client_socket(const char *ip, uint16_t port);

int recv_all_next(u_char *buffer, int socket_fd, size_t length, size_t *offset);
int send_all(int sock_fd, const void *buffer, size_t off, size_t len);

#endif //NET5_SOCK_UTILS_H
