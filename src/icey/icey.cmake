add_executable("icey"
    "src/shared/ice.h"
    "src/icey/winlite.h"
    "src/icey/main.c"
)

add_dependencies("icey"
"libicey"
)

target_include_directories("icey" PRIVATE
  ${PROJECT_SOURCE_DIR}/src/shared
)

target_compile_definitions("icey" PRIVATE
  _CRT_SECURE_NO_WARNINGS
  _SCL_SECURE_NO_WARNINGS
)

if(WIN32)
target_sources("icey" PRIVATE "res/icey.rc")

target_link_libraries("icey"
  libicey
)
else()
target_link_libraries("icey" libicey)
endif()

target_compile_options("icey" PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GCC>>:-g>
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/O2>
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GCC>>:-O2>
    $<$<AND:$<CONFIG:MinSizeRel>,$<CXX_COMPILER_ID:MSVC>>:/Os /Gr>
    $<$<AND:$<CONFIG:MinSizeRel>,$<CXX_COMPILER_ID:GCC>>:-Os -s>
)

add_custom_command(TARGET "icey" POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:icey>"
     "${PROJECT_SOURCE_DIR}/bin/$<CONFIGURATION>/$<PLATFORM_ID>/$<TARGET_FILE_NAME:icey>")
