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

static const char new_msg[] = "_NEW";
static const char more_msg[] = "MORE";
static const char match_msg[] = "MTCH";
static const char ack_msg[] = "_ACK";

int client_socket(const char *ip, uint16_t port);

int send_all(int socket_fd, const void *buffer, size_t offset, size_t length);

void fill_conf_buf(u_char *buffer, const char *msg, const unsigned char *uuid);

int recv_next(u_char *buffer, int socket_fd, size_t length, size_t *offset);

int get_contact(int new, int socket_fd, void *buffer, const unsigned char *uuid, uint16_t *length);

int new_contact(int socket_fd, const void *buffer, size_t *offset);

void init_range(range_values_t *range);

void free_range(range_values_t *range);

int run_client(const char *ip, uint16_t port) {
    uuid_t uuid = {};
    uuid_generate(uuid);

    char hash[MD5_DIGEST_LENGTH];

    u_char buffer[CLIENT_BUF_SIZE];
    uint16_t length;

    int socket_fd;

    while (1) {
        socket_fd = client_socket(ip, port);

        int result = get_contact(TRUE, socket_fd, buffer, uuid, &length);
        close(socket_fd);
        if (result == SUCCESS_CODE) {
            break;
        }

        sleep(3);
    }


    range_values_t range;
    init_range(&range);

    memcpy(range.hash_to_break, buffer, MD5_DIGEST_LENGTH);
    memcpy(range.begin_word, &buffer[LEN_LEN + MD5_DIGEST_LENGTH], length);
    memcpy(range.end_word, &buffer[LEN_LEN + MD5_DIGEST_LENGTH + length], length);


    while (1) {
        int match = find_match_in(&range);
        if (match == MATCH) {

            uint16_t match_length = (uint16_t) strlen(range.word);

            match_length = htons(match_length);

            memcpy(buffer, match_msg, MSG_LEN);
            memcpy(&buffer[MSG_LEN], &match_length, sizeof(uint16_t));

            memcpy(&buffer[MSG_LEN + sizeof(uint16_t)], range.word, match_length);

            send_all(socket_fd, buffer, 0, MSG_LEN + sizeof(uint16_t) + match_length);

            recv(socket_fd, buffer, MSG_LEN, 0);

            break;
        }

        memcpy(buffer, more_msg, MSG_LEN);
        memcpy(&buffer[MSG_LEN], uuid, UUID_LEN);

        while (1) {
            socket_fd = client_socket(ip, port);

            int result = get_contact(FALSE, socket_fd, buffer, uuid, &length);
            close(socket_fd);
            if (result == SUCCESS_CODE) {
                break;
            }

            sleep(3);
        }


        memcpy(range.begin_word, &buffer[LEN_LEN], length);
        memcpy(range.end_word, &buffer[LEN_LEN + length], length);
    }

    free_range(&range);
}

void init_range(range_values_t *range) {
    range->alphabet = "ACGT";
    range->word = malloc(32);
    range->hash_to_break = malloc(MD5_DIGEST_LENGTH);
    range->begin_word = malloc(32);
    range->end_word = malloc(32);
}

void free_range(range_values_t *range) {
    free(range->word);
    free(range->hash_to_break);
    free(range->begin_word);
    free(range->end_word);
}

int get_contact(int new, int socket_fd, void *buffer, const unsigned char *uuid, uint16_t *length) {
    (*length) = 0;
    fill_conf_buf(buffer, new ? new_msg : more_msg, uuid);
    size_t offset = 0;

    int result = send_all(socket_fd, buffer, 0, CONF_LEN);
    if (result == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    result = recv_next(buffer, socket_fd, MSG_LEN, &offset);
    if (result == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    if (memcmp(buffer, DONE_MSG, MSG_LEN) == 0) {
        return FAILURE_CODE;
    }

    if (new) {
        result = new_contact(socket_fd, buffer, &offset);

        if (result == FAILURE_CODE) {
            return FAILURE_CODE;
        }
    }

    result = recv_next(buffer, socket_fd, LEN_LEN, &offset);

    if (result == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    memcpy(length, buffer,
           LEN_LEN);
    (*length) =
            ntohs((*length));

    result = recv_next(buffer, socket_fd, (*length) * (uint) 2, &offset);

    if (result == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    result = send_all(socket_fd, ack_msg, MSG_LEN, 0);

    if (result == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    char dum;

    ssize_t chk = recv(socket_fd, &dum, 1, 0);

    if (chk >= 0) {
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int new_contact(int socket_fd, const void *buffer, size_t *offset) {
    int result = recv_next(buffer, socket_fd, MD5_DIGEST_LENGTH, offset);
    if (result == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int recv_next(u_char *buffer, int socket_fd, size_t length, size_t *offset) {
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

void fill_conf_buf(u_char *buffer, const char *msg, const unsigned char *uuid) {
    memcpy(buffer, msg, MSG_LEN);
    memcpy(&buffer[MSG_LEN], uuid, UUID_LEN);
}

int send_all(int socket_fd, const void *buffer, size_t offset, size_t length) {
    ssize_t bytes_snd = 0;
    while (bytes_snd < length) {
        ssize_t snd = send(socket_fd, &buffer[offset + bytes_snd], length - bytes_snd, 0);
        if (snd < 0) {
            return FAILURE_CODE;
        }

        bytes_snd += snd;
    }

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