cmake_minimum_required(VERSION 3.8)
project(net5)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c check_strings.c check_strings.h check_strings_inner.h server.c server.h client.c client.h cur_clients.c cur_clients.h task_maker.c task_maker.h handlers.c handlers.h get_task.c get_task.h types.h protocol.h)

link_libraries(ssl)
link_libraries(crypto)
link_libraries(uuid)

add_executable(net5 ${SOURCE_FILES})
