#ifndef NET5_PRINT_UUID_H
#define NET5_PRINT_UUID_H

#include <zconf.h>
#include <stdio.h>

#define UUID_SHORT_LEN 6

void print_uuid(const u_char *uuid, FILE *stream);
void get_uuid_short(char *uuid_short, const u_char *uuid);
#endif //NET5_PRINT_UUID_H
