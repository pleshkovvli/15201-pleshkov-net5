cmake_minimum_required(VERSION 3.8)

set(CMAKE_C_STANDARD 99)

set(UTILS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)

set(UTILS_INCLUDE_DIR ${UTILS_INCLUDE_DIR} PARENT_SCOPE)

if(TARGET utils)
    message("utils is already defined")
else()
    message("defining utils")
    include_directories(${UTILS_INCLUDE_DIR})
    set(UTILS_SRCS ./src/memcpy_next.c ./src/print_uuid.c ./src/sock_utils.c ./src/logger.c)
    add_library(utils STATIC ${UTILS_SRCS})
endif()