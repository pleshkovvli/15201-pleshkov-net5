#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include <arpa/inet.h>
#include "sock_utils.h"
#include "../agreements.h"

int recv_all_next(u_char *buffer, int socket_fd, size_t length, size_t *offset) {
    size_t read_bytes = 0;
    while (read_bytes < length) {
        ssize_t read = recv(socket_fd, &buffer[*offset + read_bytes], length - read_bytes, 0);
        if (read < 0) {
            return FAILURE_CODE;
        }

        read_bytes += read;
    }

    *offset += length;
    return SUCCESS_CODE;
}


int client_socket(const char *ip, uint16_t port) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0) {
        perror("Failed to open socket.");
    }

    struct sockaddr_in server_address;
    socklen_t server_address_size = sizeof(struct sockaddr_in);

    memset(&server_address, 0, server_address_size);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    int result = inet_pton(AF_INET, ip, &server_address.sin_addr);
    if (result != 1) {
        return FAILURE_CODE;
    }

    result = connect(socket_fd, (struct sockaddr *) &server_address, server_address_size);
    if (result != SUCCESS_CODE) {
        return result;
    }

    return socket_fd;
}

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
