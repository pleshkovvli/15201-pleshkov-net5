#include <stdio.h>
#include <openssl/md5.h>
#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <unistd.h>
#include "md5-cracker/md5-cracker.h"
#include "server/server.h"
#include "client/client.h"

int main(int argc, char *argv[]) {
    if(fork() == 0) {
        printf("RUN");
        run_client("127.0.0.1", 3112);
    } else if(fork() == 0) {
        printf("RUN");
        run_client("127.0.0.1", 3112);
    } else if(fork() == 0) {
        printf("RUN");
        run_client("127.0.0.1", 3112);
    } else {
        char *string = "TCGTTTATATATAT";
        char hash[MD5_DIGEST_LENGTH];
        MD5((u_char *) string, strlen(string), (u_char *) hash);
        run_server(hash, 3112);
    }

    return 0;
}

