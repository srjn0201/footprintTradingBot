cmake_minimum_required(VERSION 3.10)
project(FootprintTraining)

# Find SQLite3
find_package(SQLite3 REQUIRED)

add_executable(footprint_trainer 
    main.cpp 
    database/sqlite.cpp 
    database/json_writer.cpp
    finalProcessing.cpp
    initializeContract.cpp
    src/updateFootprint.cpp
    src/signalCheck.cpp
    src/updateTickSensitiveFeatures.cpp
    src/updatePriceSensitiveFeatures.cpp
    src/finalizeLastBar.cpp
    src/finalizeProcessingDay.cpp
    src/updateBarSensitiveFeatures.cpp
    src/updateDaySensitiveFeatures.cpp
    src/updateWeekSensitiveFeatures.cpp
    src/finalizeWeek.cpp
    src/finalizeContract.cpp
    convertDatesToWeek.cpp
)

target_include_directories(footprint_trainer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(footprint_trainer PRIVATE SQLite::SQLite3)