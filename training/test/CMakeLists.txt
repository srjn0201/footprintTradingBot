cmake_minimum_required(VERSION 3.10)
project(FootprintTest)

# Find SQLite3
find_package(SQLite3 REQUIRED)

add_executable(test_initialization
    testForInitialization.cpp
    ../database/sqlite.cpp
)

target_include_directories(test_initialization PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(test_initialization PRIVATE SQLite::SQLite3)
