cmake_minimum_required(VERSION 3.10)
project(FootprintTraining)

# Find SQLite3
find_package(SQLite3 REQUIRED)

# Find Arrow and Parquet
find_package(Arrow REQUIRED)
find_package(Parquet REQUIRED)

add_executable(footprint_trainer main.cpp database/sqlite.cpp database/parquet_writer.cpp)

target_include_directories(footprint_trainer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(footprint_trainer PRIVATE SQLite::SQLite3 Arrow::arrow_shared Parquet::parquet_shared)
