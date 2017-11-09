#include <stdio.h>
#include <openssl/md5.h>
#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include "check_strings.h"

static const uint MAX_LENGTH = 32;

void print_hash(u_char *hash);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./net5 STRING");
        return 0;
    }


    range_values_t values;
    values.alphabet = "ACGT";
    values.begin_word = "A";
    values.end_word = "TTTTTTTTTTTTTTT";
    values.hash_to_break = malloc(sizeof(char) * MD5_DIGEST_LENGTH);
    values.word = malloc(sizeof(char) * (MAX_LENGTH + 1));

    MD5((u_char *) (argv[1]), strlen(argv[1]), (u_char *) values.hash_to_break);
    if(find_match_in(&values) == MATCH) {
        printf("SUCCESS: %s\n", values.word);
    } else {
        printf("FAILED\n");
    }

    free(values.hash_to_break);
    free(values.word);
    return 0;
}

void print_hash(u_char *hash) {
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}
