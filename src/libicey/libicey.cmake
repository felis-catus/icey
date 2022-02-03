add_library("libicey" STATIC
    "src/shared/icey/ice.h"
    "src/libicey/ice.c"
)

if(CMAKE_BUILD_TYPE STREQUAL x64_debug)
  set_target_properties("libicey" PROPERTIES
    OUTPUT_NAME "libicey"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/lib/shared/x64/debug"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/lib/shared/x64/debug"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/lib/shared/x64/debug"
  )
endif()

if(CMAKE_BUILD_TYPE STREQUAL x86_debug)
  set_target_properties("libicey" PROPERTIES
    OUTPUT_NAME "libicey"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/lib/shared/x86/debug"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/lib/shared/x86/debug"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/lib/shared/x86/debug"
  )
endif()

if(CMAKE_BUILD_TYPE STREQUAL x64_release)
  set_target_properties("libicey" PROPERTIES
    OUTPUT_NAME "libicey"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/lib/shared/x64/release"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/lib/shared/x64/release"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/lib/shared/x64/release"
  )
endif()

if(CMAKE_BUILD_TYPE STREQUAL x86_release)
  set_target_properties("libicey" PROPERTIES
    OUTPUT_NAME "libicey"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/lib/shared/x86/release"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/lib/shared/x86/release"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/lib/shared/x86/release"
  )
endif()

target_include_directories("libicey" PRIVATE
  ${CMAKE_BINARY_DIR}/src/shared
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
