#ifndef NET5_SOCK_UTILS_H
#define NET5_SOCK_UTILS_H

#include <stddef.h>
#include <stdint.h>

#define SUCCESS_CODE 0
#define FAILURE_CODE (-1)

int send_all(int sock_fd, const void *buffer, size_t off, size_t len);
int server_socket(uint16_t port);

#endif //NET5_SOCK_UTILS_H
