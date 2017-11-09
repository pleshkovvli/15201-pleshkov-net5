cmake_minimum_required(VERSION 3.8)
project(net5)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c check_strings.c check_strings.h check_strings_inner.h server.c server.h)

link_libraries(ssl)
link_libraries(crypto)
link_libraries(uuid)

add_executable(net5 ${SOURCE_FILES})
