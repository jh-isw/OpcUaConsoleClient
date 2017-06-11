set(open62541_SRCS
        open62541.h
        open62541.c
        )

# open62 requires c99-standard
#set(CMAKE_C_STANDARD 99) gives -std=gnu99
set(CMAKE_C_FLAGS "-std=c99")

add_library(open62541 STATIC
        ${open62541_SRCS}
        )

target_include_directories(open62541 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )
