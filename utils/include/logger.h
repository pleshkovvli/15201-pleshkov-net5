#ifndef PROJECT_LOGGER_H
#define PROJECT_LOGGER_H

#include <stdio.h>

typedef struct logger {
    FILE * stream;
    const char *tag;
} logger_t;

void init_logger(logger_t *logger, const char *tag, FILE *stream);

void log_simple(logger_t *logger, const char *message);

#endif //PROJECT_LOGGER_H
