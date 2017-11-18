#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../utils/include/sock_utils.h"
#include "client.h"

int main(int argc, char *argv[]) {
    if(argc < 3) {
        printf("Usage: ./net5client IP PORT\n");
        exit(EXIT_FAILURE);
    }

    uint16_t port = get_port(argv[2]);
    if(port == 0) {
        exit(EXIT_FAILURE);
    }

    run_client(argv[1], port);
}