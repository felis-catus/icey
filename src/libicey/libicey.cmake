add_library("libicey" STATIC
    "src/shared/ice.h"
    "src/libicey/ice.c"
)

if(CMAKE_BUILD_TYPE STREQUAL x64_debug OR CMAKE_BUILD_TYPE STREQUAL x64_release)
set_target_properties("libicey" PROPERTIES
  OUTPUT_NAME "libicey"
  ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/x64"
  LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/x64"
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/x64"
  PREFIX ""
)
endif()

if(CMAKE_BUILD_TYPE STREQUAL x86_debug OR CMAKE_BUILD_TYPE STREQUAL x86_release)
set_target_properties("libicey" PROPERTIES
  OUTPUT_NAME "libicey"
  ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/x86"
  LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/x86"
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/x86"
  PREFIX ""
)
endif()

target_include_directories("libicey" PRIVATE
  ${PROJECT_SOURCE_DIR}/src/shared
)

target_compile_definitions("libicey" PRIVATE
  _CRT_SECURE_NO_WARNINGS
)

target_link_directories("libicey" PRIVATE
)

target_link_libraries("libicey"
)

target_compile_options("libicey" PRIVATE
  $<$<AND:$<CONFIG:x64_debug>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:x64_debug>,$<COMPILE_LANGUAGE:C>>:-g>
  $<$<AND:$<CONFIG:x64_debug>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:x64_debug>,$<COMPILE_LANGUAGE:CXX>>:-g>
  
  $<$<AND:$<CONFIG:x86_debug>,$<COMPILE_LANGUAGE:C>>:-m32>
  $<$<AND:$<CONFIG:x86_debug>,$<COMPILE_LANGUAGE:C>>:-g>
  $<$<AND:$<CONFIG:x86_debug>,$<COMPILE_LANGUAGE:CXX>>:-m32>
  $<$<AND:$<CONFIG:x86_debug>,$<COMPILE_LANGUAGE:CXX>>:-g>
  
  $<$<AND:$<CONFIG:x64_release>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:x64_release>,$<COMPILE_LANGUAGE:C>>:-O2>
  $<$<AND:$<CONFIG:x64_release>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:x64_release>,$<COMPILE_LANGUAGE:CXX>>:-O2>
  
  $<$<AND:$<CONFIG:x86_release>,$<COMPILE_LANGUAGE:C>>:-m32>
  $<$<AND:$<CONFIG:x86_release>,$<COMPILE_LANGUAGE:C>>:-O2>
  $<$<AND:$<CONFIG:x86_release>,$<COMPILE_LANGUAGE:CXX>>:-m32>
  $<$<AND:$<CONFIG:x86_release>,$<COMPILE_LANGUAGE:CXX>>:-O2>
)

add_custom_command(TARGET "libicey" POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:libicey>"
     "${PROJECT_SOURCE_DIR}/lib/$<CONFIGURATION>/$<PLATFORM_ID>/$<TARGET_FILE_NAME:libicey>")
