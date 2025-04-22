cmake_minimum_required(VERSION 3.13)
project(DynamicConnectivity VERSION 1.0 DESCRIPTION "Dynamic Connectivity" LANGUAGES CXX)

# Set a default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (Debug Release RelWithDebInfo MinSizeRel)" FORCE)
  message(STATUS "No build type specified. Defaulted to RELEASE.")
  message(STATUS "To specify a build type, add -DCMAKE_BUILD_TYPE=<DEBUG/RELEASE/RELWITHDEBINFO/MINSIZEREL>")
endif(NOT CMAKE_BUILD_TYPE)

# clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# gprof
# set (CMAKE_CXX_FLAGS_RELEASE "-O3 -g -pg")
message(STATUS "--------------- General configuration -------------")
message(STATUS "CMake Generator:                ${CMAKE_GENERATOR}")
message(STATUS "Compiler:                       ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build type:                     ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_FLAGS:                ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG:          ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE:        ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS          ${CMAKE_CXX_LINKER_FLAGS}")
message(STATUS "CMAKE_INSTALL_PREFIX:           ${CMAKE_INSTALL_PREFIX}" )
message(STATUS "PROJECT_SOURCE_DIR              ${DynamicConnectivity_SOURCE_DIR}")

# -------------------------------------------------------------------
#                       Library definition

add_library(dycon INTERFACE)
set(DYCON_INCLUDE_DIR "${DynamicConnectivity_SOURCE_DIR}/include/")
target_include_directories(dycon INTERFACE ${DYCON_INCLUDE_DIR})
target_compile_features(dycon INTERFACE cxx_std_17)
target_compile_options(dycon INTERFACE -mcx16 -march=native)

# Find threading library
find_package(Threads REQUIRED)

# Link against ParlayLib
#
# If the option -DDOWNLOAD_PARLAY is set to true, then we will pull a fresh
# copy of Parlay and use that. Otherwise, we will look for an installed version
# of Parlay on the system.
#
option(DOWNLOAD_PARLAY "Download and use a local copy of Parlay rather than looking for an installed version")
if(DOWNLOAD_PARLAY)
  message(STATUS "Downloading a local copy of ParlayLib")
  # Download and configure ParlayLib
  include(FetchContent)
  FetchContent_Declare(parlaylib
    GIT_REPOSITORY  https://github.com/cmuparlay/parlaylib.git
    GIT_TAG         master
  )
  FetchContent_GetProperties(parlaylib)
  if(NOT parlaylib_POPULATED)
    FetchContent_Populate(parlaylib)
    add_subdirectory(${parlaylib_SOURCE_DIR} EXCLUDE_FROM_ALL)
  endif()
  target_link_libraries(dycon INTERFACE parlay)
else()
  find_package(Parlay REQUIRED)
  if(Parlay_FOUND)
    message(STATUS "Parlay configuration found at " ${Parlay_DIR})
    target_link_libraries(dycon INTERFACE Parlay::parlay)
  else()
    message(FATAL_ERROR "ParlayLib is required but could not be found. Either install it, or set the option -DDOWNLOAD_PARLAY=True to download a local copy of Parlay to use")
  endif()
endif()

option(DOWNLOAD_PAM "Download and use a local copy of PAM rather than looking for an installed version")
if(DOWNLOAD_PAM)
  message(STATUS "Downloading a local copy of PAM")
  # Download and configure PAM
  include(FetchContent)
  FetchContent_Declare(pam
    GIT_REPOSITORY  https://github.com/cmuparlay/PAM.git
    GIT_TAG         master
  )
  FetchContent_GetProperties(pam)
  if(NOT pam_POPULATED)
    FetchContent_Populate(pam)
    add_subdirectory(${pam_SOURCE_DIR} EXCLUDE_FROM_ALL)
  endif()
  target_link_libraries(dycon INTERFACE pam)
else()
  find_package(pam REQUIRED)
  target_link_libraries(dycon INTERFACE pam)
endif()


FetchContent_Declare(abseil-cpp
GIT_REPOSITORY git@github.com:abseil/abseil-cpp.git
GIT_TAG master
)
FetchContent_MakeAvailable(abseil-cpp)
target_link_libraries(dycon INTERFACE absl::flat_hash_map absl::flat_hash_set)

# Link against jemalloc
# find_library(JEMALLOC_LIB jemalloc)
# if(NOT JEMALLOC_LIB)
#  message(FATAL_ERROR "Could not find jemalloc.")
# else()
#  message("-- Found jemalloc: ${JEMALLOC_LIB}")
#  target_link_libraries(dycon INTERFACE optimized ${JEMALLOC_LIB})
# endif()

if (JEMALLOC)
  set(JEMALLOC_LIB_DIR  "/home/zhongqi/local/lib")
  set(JEMALLOC_INCLUDE_DIR "/home/zhongqi/local/include")
#   include_directories(${JEMALLOC_INCLUDE_DIR})
#   target_link_libraries(dycon PRIVATE "${JEMALLOC_LIB_DIR}/libjemalloc.so")
endif()
# -------------------------------------------------------------------
#                         Unit tests

# enable_testing()
add_subdirectory(tests)

# -------------------------------------------------------------------
#                       Benchmarks

add_subdirectory(benchmark)

# -------------------------------------------------------------------
#                       Examples
add_executable(example example.cc)
target_link_libraries(example PRIVATE dycon)

#add_subdirectory(examples)

# -------------------------------------------------------------------
#                       utils

#add_subdirectory(utils)

