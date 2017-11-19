#include <sys/socket.h>
#include <arpa/inet.h>
#include <memory.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <zconf.h>
#include "client.h"
#include "../utils/include/sock_utils.h"
#include "../agreements.h"
#include "../protocol.h"
#include "../utils/include/memcpy_next.h"

static const char new_msg[] = NEW_MSG;
static const char more_msg[] = MORE_MSG;
static const char match_msg[] = MATCH_MSG;
static const char ack_msg[] = ACK_MSG;

static const char do_msg[] = DO_MSG;
static const char done_msg[] = DONE_MSG;

int run_client(const char *ip, uint16_t port) {
    local_client_t *client = malloc(sizeof(local_client_t));
    init_local_client(client, ip, port);

    fprintf(stderr, "UUID: ");
    print_uuid(client->uuid, stderr);
    fprintf(stderr, "\n");

    log_simple(client->logger, "Starting client: connecting with server first time");

    contact_res_t result;
    result = try_contact(client, get_new_contact);

    if (result == CON_DONE) {
        log_simple(client->logger, "Answer already have been found");
    }

    while (result == CON_DO) {
        range_values_t *range = client->range;
        fprintf(stderr, "Looking for answer in %s --- %s\n", range->begin_word, range->end_word);
        int match = find_match_in(range);
        if (match == MATCH) {
            log_simple(client->logger, "Answer found!");
            fprintf(stderr, "%s\n", range->word);
            result = try_contact(client, get_match_contact);
        } else {
            log_simple(client->logger, "Answer not found: asking more");
            result = try_contact(client, get_more_contact);
        }
    }

    log_simple(client->logger, "Finishing client");
    destroy_local_client(client);
}


void init_local_client(local_client_t *client, const char *ip, uint16_t port) {
    client->ip = ip;
    client->port = port;

    uuid_generate(client->uuid);
    get_uuid_short(client->uuid_short, client->uuid);

    client->sock_fd = -1;
    client->buffer = malloc(BUF_SIZE);

    client->range = malloc(sizeof(range_values_t));
    init_range(client->range);

    client->logger = malloc(sizeof(logger_t));
    init_logger(client->logger, client->uuid_short, stderr);
}

void destroy_local_client(local_client_t *client) {
    free(client->buffer);

    free_range(client->range);
    free(client->range);

    free(client->logger);

}


contact_res_t try_contact(local_client_t *client, contact_res_t (*contact_fun)(local_client_t *)) {
    contact_res_t result = CON_FAILURE;

    do {
        log_simple(client->logger, "Connecting to server");
        client->sock_fd = client_socket(client->ip, client->port);
        if(client->sock_fd >= 0) {
            log_simple(client->logger, "Contacting with server");

            result = contact_fun(client);
            close(client->sock_fd);
        } else {
            log_simple(client->logger, "Failed to connect to server");
        }
    } while (result == CON_FAILURE && (sleep(3) == 0));

    return result;
}

contact_res_t get_match_contact(local_client_t *client) {
    size_t written = fill_match_buf(client->buffer, client->uuid, client->range);

    int result = send_all(client->sock_fd, client->buffer, 0, written);
    if (result == FAILURE_CODE) {
        return CON_FAILURE;
    }

    size_t offset = 0;
    result = recv_all_next(client->buffer, client->sock_fd, MSG_LEN, &offset);
    if (result == FAILURE_CODE || memcmp(client->buffer, done_msg, MSG_LEN) != 0) {
        return CON_FAILURE;
    }

    send_all(client->sock_fd, ack_msg, 0, MSG_LEN);
    recv(client->sock_fd, client->buffer, 1, 0);

    return CON_DONE;
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

contact_res_t get_new_contact(local_client_t *client) {
    size_t offset = 0;

    fill_conf_buf(client->buffer, new_msg, client->uuid);
    contact_res_t con_result = get_check_msg(client->sock_fd, client->buffer, &offset);
    if (con_result != CON_DO) {
        return con_result;
    }

    int res = get_hash(client->sock_fd, client->buffer, &offset);
    if (res == FAILURE_CODE) {
        return CON_FAILURE;
    }

    memcpy(client->range->hash_to_break, &client->buffer[MSG_LEN], MD5_DIGEST_LENGTH);

    con_result = get_words(client->sock_fd, client->buffer, offset, client->range);
    if (con_result != CON_DO) {
        return con_result;
    }

    send_all(client->sock_fd, ack_msg, 0, MSG_LEN);
    recv(client->sock_fd, client->buffer, 1, 0);

    return con_result;
}

contact_res_t get_more_contact(local_client_t *client) {
    size_t offset = 0;

    fill_conf_buf(client->buffer, more_msg, client->uuid);
    contact_res_t con_result = get_check_msg(client->sock_fd, client->buffer, &offset);
    if (con_result != CON_DO) {
        return con_result;
    }

    con_result = get_words(client->sock_fd, client->buffer, offset, client->range);
    if (con_result != CON_DO) {
        return con_result;
    }

    send_all(client->sock_fd, ack_msg, 0, MSG_LEN);
    recv(client->sock_fd, client->buffer, 1, 0);

    return con_result;
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

int get_hash(int socket_fd, u_char *buffer, size_t *offset) {
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
