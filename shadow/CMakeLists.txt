cmake_minimum_required(VERSION 3.0.0)
project(shadow VERSION 0.1.0)

include(CTest)
enable_testing()

add_definitions(-std=c++17)

find_package(Boost 1.74.0 REQUIRED COMPONENTS program_options)

add_executable(shadow main.cpp shadow.cpp host_server.cpp)
target_link_libraries(shadow ${Boost_LIBRARIES} pthread)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
