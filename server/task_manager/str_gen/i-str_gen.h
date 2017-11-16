#ifndef NET5_I_STR_GEN_H
#define NET5_I_STR_GEN_H

#include <zconf.h>

typedef struct str_gen {
    char *begin_str;
    char *end_str;

    char *cur_aff;
    char *suf_begin;
    char *suf_end;

    char *abc;
    ushort abc_len;
    ushort *char_pos;

    ushort max_str_len;
} str_gen_t;

int init_str_gen(str_gen_t *str_gen, const char *abc, ushort max_str_len);
void destroy_str_gen(str_gen_t *str_gen);

int next_str(str_gen_t *str_gen);

#endif //NET5_I_STR_GEN_H
