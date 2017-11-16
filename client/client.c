#include <sys/socket.h>
#include <arpa/inet.h>
#include <memory.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <zconf.h>
#include "client.h"
#include "../utils/sock_utils.h"
#include "../agreements.h"
#include "../protocol.h"
#include "../utils/memcpy_next.h"

static const char new_msg[] = NEW_MSG;
static const char more_msg[] = MORE_MSG;
static const char match_msg[] = MATCH_MSG;
static const char ack_msg[] = ACK_MSG;

static const char do_msg[] = DO_MSG;
static const char done_msg[] = DONE_MSG;

contact_res_t get_check_msg(int socket_fd, u_char *buffer, size_t *offset);

contact_res_t get_words(int socket_fd, u_char *buffer, size_t offset, range_values_t *range);

contact_res_t get_one_word(int socket_fd, u_char *buffer, size_t *offset, char *word);

contact_res_t get_match_contact(int sock_fd, u_char *buffer, uuid_t uuid, range_values_t *range);

size_t fill_match_buf(u_char *buffer, uuid_t uuid, const range_values_t *range);

int run_client(const char *ip, uint16_t port) {
    uuid_t uuid = {};
    uuid_generate(uuid);

    int sock_fd;
    u_char buffer[BUF_SIZE];

    range_values_t range;
    init_range(&range);

    contact_res_t result;
    do {
        sock_fd = client_socket(ip, port);
        result = get_new_contact(sock_fd, buffer, uuid, &range);
        close(sock_fd);
    } while (result == CON_FAILURE && (sleep(3) == 0));

    if (result == CON_DONE) {
        exit(EXIT_SUCCESS);
    }

    while (result == CON_DO) {
        int match = find_match_in(&range);
        if (match == MATCH) {
            do {
                sock_fd = client_socket(ip, port);
                result = get_match_contact(sock_fd, buffer, uuid, &range);
                close(sock_fd);
            } while (result == CON_FAILURE && (sleep(3) == 0));
            break;
        }

        do {
            sock_fd = client_socket(ip, port);
            result = get_more_contact(sock_fd, buffer, uuid, &range);
            close(sock_fd);
        } while (result == CON_FAILURE && (sleep(3) == 0));
    }

    if (result == CON_DONE) {
        exit(EXIT_SUCCESS);
    }

    free_range(&range);
}

contact_res_t get_match_contact(int sock_fd, u_char *buffer, uuid_t uuid, range_values_t *range) {
    size_t written = fill_match_buf(buffer, uuid, range);

    int result = send_all(sock_fd, buffer, 0, written);
    if (result == FAILURE_CODE) {
        return CON_FAILURE;
    }

    size_t offset = 0;
    result = recv_all_next(buffer, sock_fd, MSG_LEN, &offset);
    if (result == FAILURE_CODE || memcmp(buffer, done_msg, MSG_LEN) != 0) {
        return CON_FAILURE;
    }

}

size_t fill_match_buf(u_char *buffer, uuid_t uuid, const range_values_t *range) {
    uint16_t match_length = (uint16_t) strlen(range->word);
    uint16_t len_to_send = htons(match_length);

    size_t written = 0;

    memcpy_next(buffer, match_msg, MSG_LEN, &written);
    memcpy_next(buffer, uuid, UUID_LEN, &written);
    memcpy_next(buffer, &len_to_send, sizeof(uint16_t), &written);
    memcpy_next(buffer, range->word, match_length, &written);
    return written;
}

contact_res_t get_new_contact(int socket_fd, u_char *buffer, uuid_t uuid, range_values_t *range) {
    size_t offset = 0;

    fill_conf_buf(buffer, new_msg, uuid);
    contact_res_t con_result = get_check_msg(socket_fd, buffer, &offset);
    if (con_result != CON_DO) {
        return con_result;
    }

    int res = get_hash(socket_fd, buffer, &offset);
    if (res == FAILURE_CODE) {
        return CON_DONE;
    }

    return get_words(socket_fd, buffer, offset, range);
}

contact_res_t get_more_contact(int socket_fd, u_char *buffer, uuid_t uuid, range_values_t *range) {
    size_t offset = 0;

    fill_conf_buf(buffer, more_msg, uuid);
    contact_res_t con_result = get_check_msg(socket_fd, buffer, &offset);
    if (con_result != CON_DO) {
        return con_result;
    }

    return get_words(socket_fd, buffer, offset, range);
}


contact_res_t get_words(int socket_fd, u_char *buffer, size_t offset, range_values_t *range) {
    contact_res_t contact_res = get_one_word(socket_fd, buffer, &offset, range->begin_word);
    if (contact_res != CON_DO) {
        return contact_res;
    }

    contact_res = get_one_word(socket_fd, buffer, &offset, range->end_word);
    if (contact_res != CON_DO) {
        return contact_res;
    }

    return CON_DO;
}

contact_res_t get_one_word(int socket_fd, u_char *buffer, size_t *offset, char *word) {
    uint16_t length_recv;
    uint16_t length;

    int result = recv_all_next(buffer, socket_fd, LEN_LEN, offset);

    if (result == FAILURE_CODE) {
        return CON_FAILURE;
    }

    memcpy(&length_recv, &buffer[(*offset) - LEN_LEN], LEN_LEN);
    length = ntohs(length_recv);

    result = recv_all_next(buffer, socket_fd, length, offset);
    if (result == FAILURE_CODE) {
        return CON_FAILURE;
    }

    memcpy(word, &buffer[(*offset) - length], length);
    word[length] = '\0';

    return CON_DO;
}

contact_res_t get_check_msg(int socket_fd, u_char *buffer, size_t *offset) {
    int result = send_all(socket_fd, buffer, 0, CONF_LEN);
    if (result == FAILURE_CODE) {
        return CON_FAILURE;
    }

    result = recv_all_next(buffer, socket_fd, MSG_LEN, offset);
    if (result == FAILURE_CODE) {
        return CON_FAILURE;
    }

    if (memcmp(buffer, done_msg, MSG_LEN) == 0) {
        send_all(socket_fd, ack_msg, 0, MSG_LEN);
        recv(socket_fd, buffer, 1, 0);
        return CON_DONE;
    }

    if (memcmp(buffer, do_msg, MSG_LEN) == 0) {
        return CON_DO;
    }

    return CON_FAILURE;
}

int get_hash(int socket_fd, const void *buffer, size_t *offset) {
    int result = recv_all_next(buffer, socket_fd, MD5_DIGEST_LENGTH, offset);

    if (result == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}


void fill_conf_buf(u_char *buffer, const char *msg, const unsigned char *uuid) {
    memcpy(buffer, msg, MSG_LEN);
    memcpy(&buffer[MSG_LEN], uuid, UUID_LEN);
}
