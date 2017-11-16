cmake_minimum_required(VERSION 3.8)
project(net5)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c client/md5-cracker/md5-cracker.c client/md5-cracker/md5-cracker.h client/md5-cracker/md5-cracker-inner.h server/server.c server/server.h client/client.c client/client.h server/cur_clients/cur_clients.c server/cur_clients/cur_clients.h server/task_manager/task_manager.c server/task_manager/task_manager.h server/handlers.c server/handlers.h types.h protocol.h server/task_manager/str_gen/str_gen.c server/task_manager/str_gen/str_gen.h utils/memcpy_next.c utils/memcpy_next.h server/task_manager/task_list/task_list.c server/task_manager/task_list/task_list.h utils/sock_utils.c utils/sock_utils.h server/sock_polls/sock_polls.c server/sock_polls/sock_polls.h server/success/success.c server/success/success.h server/i-server.h agreements.h server/task_manager/task_list/task.c server/task_manager/task_list/task.h server/task_manager/str_gen/i-str_gen.h server/cur_clients/client_type.h utils/print_uuid.c utils/print_uuid.h)

link_libraries(ssl)
link_libraries(crypto)
link_libraries(uuid)

add_executable(net5 ${SOURCE_FILES})
