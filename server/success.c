#include "success.h"


void init_success(success_t *success) {
    success->happened = FALSE;
    success->num = -1;
    success->str_len = 0;
    success->answer = malloc(MAX_STR_LEN + 1);
}

void destroy_success(success_t *success) {
    free(success->answer);
}

void cancel_success(success_t *success) {
    success->happened = FALSE;
    success->num = -1;
    success->str_len = 0;
}