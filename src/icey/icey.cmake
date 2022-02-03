add_executable("icey"
	"src/shared/ice.h"
	"src/icey/winlite.h"
	"src/icey/main.c"
)

add_dependencies("icey"
"libicey"
)

if(CMAKE_BUILD_TYPE STREQUAL x64_debug OR CMAKE_BUILD_TYPE STREQUAL x64_release)
set_target_properties("icey" PROPERTIES
  OUTPUT_NAME "icey"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/x64"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/x64"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/x64"
)
endif()

if(CMAKE_BUILD_TYPE STREQUAL x86_debug OR CMAKE_BUILD_TYPE STREQUAL x86_release)
set_target_properties("icey" PROPERTIES
  OUTPUT_NAME "icey"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/x86"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/x86"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/x86"
)
endif()

target_include_directories("icey" PRIVATE
  ${CMAKE_BINARY_DIR}/src/shared
)

target_compile_definitions("icey" PRIVATE
  _CRT_SECURE_NO_WARNINGS
  _SCL_SECURE_NO_WARNINGS
)

target_link_directories("icey" PRIVATE
  ${CMAKE_BINARY_DIR}/x64
)

target_link_libraries("icey"
  libicey
  libicey.lib
)

target_compile_options("icey" PRIVATE
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

add_custom_command(TARGET "icey" POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:icey>"
     "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/$<PLATFORM_ID>/$<TARGET_FILE_NAME:icey>")
