#ifndef NET5_CHECK_STRINGS_INNER_H
#define NET5_CHECK_STRINGS_INNER_H

#include <stdio.h>
#include <openssl/md5.h>
#include <string.h>
#include <sys/param.h>

#define INVALID (-1)
#define VALID 1

typedef struct lengths {
    ushort min_length;
    ushort max_length;
    ushort word_length;
    ushort alphabet_length;
} lengths_t;

int find_match_inner(
        range_values_t *range_values,
        lengths_t *lengths,
        ushort *reverse_table,
        ushort index
);

int find_match_in_inner(
        range_values_t *range_values,
        lengths_t *lengths,
        ushort *reverse_table,
        ushort index
);

int validate_and_get_limits(range_values_t *range_values, lengths_t *lengths);

int check_md5(char *word, size_t length, char *hash_to_break);

ushort get_position(
        ushort default_value,
        const char *key_word,
        const char *word,
        ushort word_length,
        ushort index,
        ushort *reverse_table
);

#endif //NET5_CHECK_STRINGS_INNER_H
