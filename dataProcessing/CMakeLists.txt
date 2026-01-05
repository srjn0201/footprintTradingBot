cmake_minimum_required(VERSION 3.10)
project(FootprintTraining)

# Force C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SQLite3 REQUIRED)

add_executable(footprint_trainer 
    main.cpp 
    database/sqlite.cpp 
    database/json_writer.cpp
    finalProcessing.cpp
    initializeContract.cpp
    initializeNewDay.cpp
    src/updateFootprint.cpp
    src/signalCheck.cpp
    src/updateTickSensitiveFeatures.cpp
    src/updatePriceSensitiveFeatures.cpp
    src/initializeNewBar.cpp
    src/finalizeProcessingDay.cpp
    src/updateBarSensitiveFeatures.cpp
    src/updateDaySensitiveFeatures.cpp
    src/updateWeekSensitiveFeatures.cpp
    src/initializeWeek.cpp
    src/finalizeContract.cpp
    src/indicators/BBands.cpp
    src/indicators/RSI.cpp
    src/indicators/dayVWAP.cpp
    src/indicators/weekVWAP.cpp 
    src/indicators/deltaZscore.cpp
    src/indicators/cumDelta5barSlope.cpp
    src/indicators/deltaDivergence.cpp
    src/indicators/interactionReversal.cpp
    src/TPO/dayTPO.cpp
    src/TPO/weekTPO.cpp
    convertDatesToWeek.cpp
)

target_link_libraries(footprint_trainer PRIVATE SQLite::SQLite3)
target_include_directories(footprint_trainer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})