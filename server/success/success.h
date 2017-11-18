#ifndef NET5_SUCCESS_H
#define NET5_SUCCESS_H

#include <zconf.h>
#include <stdlib.h>

typedef struct success {
    int happened;
    ushort str_len;
    u_char *answer;
} success_t;

void init_success(success_t *success, uint max_str_len);

void destroy_success(success_t *success);

void cancel_success(success_t *success);


#endif //NET5_SUCCESS_H
