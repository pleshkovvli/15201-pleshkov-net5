#include "server.h"
#include "sock_polls.h"
#include "success.h"

void run_server(const char *hash, uint16_t port) {
    int server_socket_fd = server_socket(port);

    if (server_socket_fd == FAILURE_CODE) {
        exit(EXIT_FAILURE);
    }

    task_manager_t task_manager;
    init_task_manager(&task_manager, hash);

    struct pollfd *sock_polls;
    init_sock_polls(&sock_polls, server_socket_fd);

    cur_clients_t cur_clients;
    init_cur_clients(&cur_clients, sock_polls);

    success_t success;
    init_success(&success);

    int closing = FALSE;

    while (!closing || task_manager.tasks_going->amount > 0 || cur_clients.amount > 0) {
        //printf("Closing is %d, tasks are %d, clients are %ld\n", closing, task_manager.tasks_list->amount, cur_clients.amount);

        int events = poll(sock_polls, cur_clients.amount + 1, TIMEOUT_MS);

        fprintf(stderr, "Poll\n");

        check_tasks(&task_manager);

        if (events == 0) {
            fprintf(stderr, "Empty poll\n");
            continue;
        }

        if (sock_polls[0].revents == POLLIN) {
            --events;
            fprintf(stderr, "Accepting client\n");
            int client_fd = accept(server_socket_fd, NULL, NULL);
            add_client(&cur_clients, client_fd);
        }

        for (int sock_num = 0; sock_num < cur_clients.amount && events > 0; ++sock_num) {
            struct pollfd *cur_poll = &cur_clients.polls[sock_num];
            if (cur_poll->revents == 0) {
                continue;
            }

            --events;
            if (cur_poll->revents != POLLIN) {
                fprintf(stderr, "ERROR: removing client on poll\n");


                remove_client(&cur_clients, sock_num);
                continue;
            }

            client_t *cur_client = &(cur_clients.clients[sock_num]);

            ssize_t read = recv(cur_poll->fd, cur_client->buffer, CLIENT_BUF_SIZE, 0);

            if (read < 0) {
                fprintf(stderr, "ERROR: removing client on read\n");
                remove_client(&cur_clients, sock_num);

                continue;
            }

            cur_client->bytes_read += read;

            enum handle_res for_close = UNFINISHED;

            if (cur_client->state == UNKNOWN) {
                fprintf(stderr, "UNKNOWN\n");
                for_close = try_handle_unknown(cur_client);
            }

            if(closing && (for_close == UNFINISHED)) {
                for_close = try_handle_close(&task_manager, cur_poll->fd, cur_client);
            }

            switch (cur_client->state) {
                case UNKNOWN:
                    break;
                case NEW: {
                    for_close = try_handle_new(&task_manager, cur_poll->fd, cur_client);
                }
                    break;
                case MORE: {
                    //fprintf(stderr, "MORE\n");
                    for_close = try_handle_more(&task_manager, cur_poll->fd, cur_client);
                }
                    break;
                case SUCCESS: {
                    //fprintf(stderr, "SUCCESS\n");
                    for_close = try_handle_success(
                            task_manager.tasks_going, sock_num, cur_client, &success);
                    if(for_close == SUCCESS_RES) {
                        send_done(cur_poll->fd, cur_client);
                        remove_task(task_manager.tasks_going, cur_client->uuid);
                        closing = TRUE;
                    }
                    if(for_close == FAILURE_RES) {
                        cancel_success(&success);
                    }
                }
                    break;
                case TO_ACK: {
                    fprintf(stderr, "ACK\n");
                    for_close = try_handle_to_ack(cur_client);
                }
                    break;
            }

            cur_poll->revents = 0;
            fprintf(stderr, "Now events are %d\n", cur_poll->events);

            if (for_close != UNFINISHED) {
                if(for_close == FAILURE_RES && cur_client->got_uuid) {
                    int task_num = check_uuid(task_manager.tasks_going, cur_client);
                    if(task_num != FAILURE_CODE) {
                        move_task(task_manager.tasks_to_do, task_manager.tasks_going, task_num);
                    }
                }

                fprintf(stderr, "Removing client\n");
                remove_client(&cur_clients, sock_num);
            }
        }
    }


    fprintf(stderr, "Finishing server\n");
    close(server_socket_fd);

    destroy_success(&success);
    destroy_sock_polls(sock_polls);
    destroy_task_manager(&task_manager);
    destroy_cur_clients(&cur_clients);
}
