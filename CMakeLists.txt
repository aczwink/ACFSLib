cmake_minimum_required(VERSION 3.6)
project(ACFSLib)

set(CMAKE_CXX_STANDARD 11)

#debug or release
if(CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(-D_DEBUG)
else()
    message(FATAL_ERROR "Unknown build type. Must be either 'Debug' or 'Release'")
endif()

find_library(STDPLUSPLUS_LIBRARY NAMES Std++_d REQUIRED)
find_library(STDPLUSPLUSSTATIC_LIBRARY NAMES Std++Static_d REQUIRED)

set(LIBS ${STDPLUSPLUS_LIBRARY} ${STDPLUSPLUSSTATIC_LIBRARY})

add_library(acfs include/ACFSLib.hpp src_acfs/ACFSLib.cpp src_acfs/scct_umd/SCCT_UMD_Format.hpp src_acfs/scct_umd/UMD.hpp src_acfs/scct_umd/SCCT_UMD_FileSystem.hpp)
target_include_directories(acfs PRIVATE include)
target_link_libraries(acfs ${LIBS})

add_executable(ArC src_arc/main.cpp)
target_include_directories(ArC PRIVATE include)
target_link_libraries(ArC ${LIBS})
target_link_libraries(ArC acfs)