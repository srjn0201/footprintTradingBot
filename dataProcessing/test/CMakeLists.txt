cmake_minimum_required(VERSION 3.10)
project(FootprintTest)

# Find SQLite3
find_package(SQLite3 REQUIRED)



add_executable(test_day_tpo
    test_day_tpo.cpp
    /home/sarjil/sarjil_u/C++/footprintTradingBot/dataProcessing/src/TPO/dayTPO.cpp
)

target_include_directories(test_day_tpo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(test_day_tpo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_link_libraries(test_day_tpo PRIVATE SQLite::SQLite3)