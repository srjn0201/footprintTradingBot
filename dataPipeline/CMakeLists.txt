# specifying minimum version of cmake required
cmake_minimum_required(VERSION 3.16)

#defining the project name and version
project(DataPipeline VERSION 1.0)

#setting the C++ version
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Tell CMake to look for libraries in our install directory
set(CMAKE_PREFIX_PATH $ENV{HOME}/.local)

# 2. Find the pre-installed packages
find_package(Arrow REQUIRED)
find_package(Parquet REQUIRED)
find_package(SQLite3 REQUIRED)

#finding our source file and defining the executable
add_executable(data_cleaner src/main.cpp)

# Tell our program where to find the SQLAPI++ headers
# CORRECTED FOLDER NAME: "SQLAPI"
target_include_directories(data_cleaner PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/libs/SQLAPI/include
)

# Link our program against ALL libraries in a single command
target_link_libraries(data_cleaner PRIVATE
  # Arrow and Parquet
  Arrow::arrow_shared
  Parquet::parquet_shared
  
  # SQLAPI++ and its dependencies
  # CORRECTED FOLDER AND FILE NAME: "SQLAPI" and "libsqlapi.a"
  ${CMAKE_CURRENT_SOURCE_DIR}/libs/SQLAPI/lib/libsqlapi.a
  SQLite::SQLite3
  pthread
  dl
)