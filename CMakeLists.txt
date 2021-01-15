cmake_minimum_required(VERSION 3.12)

project(explore)
message("asdf")
set(CMAKE_CXX_STANDARD 14)


include(cmake/FindPackageTaoJson.cmake)
find_package(Boost REQUIRED)
find_package(GTest REQUIRED)
find_package(RapidJSON)

file(GLOB_RECURSE SOURCES
        "boost_lib.cpp"
        "algorithms.cpp"
        "modern_cpp.cpp"
        "language_features.cpp"
        "explore_cpp.cpp"
        "behavioral_patterns.cpp"
        "concurrency.cpp"
        "solid.cpp"
        "solid.cpp"
        "gmock.cpp"
        "templates/*.*"
        "solid/*.*"
        "creational_patterns/*/*.*"
        "structural_patterns/*/*.*"
        "behavioral/*/*.*"
        "json.cpp"
        "cherno.cpp")

add_executable(${PROJECT_NAME} ${SOURCES} behavioral/mediator/ChatRoom.cpp behavioral/mediator/ChatRoom.h behavioral/mediator/Person.cpp behavioral/mediator/Person.cpp behavioral/mediator/Person.h)

target_link_libraries(${PROJECT_NAME}
        dl
        -pthread
        GTest::GTest
        gmock
        GTest::Main
        boost_system
        boost_date_time
        boost_serialization
        boost_regex
        taocpp::json)

target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS} .)