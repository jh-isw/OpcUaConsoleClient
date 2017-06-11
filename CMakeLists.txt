cmake_minimum_required(VERSION 3.7)
project(OpcUaConsoleClient)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(open62541)

set(SOURCE_FILES main.cpp)

set(CMAKE_CXX_STANDARD 11)
add_executable(OpcUaConsoleClient ${SOURCE_FILES})
target_link_libraries(OpcUaConsoleClient open62541 readline)
