#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include "i-server.h"
#include "../utils/include/sock_utils.h"


char hex_to_dec(char hex);

int main(int argc, char *argv[]) {
    if(argc < 3) {
        printf("Usage: ./net5server HASH PORT\n");
        exit(EXIT_FAILURE);
    }

    if(strnlen(argv[1], MD5_DIGEST_LENGTH * 2 + 1) != (MD5_DIGEST_LENGTH * 2)) {
        printf("%s is not valid hash\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    uint16_t port = get_port(argv[2]);
    if(port == 0) {
        exit(EXIT_FAILURE);
    }

    char hash[MD5_DIGEST_LENGTH];
    for(int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        hash[i] = hex_to_dec(argv[1][i * 2]) * (u_char) 16 + hex_to_dec(argv[1][i * 2 + 1]);
    }

    run_server(hash, port);
}

char hex_to_dec(char hex) {
    if(hex <= 'F' && hex >= 'A') {
        return (u_char) (hex - 'A' + 10);
    }
    if(hex <= 'f' && hex >= 'a') {
        return (u_char) (hex - 'a' + 10);
    }

    return (u_char) (hex - '0');
}
