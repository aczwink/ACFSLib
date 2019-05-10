cmake_minimum_required(VERSION 3.6)
project(ACFSLib)
find_package(XP-CMake REQUIRED) #we need advanced cmake modules

set(CMAKE_CXX_STANDARD 17)

XPC_AutoBuildType()
XPC_OptForHost()
XPC_SetCompileDefinitions()

find_package(StdPlusPlus REQUIRED)

add_subdirectory(src_acfs)
add_library(acfs include/ACFSLib.hpp ${SOURCE_FILES})
target_include_directories(acfs PRIVATE include)
target_link_libraries(acfs PRIVATE Std++)

add_executable(ArC src_arc/main.cpp)
target_include_directories(ArC PRIVATE include)
target_link_libraries(ArC Std++ Std++Static acfs)

add_executable(gArC src_garc/main.cpp)
target_link_libraries(gArC Std++ Std++Static)