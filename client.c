#include <bits/socket_type.h>
#include <bits/socket.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <memory.h>
#include <arpa/inet.h>
#include "client.h"
#include "server.h"
#include "check_strings.h"

static char register_msg[] = "_NEW";
static char more_msg[] = "MORE";
static char match_msg[] = "MTCH";
static char ack_msg[] = "_ACK";

int client_socket(const char *ip, uint16_t port);

int run_client(const char *ip, uint16_t port) {
    uuid_t uuid = {};
    uuid_generate(uuid);

    int socket_fd = client_socket(ip, port);

    u_char buffer[32];

    memcpy(buffer, register_msg, MSG_LEN);
    memcpy(&buffer[MSG_LEN], uuid, UUID_LEN);

    ssize_t n = send(socket_fd, buffer, CONF_LEN, 0);

    if(n < CONF_LEN) {
        exit(EXIT_FAILURE);
    }

    ssize_t read = recv(socket_fd, buffer, MD5_DIGEST_LENGTH, 0);

    if(read < MD5_DIGEST_LENGTH) {
        exit(EXIT_FAILURE);
    }

    read = recv(socket_fd, &buffer[MD5_DIGEST_LENGTH], LEN_LEN, 0);

    if(read < LEN_LEN) {
        exit(EXIT_FAILURE);
    }

    uint16_t length = 0;
    memcpy(&length, buffer, LEN_LEN);
    length = ntohs(length);

    read = recv(socket_fd, &buffer[LEN_LEN + MD5_DIGEST_LENGTH], length * (uint) 2, 0);

    if(read < (length * (uint) 2)) {
        exit(EXIT_FAILURE);
    }

    send(socket_fd, ack_msg, MSG_LEN, 0);


    range_values_t range;
    range.alphabet = "ACGT";
    range.word = malloc(32);
    range.hash_to_break = malloc(MD5_DIGEST_LENGTH);
    range.begin_word = malloc(length);
    range.end_word = malloc(length);

    memcpy(range.hash_to_break, buffer, MD5_DIGEST_LENGTH);

    memcpy(range.begin_word, &buffer[LEN_LEN + MD5_DIGEST_LENGTH], length);

    memcpy(range.end_word, &buffer[LEN_LEN + MD5_DIGEST_LENGTH + length], length);


    while(1) {
        int match = find_match_in(&range);
        if(match == MATCH) {
            break;
        }

        memcpy(buffer, more_msg, MSG_LEN);
        memcpy(&buffer[MSG_LEN], uuid, UUID_LEN);

        n = send(socket_fd, buffer, CONF_LEN, 0);

        if(n < CONF_LEN) {
            exit(EXIT_FAILURE);
        }

        read = recv(socket_fd, buffer, LEN_LEN, 0);

        if(read < LEN_LEN) {
            exit(EXIT_FAILURE);
        }

        length = 0;
        memcpy(&length, buffer, LEN_LEN);
        length = ntohs(length);

        read = recv(socket_fd, &buffer[LEN_LEN], length * (uint) 2, 0);

        if(read < (length * (uint) 2)) {
            exit(EXIT_FAILURE);
        }

        send(socket_fd, ack_msg, MSG_LEN, 0);


        memcpy(range.begin_word, &buffer[LEN_LEN], length);

        memcpy(range.end_word, &buffer[LEN_LEN + length], length);

    }

    uint16_t match_length = (uint16_t) strlen(range.word);

    match_length = htons(match_length);

    memcpy(buffer, match_msg, MSG_LEN);
    memcpy(&buffer[MSG_LEN], &match_length, sizeof(uint16_t));

    memcpy(&buffer[MSG_LEN + sizeof(uint16_t)], range.word, match_length);

    send(socket_fd, buffer, MSG_LEN + sizeof(uint16_t) + match_length, 0);

    recv(socket_fd, buffer, MSG_LEN, 0);


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
    if(result != 1) {
        return FAILURE_CODE;
    }

    result = connect(socket_fd, (struct sockaddr *) &server_address, server_address_size);
    if(result != SUCCESS_CODE) {
        return result;
    }

    return socket_fd;
}