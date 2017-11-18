cmake_minimum_required(VERSION 3.8)

set(CMAKE_C_STANDARD 99)

file(GLOB_RECURSE SERVER_SRC "*.h" "*.c")

add_subdirectory(./../utils utils)

include_directories(${UTILS_INCLUDE_DIR})

link_libraries(uuid)

add_executable(net5server ${SERVER_SRC})
target_link_libraries(net5server utils)