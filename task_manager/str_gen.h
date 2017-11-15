#ifndef NET5_STR_GEN_H
#define NET5_STR_GEN_H

#include <zconf.h>

#define SUF_LEN 9

typedef struct str_gen {
    char *begin_str;
    char *end_str;

    char *cur_aff;
    char *suf_begin;
    char *suf_end;

    char *abc;
    ushort abc_len;
    ushort *char_pos;
} str_gen_t;

int init_str_gen(str_gen_t *str_gen, const char *abc);
void destroy_str_gen(str_gen_t *str_gen);

void next_str(str_gen_t *str_gen);

enum next_action {
    ADD, NOT_ADD
};

enum next_action next_string(str_gen_t *str_gen, ushort length, int index);

enum next_action change_sym(str_gen_t *str_gen, int index);

char *make_suf(char sym, ushort len);

#endif //NET5_STR_GEN_H
