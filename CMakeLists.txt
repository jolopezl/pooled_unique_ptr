cmake_minimum_required(VERSION 3.17)

project(pooled_unique_ptr)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-g -O0 -Wall -Wextra -fsanitize=address -fno-omit-frame-pointer")
    set (CMAKE_EXE_LINKER_FLAG ${CMAKE_EXE_LINKER_FLAGS} "-g -fsanitize=address")
else ()
    set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-O3 -Wall -Wextra -fno-omit-frame-pointer")
    set (CMAKE_EXE_LINKER_FLAG ${CMAKE_EXE_LINKER_FLAGS} "-O3")
endif()

add_executable(run_test_pooled_unique_ptr test_pooled_unique_ptr.cpp)
