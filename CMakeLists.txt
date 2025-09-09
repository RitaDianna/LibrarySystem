cmake_minimum_required(VERSION 3.10)
project(LibrarySystem)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(LibrarySystem Src/main.cpp Src/database.cpp Src/sha256.cpp
        lib/sqlite3.c
        lib/sqlite3.h
)

target_include_directories(LibrarySystem PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
