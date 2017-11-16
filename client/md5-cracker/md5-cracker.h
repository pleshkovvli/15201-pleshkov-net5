#ifndef NET5_CHECK_STRINGS_H
#define NET5_CHECK_STRINGS_H

#define MATCH 1
#define MISMATCH 0
#define INVALID_ARGS (-1)

typedef struct range_values_struct {
    char *begin_word;
    char *end_word;
    char *word;
    char *alphabet;
    char *hash_to_break;
} range_values_t;

int find_match_in(range_values_t *range_values);

void init_range(range_values_t *range);

void free_range(range_values_t *range);


#endif //NET5_CHECK_STRINGS_H
