cmake_minimum_required(VERSION 3.6)
project(ACFSLib)
find_package(XP-CMake REQUIRED) #we need advanced cmake modules

set(CMAKE_CXX_STANDARD 20)

XPC_AutoBuildType()
XPC_SetCompileDefinitions()

find_package(StdPlusPlus REQUIRED)

add_subdirectory(src_acfs)
add_library(acfs include/ACFSLib.hpp ${SOURCE_FILES})
target_include_directories(acfs PRIVATE include)
target_link_libraries(acfs PRIVATE Std++)

add_executable(ArC src_arc/main.cpp src_arc/extract.cpp src_arc/info.cpp src_arc/pack.cpp)
target_include_directories(ArC PRIVATE include)
target_link_libraries(ArC Std++ Std++Static acfs)

add_executable(gArC src_garc/main.cpp)
target_link_libraries(gArC Std++ Std++Static)

add_executable(test_manual src_tests/manual.cpp)
target_include_directories(test_manual PRIVATE include)
target_link_libraries(test_manual Std++ Std++Static acfs)