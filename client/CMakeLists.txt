cmake_minimum_required(VERSION 3.8)
project(net5)

set(CMAKE_C_STANDARD 99)

file(GLOB_RECURSE CLIENT_SRC "*.h" "*.c")

add_subdirectory(./../utils utils)

include_directories(${UTILS_INCLUDE_DIR})

link_libraries(ssl)
link_libraries(crypto)
link_libraries(uuid)

add_executable(net5client ${CLIENT_SRC})
target_link_libraries(net5client utils)