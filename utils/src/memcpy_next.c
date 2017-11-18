#include "../include/memcpy_next.h"

void memcpy_next(void *buffer, const void *src, size_t n, size_t *written) {
    memcpy(&(((char*) buffer)[*written]), src, n);
    *written += n;
}
