#include <stdio.h>
#include "../include/print_uuid.h"
#include "../../agreements.h"

void print_uuid(const u_char *uuid, FILE* stream) {
    for(int i = 0; i < UUID_SIZE; ++i) {
        fprintf(stream, "%02x", uuid[i]);
    }
}

void get_uuid_short(char *uuid_short, const u_char *uuid) {
    for(int i = 0; i < UUID_SHORT_LEN / 2; ++i) {
        snprintf(&uuid_short[i * 2], 3, "%02x", uuid[i]);
    }
    uuid_short[UUID_SHORT_LEN] = '\0';
}