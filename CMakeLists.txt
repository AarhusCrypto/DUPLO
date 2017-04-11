cmake_minimum_required (VERSION 2.8.3)
project (DUPLO CXX)

#Use -DCMAKE_BUILD_TYPE=Release to invoke
IF(${CMAKE_BUILD_TYPE} MATCHES "Release")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/release/")
set(CMAKE_VERBOSE_MAKEFILE FALSE)

#Defaults to debug mode
ELSE(${CMAKE_BUILD_TYPE} MATCHES "Release")
set(${CMAKE_BUILD_TYPE} "DEBUG")
set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/debug/")
ENDIF(${CMAKE_BUILD_TYPE} MATCHES "Release")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/obj/")

#Find libraries
find_package(FindGMP QUIET HINTS cmake/)

#Required for large mallocs on some platforms
set(ENV{MALLOC_PERMIT_INSANE_REQUESTS} "SOME VALUE")

#Compiler flags
set(CMAKE_C_FLAGS "-march=native -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++14")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -g3 -O0 -Wall -pg")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG -g3 -O0 -Wall -pg")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DRELEASE")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DRELEASE")

#Include headers
include_directories(${GMP_INCLUDE_DIR})

#Build sublibraries
add_subdirectory(libs/SplitCommit)

#Build Extended Frigate
add_subdirectory(frigate)

#Include headers
include_directories(${CMAKE_CURRENT_LIST_DIR}/libs/SplitCommit/libs/libOTe)
include_directories(${CMAKE_CURRENT_LIST_DIR}/libs/SplitCommit/libs/libOTe/cryptoTools)
include_directories(${CMAKE_CURRENT_LIST_DIR}/libs/SplitCommit/libs/libOTe/cryptoTools/thirdparty/linux/boost)
include_directories(${CMAKE_CURRENT_LIST_DIR}/libs/SplitCommit/src/)
include_directories(${CMAKE_CURRENT_LIST_DIR}/libs/SplitCommit/libs/)

include_directories(${CMAKE_CURRENT_LIST_DIR}/libs)
include_directories(${CMAKE_CURRENT_LIST_DIR}/src)
include_directories(${CMAKE_CURRENT_LIST_DIR}/test)

#add sources
add_subdirectory(src)

#add tests
add_subdirectory(test)