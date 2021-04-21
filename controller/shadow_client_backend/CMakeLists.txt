cmake_minimum_required(VERSION 3.0.0)
project(shadow_client_backend VERSION 0.1.0)

include(CTest)
enable_testing()

find_package(Boost 1.74.0 REQUIRED)

add_library(shadow_client_backend SHARED shadow_client_backend.cpp client.cpp)
target_link_libraries(shadow_client_backend ${Boost_LIBRARIES} pthread)

add_executable(shadow_client_backend_test shadow_client_backend.cpp client.cpp)
target_link_libraries(shadow_client_backend_test ${Boost_LIBRARIES} pthread)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
