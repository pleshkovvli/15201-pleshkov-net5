#include "../include/logger.h"

void init_logger(logger_t *logger, const char *tag, FILE *stream) {
    logger->tag = tag;
    logger->stream = stream;
}

void log_simple(logger_t *logger, const char *message) {
    fprintf(logger->stream, "%s: %s\n", logger->tag, message);
}