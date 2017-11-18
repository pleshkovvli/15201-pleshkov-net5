#ifndef NET5_MEMCPY_NEXT_H
#define NET5_MEMCPY_NEXT_H

#include <stddef.h>
#include <memory.h>

void memcpy_next(void *buffer, const void *src, size_t n, size_t *written);

#endif //NET5_MEMCPY_NEXT_H
