cmake_minimum_required(VERSION 3.16)

if (NOT CMAKE_BUILD_TYPE)
   if(WIN32)
      set(CMAKE_BUILD_TYPE Debug)
   else()
	  set(CMAKE_BUILD_TYPE x64_debug)
   endif()
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(BUILD_TYPES "Debug" "Release" "RelWithDebInfo" "MinSizeRel" "x64_debug" "x86_debug" "x64_release" "x86_release")
get_property(multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(multi_config)
  set(CMAKE_CONFIGURATION_TYPES "${BUILD_TYPES}" CACHE STRING "list of supported configuration types" FORCE)
else()
  set(CMAKE_BUILD_TYPE "x64_debug" CACHE STRING "Build Type of the project.")
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${BUILD_TYPES}")
  if(NOT CMAKE_BUILD_TYPE IN_LIST BUILD_TYPES)
    message(FATAL_ERROR
      "Invalid build type '${CMAKE_BUILD_TYPE}'.
      CMAKE_BUILD_TYPE must be any one of the possible values:
      ${BUILD_TYPES}"
    )
  endif()
endif()

project("icey")

include(src/libicey/libicey.cmake)
include(src/icey/icey.cmake)

set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_STATIC_LIBRARY_PREFIX "")

