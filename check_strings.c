#include "check_strings.h"
#include "check_strings_inner.h"

int find_match_in(range_values_t *range_values) {
    lengths_t lengths;

    int is_valid = validate_and_get_limits(range_values, &lengths);
    if(is_valid == INVALID) {
        return INVALID_ARGS;
    }

    ushort reverse_table[256];
    ushort alphabet_length = lengths.alphabet_length;
    
    for(ushort i = 0; i < alphabet_length; ++i) {
        reverse_table[(u_char) range_values->alphabet[i]] = i;
    }

    ushort min_length = lengths.min_length;
    char *word = range_values->word;
    
    word[min_length] = '\0';
    lengths.word_length = min_length;

    if (find_match_in_inner(range_values, &lengths, reverse_table, 0)) {
        return MATCH;
    }

    ushort max_length = lengths.max_length;
    for (ushort length = min_length + (ushort) 1; length < max_length; ++length) {
        word[length + 1] = '\0';
        lengths.word_length = length;
        if (find_match_inner(range_values, &lengths, reverse_table, 0) == MATCH) {
            return MATCH;
        }
    }

    word[max_length + 1] = '\0';
    lengths.word_length = max_length;
    if (find_match_in_inner(range_values, &lengths, reverse_table, 0)) {
        return MATCH;
    }

    return MISMATCH;
}

int find_match_inner(
        range_values_t *range_values,
        lengths_t *lengths,
        ushort *reverse_table,
        ushort index
) {
    for (int i = 0; i < lengths->alphabet_length; ++i) {
        char *word = range_values->word;
        ushort word_length = lengths->word_length;

        word[index] = range_values->alphabet[i];
        
        if (index < word_length - 1) {
            int found = find_match_inner(
                    range_values,
                    lengths,
                    reverse_table,
                    index + (ushort) 1
            );
            
            if (found == MATCH) {
                return MATCH;
            }
        } else {
            if (check_md5(word, word_length, range_values->hash_to_break) == MATCH) {
                return MATCH;
            }
        }
    }

    return MISMATCH;
}

int find_match_in_inner(
        range_values_t *range_values,
        lengths_t *lengths,
        ushort *reverse_table,
        ushort index
) {
    char *word = range_values->word;
    ushort word_length = lengths->word_length;

    int first = get_position(
            0,
            range_values->begin_word,
            word,
            word_length,
            index,
            reverse_table
    );
    int last = get_position(
            lengths->alphabet_length - (ushort) 1,
            range_values->end_word,
            word,
            word_length,
            index,
            reverse_table
    );
    
    for (int i = first; i <= last; ++i) {
        word[index] = range_values->alphabet[i];
        if (index < word_length - 1) {
            if (find_match_in_inner(
                    range_values,
                    lengths,
                    reverse_table,
                    index + (ushort) 1
            ) == MATCH) {
                return MATCH;
            }
        } else {
            if (check_md5(word, word_length, range_values->hash_to_break) == MATCH) {
                return MATCH;
            }
        }
    }

    return MISMATCH;
}


int validate_and_get_limits(range_values_t *range_values, lengths_t *lengths) {

    size_t alphabet_length = strlen(range_values->alphabet);

    if(alphabet_length >= 1 && alphabet_length < 256) {
        lengths->alphabet_length = (ushort) alphabet_length;
    } else {
        return INVALID;
    }

    size_t min_length = strlen(range_values->begin_word);
    size_t max_length = strlen(range_values->end_word);

    if(min_length > max_length) {
        return INVALID;
    }

    if((min_length >= MIN_WORD_LENGTH) && (min_length <= MAX_WORD_LENGTH)) {
        lengths->min_length = (ushort) min_length;
    } else {
        return INVALID;
    }

    if((max_length >= MIN_WORD_LENGTH) && (max_length <= MAX_WORD_LENGTH)) {
        lengths->max_length = (ushort) max_length;
    } else {
        return INVALID;
    }

    return VALID;
}

int check_md5(char *word, size_t length, char *hash_to_break) {
    char word_hash[MD5_DIGEST_LENGTH];

    MD5((u_char*) word, length, (u_char*) word_hash);

    if (strncmp(hash_to_break, word_hash, MD5_DIGEST_LENGTH) == 0) {
        return MATCH;
    }
    return MISMATCH;
}

ushort get_position(
        ushort default_value,
        const char *key_word,
        const char *word,
        ushort word_length,
        ushort index,
        ushort *reverse_table
) {
    int same_length = (word_length == strlen(key_word));
    if(!same_length) {
        return default_value;
    }
    int start_key_word = (index == 0);
    if(start_key_word) {
        return reverse_table[key_word[index]];
    }
    int continue_key_word = (strncmp(word, key_word, index) == 0);
    return continue_key_word ? reverse_table[key_word[index]] : default_value;
}
