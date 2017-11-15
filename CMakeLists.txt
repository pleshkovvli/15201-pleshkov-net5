cmake_minimum_required(VERSION 3.8)
project(net5)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c md5-cracker/md5-cracker.c md5-cracker/md5-cracker.h md5-cracker/check_strings_inner.h server/server.c server/server.h client/client.c client/client.h server/cur_clients.c server/cur_clients.h task_manager/task_manager.c task_manager/task_manager.h server/handlers.c server/handlers.h types.h protocol.h task_manager/str_gen.c task_manager/str_gen.h utils/memcpy_next.c utils/memcpy_next.h task_manager/task_list.c task_manager/task_list.h utils/sock_utils.c utils/sock_utils.h server/sock_polls.c server/sock_polls.h server/success.c server/success.h)

link_libraries(ssl)
link_libraries(crypto)
link_libraries(uuid)

add_executable(net5 ${SOURCE_FILES})
