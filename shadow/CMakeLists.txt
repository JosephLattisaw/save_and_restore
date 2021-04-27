cmake_minimum_required(VERSION 3.0.0)
project(shadow VERSION 0.1.0)

include(CTest)
enable_testing()

add_definitions(-std=c++17)
add_definitions(-DVBOX_WITH_XPCOM_NAMESPACE_CLEANUP)
add_definitions(-DRT_OS_LINUX)
add_definitions(-fshort-wchar)
add_definitions(-Wno-extra)

find_package(Boost 1.74.0 REQUIRED COMPONENTS program_options)

find_library(VirtualBoxSDK NAMES VBoxXPCOM.so HINTS "/usr/lib/virtualbox")
if(NOT VirtualBoxSDK)
message(FATAL_ERROR "VirtualBoxSDK library wasn't found")
else()
message(STATUS "Found VirtualBoxSDK: ${VirtualBoxSDK}")
endif()

include_directories(../virtualbox/sdk/bindings/xpcom/include)
include_directories(../virtualbox/sdk/bindings/xpcom/include/nsprpub)
include_directories(../virtualbox/sdk/bindings/xpcom/include/string)
include_directories(../virtualbox/sdk/bindings/xpcom/include/xpcom)
include_directories(../virtualbox/sdk/bindings/xpcom/include/ipcd)

add_executable(shadow main.cpp shadow.cpp host_server.cpp vm_handler.cpp)
target_link_libraries(shadow ${Boost_LIBRARIES} pthread ${VirtualBoxSDK})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
