cmake_minimum_required(VERSION 3.10)
project(LibrarySystem)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SQLite3 REQUIRED)

add_executable(LibrarySystem src/main.cpp src/database.cpp src/sha256.cpp
)

target_include_directories(LibrarySystem PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(LibrarySystem PRIVATE SQLite::SQLite3)