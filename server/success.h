#ifndef NET5_SUCCESS_H
#define NET5_SUCCESS_H


#include <zconf.h>
#include <stdlib.h>
#include "../protocol.h"

typedef struct success {
    int happened;
    int num;
    ushort str_len;
    u_char *answer;
} success_t;



void init_success(success_t *success);

void destroy_success(success_t *success);

void cancel_success(success_t *success);


#endif //NET5_SUCCESS_H
