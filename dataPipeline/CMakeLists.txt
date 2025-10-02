
cmake_minimum_required(VERSION 3.16)
project(DataCleaner VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

list(APPEND CMAKE_PREFIX_PATH "$ENV{HOME}/.local")

find_package(Arrow REQUIRED)
find_package(Parquet REQUIRED)
find_package(SQLite3 REQUIRED)

# --- SQLiteCpp Library (submodule) ---
add_subdirectory(libs/SQLiteCpp)

# --- Date Library (submodule) ---
# Enable the compiled timezone library
set(BUILD_TZ_LIB ON CACHE BOOL "Build the timezone library" FORCE)
# Add the submodule directory
add_subdirectory(libs/date)

add_executable(data_cleaner src/main.cpp)

target_link_libraries(data_cleaner PRIVATE
  # Link against the timezone library
  date::date-tz
  
  # Link against Arrow and Parquet
  Arrow::arrow_shared
  Parquet::parquet_shared
  
  # Link against SQLite
  SQLite::SQLite3
  
  # Link against SQLiteCpp
  SQLiteCpp

  pthread
  dl
)
