cmake_minimum_required(VERSION 3.8)
project(hw5)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
add_definitions(-std=c99)


set(SOURCE_FILES
        include/const.h
        include/cream.h
        include/debug.h
        include/extracredit.h
        include/hashmap.h
        include/queue.h
        include/utils.h
        src/cream.c
#        src/extracredit.c
        src/hashmap.c
        src/queue.c
        src/utils.c
        tests/extracredit_tests.c
        tests/hashmap_tests.c
        tests/queue_tests.c)

add_executable(hw5 ${SOURCE_FILES})

include_directories(include)
TARGET_LINK_LIBRARIES(hw5 criterion pthread)