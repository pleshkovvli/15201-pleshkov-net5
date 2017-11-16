#ifndef NET5_STR_GEN_H
#define NET5_STR_GEN_H

#include "i-str_gen.h"

#define SUF_LEN 9

enum next_action {
    ADD, NOT_ADD
};

enum next_action next_string(str_gen_t *str_gen, ushort length, int index);

enum next_action change_sym(str_gen_t *str_gen, int index);

char *make_suf(char sym, ushort len);

#endif //NET5_STR_GEN_H
