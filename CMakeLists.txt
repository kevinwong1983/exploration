cmake_minimum_required(VERSION 3.12)

project(explore)
set(CMAKE_CXX_STANDARD 14)
#set(DCMAKE_C_COMPILER /usr/bin/gcc)
#set(DCMAKE_CXX_COMPILER /usr/bin/g++)

include(cmake/FindPackageTaoJson.cmake)
find_package(Boost REQUIRED)
find_package(GTest REQUIRED)
find_package(RapidJSON)
find_package(Sqlite3 REQUIRED)

file(GLOB_RECURSE SOURCES
        "boost_lib.cpp"
        "algorithms.cpp"
        "modern_cpp.cpp"
        "language_features.cpp"
        "explore_cpp.cpp"
        "compound_patterns.cpp"
        "behavioral_patterns.cpp"
        "concurrency.cpp"
        "solid.cpp"
        "gmock.cpp"
        "templates/*.*"
        "solid/*.*"
        "design_patterns/creational/*/*.*"
        "design_patterns/structural/*/*.*"
        "design_patterns/behavioral/*/*.*"
        "json.cpp"
        "cherno.cpp"
        )

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
        dl
        -pthread
        -lsqlite3
        GTest::GTest
        GTest::Main
        gmock_main
        gmock
        boost_system
        boost_date_time
        boost_serialization
        boost_regex
        taocpp::json
        )

target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS} .)