#include "success.h"
#include "../../agreements.h"

void init_success(success_t *success, uint max_str_len) {
    success->happened = FALSE;
    success->str_len = 0;
    success->answer = malloc(max_str_len + 1);
}

void destroy_success(success_t *success) {
    free(success->answer);
}

void cancel_success(success_t *success) {
    success->happened = FALSE;
    success->str_len = 0;
}
