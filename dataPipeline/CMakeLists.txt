# specifying minimum version of cmake required
cmake_minimum_required(VERSION 3.16)

#defining the project name and version
project(DataPipeline VERSION 1.0)

#setting the C++ version
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#finding our source file from src/main.cpp and defining the executable named 'data_cleaner'
add_executable(data_cleaner src/main.cpp)