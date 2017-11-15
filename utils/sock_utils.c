#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include "sock_utils.h"

int send_all(int sock_fd, const void *buffer, size_t off, size_t len) {
    ssize_t bytes_snd = 0;
    while (bytes_snd < len) {
        ssize_t snd = send(sock_fd, &((char *)buffer)[off + bytes_snd], len - bytes_snd, 0);
        if (snd < 0) {
            return FAILURE_CODE;
        }

        bytes_snd += snd;
    }

    return SUCCESS_CODE;
}

int server_socket(uint16_t port) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0) {
        perror("Open socket error");
        return FAILURE_CODE;
    }

    struct sockaddr_in address;
    socklen_t addr_size = sizeof(struct sockaddr_in);

    memset(&address, 0, addr_size);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    int result_code = bind(socket_fd, (struct sockaddr *) &address, addr_size);

    if (result_code != SUCCESS_CODE) {
        perror("Bind socket error");
        return FAILURE_CODE;
    }

    result_code = listen(socket_fd, 32);

    if (result_code != SUCCESS_CODE) {
        perror("Listen socket error");
        return FAILURE_CODE;
    }

    return socket_fd;
}
