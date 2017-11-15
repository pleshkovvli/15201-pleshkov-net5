#include <stdlib.h>
#include <memory.h>
#include "str_gen.h"
#include "../protocol.h"
#include "../utils/memcpy_next.h"

int init_str_gen(str_gen_t *str_gen, const char *abc) {
    str_gen->cur_aff = malloc(sizeof(char) * (MAX_STR_LEN - SUF_LEN + 1));
    if (str_gen->cur_aff == NULL) {
        return FAILURE_CODE;
    }

    str_gen->cur_aff[0] = '\0';
    
    str_gen->abc_len = (ushort) strlen(abc);
    ushort abc_len = str_gen->abc_len;

    str_gen->abc = malloc(abc_len + 1);
    if (str_gen->abc == NULL) {
        return FAILURE_CODE;
    }

    strncpy(str_gen->abc, abc, abc_len + 1);

    str_gen->suf_begin = make_suf(str_gen->abc[0], SUF_LEN);
    if (str_gen->suf_begin == NULL) {
        return FAILURE_CODE;
    }

    str_gen->suf_end = make_suf(str_gen->abc[abc_len - 1], SUF_LEN);
    if (str_gen->suf_end == NULL) {
        return FAILURE_CODE;
    }

    str_gen->begin_str = malloc(sizeof(char) * (MAX_STR_LEN + 1));
    if (str_gen->begin_str == NULL) {
        return FAILURE_CODE;
    }

    str_gen->begin_str[0] = str_gen->abc[0];
    str_gen->begin_str[1] = '\0';

    str_gen->end_str = malloc(sizeof(char) * (MAX_STR_LEN + 1));
    if (str_gen->end_str == NULL) {
        return FAILURE_CODE;
    }

    strncpy(str_gen->end_str, str_gen->suf_end, SUF_LEN + 1);

    str_gen->char_pos = malloc(sizeof(ushort) * 256);
    if (str_gen->char_pos == NULL) {
        return FAILURE_CODE;
    }

    for (ushort i = 0; i < abc_len; ++i) {
        str_gen->char_pos[(u_char) str_gen->abc[i]] = i;
    }

    return SUCCESS_CODE;
}

void destroy_str_gen(str_gen_t *str_gen) {
    free(str_gen->abc);

    free(str_gen->suf_begin);
    free(str_gen->suf_end);

    free(str_gen->begin_str);
    free(str_gen->end_str);

    free(str_gen->char_pos);
    free(str_gen->cur_aff);
}

void update_str(char *str, const char *aff, const char *suf);

void next_str(str_gen_t *str_gen) {
    char *cur_aff = str_gen->cur_aff;

    if(cur_aff[0] == '\0') {
        cur_aff[0] = 'A';
        cur_aff[1] = '\0';
    } else {
        ushort length = (ushort) strlen(cur_aff);
        enum next_action add = next_string(str_gen, length, 0);
        if (add == ADD) {
            cur_aff[length] = str_gen->abc[0];
            cur_aff[length + 1] = '\0';
        }
    }

    update_str(str_gen->begin_str, str_gen->cur_aff, str_gen->suf_begin);
    update_str(str_gen->end_str, str_gen->cur_aff, str_gen->suf_end);
}

void update_str(char *str, const char *aff, const char *suf) {
    size_t written = 0;
    memcpy_next(str, aff, strlen(aff), &written);
    memcpy_next(str, suf, strlen(suf), &written);
    str[written] = '\0';
}


enum next_action next_string(str_gen_t *str_gen, ushort length, int index) {
    if (index == length - 1) {
        return change_sym(str_gen, index);
    }

    int result = next_string(str_gen, length, index + 1);

    if (result == ADD) {
        return change_sym(str_gen, index);
    }

    return NOT_ADD;
}

enum next_action change_sym(str_gen_t *str_gen, int index) {
    char *string = str_gen->cur_aff;
    const char *abc = str_gen->abc;
    const int abc_len = str_gen->abc_len;
    const ushort *char_pos = str_gen->char_pos;

    char cur_sym = string[index];
    ushort cur_position = char_pos[cur_sym];

    int next_position = (cur_position + 1) % abc_len;

    string[index] = abc[next_position];
    return (next_position == 0) ? ADD : NOT_ADD;
}

char *make_suf(char sym, ushort len) {
    char *suf = malloc(len + 1);
    if(suf == NULL) {
        return NULL;
    }

    for(ushort i = 0; i < len; ++i) {
        suf[i] = sym;
    }
    suf[len] = '\0';

    return suf;
}
