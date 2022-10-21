cmake_minimum_required(VERSION 3.23)
project(gbapp_win32 LANGUAGES C VERSION 0.0.0)

add_library(gbapp_win32 gbapp_win32.c)
add_definitions(-DUNICODE -D_UNICODE)
target_include_directories(gbapp_win32
    PUBLIC 
    "${CMAKE_CURRENT_LIST_DIR}"
)
