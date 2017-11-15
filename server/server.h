#ifndef NET5_SERVER_H
#define NET5_SERVER_H

#include "../protocol.h"
#include "../types.h"
#include "cur_clients.h"
#include "../task_manager/task_manager.h"
#include "handlers.h"

#include <sys/socket.h>
#include <stdio.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <memory.h>
#include <poll.h>
#include <uuid/uuid.h>
#include <zconf.h>

#define TIMEOUT_MS 3000
#define TIME_TO_DO 30

void run_server(const char *hash, uint16_t port);


#endif //NET5_SERVER_H
