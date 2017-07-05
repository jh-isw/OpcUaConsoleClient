cmake_minimum_required(VERSION 3.5)
project(OpcUaConsoleClient)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(open62541)

set(SOURCE_FILES main.cpp)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost 1.58 REQUIRED COMPONENTS program_options)

add_executable(OpcUaConsoleClient ${SOURCE_FILES})
target_link_libraries(OpcUaConsoleClient open62541 readline ${Boost_LIBRARIES})
