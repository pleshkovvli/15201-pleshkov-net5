#include <stdio.h>
#include "print_uuid.h"
#include "../agreements.h"

void print_uuid(const u_char *uuid) {
    for(int i = 0; i < UUID_SIZE; ++i) {
        printf("%02x", uuid[i]);
    }
}

void print_uuid_short(const u_char *uuid) {
    for(int i = 0; i < 3; ++i) {
        printf("%02x", uuid[i]);
    }
}