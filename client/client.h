#ifndef NET5_CLIENT_H
#define NET5_CLIENT_H

#include "../types.h"
#include "md5-cracker/md5-cracker.h"

#define BUF_SIZE 64

typedef enum contact_res {CON_DO, CON_DONE, CON_FAILURE} contact_res_t;

int run_client(const char *ip, uint16_t port);

void fill_conf_buf(u_char *buffer, const char *msg, const unsigned char *uuid);

contact_res_t get_more_contact(int socket_fd, u_char *buffer, uuid_t uuid, range_values_t *range);

contact_res_t get_new_contact(int socket_fd, u_char *buffer, uuid_t uuid, range_values_t *range);

int get_hash(int socket_fd, const void *buffer, size_t *offset);

#endif //NET5_CLIENT_H
