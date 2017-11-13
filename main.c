#include <stdio.h>
#include <openssl/md5.h>
#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <unistd.h>
#include "check_strings.h"
#include "server.h"
#include "client.h"

int main(int argc, char *argv[]) {
    if(fork() == 0) {
        sleep(2);
        run_client("127.0.0.1", 3112);
    } else if(fork() == 0) {
        sleep(2);
        run_client("127.0.0.1", 3112);
    } else if(fork() == 0) {
        sleep(2);
        run_client("127.0.0.1", 3112);
    } else {
        char *string = "TCGA";
        char hash[MD5_DIGEST_LENGTH];
        MD5((u_char *) string, strlen(string), (u_char *) hash);
        run_server(hash, 3112);
    }

    return 0;
}

