cmake_minimum_required(VERSION 3.12)

project(explore)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost REQUIRED)
find_package(GTest REQUIRED)

file(GLOB_RECURSE SOURCES "boost_lib.cpp" "algorithms.cpp" "modern_cpp.cpp" "language_features.cpp" "explore_cpp.cpp" "behavioral_patterns.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} dl -pthread GTest::GTest gmock GTest::Main boost_system boost_date_time)