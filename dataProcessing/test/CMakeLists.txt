cmake_minimum_required(VERSION 3.10)
project(FootprintTest)

# Find SQLite3
find_package(SQLite3 REQUIRED)

add_executable(test_vwap
    test_vwap.cpp
    /home/sarjil/sarjil_u/C++/footprintTradingBot/dataProcessing/src/indicators/VWAP.cpp
)

target_include_directories(test_vwap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(test_vwap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_link_libraries(test_vwap PRIVATE SQLite::SQLite3)