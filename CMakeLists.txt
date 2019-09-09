cmake_minimum_required(VERSION 3.12)

project(explore)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost REQUIRED)
find_package(GTest REQUIRED)

file(GLOB_RECURSE SOURCES "explore_cpp.cpp" "explore_duktape.cpp" "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${GTEST_MAIN_LIBRARIES} ${GTEST_LIBRARIES} boost_system boost_date_time)