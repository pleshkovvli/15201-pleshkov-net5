#ifndef NET5_CLIENT_H
#define NET5_CLIENT_H

#include "uuid/uuid.h"
#include "md5-cracker/md5-cracker.h"
#include "../utils/include/print_uuid.h"
#include "../utils/include/logger.h"

#define BUF_SIZE 64

typedef enum contact_res {CON_DO, CON_DONE, CON_FAILURE} contact_res_t;

typedef struct local_client {
    const char *ip;
    uint16_t port;

    uuid_t uuid;
    char uuid_short[UUID_SHORT_LEN + 1];

    int sock_fd;
    u_char *buffer;

    range_values_t *range;

    logger_t *logger;
} local_client_t;

contact_res_t try_contact(local_client_t *client, contact_res_t (*contact_fun)(local_client_t *));


int run_client(const char *ip, uint16_t port);

void fill_conf_buf(u_char *buffer, const char *msg, const unsigned char *uuid);

contact_res_t get_more_contact(local_client_t *client);

contact_res_t get_new_contact(struct local_client *client);

int get_hash(int socket_fd, u_char *buffer, size_t *offset);

contact_res_t get_check_msg(int socket_fd, u_char *buffer, size_t *offset);

contact_res_t get_words(int socket_fd, u_char *buffer, size_t offset, range_values_t *range);

contact_res_t get_one_word(int socket_fd, u_char *buffer, size_t *offset, char *word);

contact_res_t get_match_contact(local_client_t *client);

size_t fill_match_buf(u_char *buffer, uuid_t uuid, const range_values_t *range);

#endif //NET5_CLIENT_H
