add_library("libicey" STATIC
    "src/shared/ice.h"
    "src/libicey/ice.c"
)

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
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GCC>>:-g>
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/O2>
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GCC>>:-O2>
    $<$<AND:$<CONFIG:MinSizeRel>,$<CXX_COMPILER_ID:MSVC>>:/Os /Gr>
    $<$<AND:$<CONFIG:MinSizeRel>,$<CXX_COMPILER_ID:GCC>>:-Os -s>
)

add_custom_command(TARGET "libicey" POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:libicey>"
     "${PROJECT_SOURCE_DIR}/lib/$<CONFIGURATION>/$<PLATFORM_ID>/$<TARGET_FILE_NAME:libicey>")
