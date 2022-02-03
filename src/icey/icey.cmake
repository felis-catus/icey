add_executable("icey"
	"src/shared/icey/ice.h"
	"src/icey/winlite.h"
	"src/icey/main.c"
)

add_dependencies("icey"
"libicey"
)

set_target_properties("icey" PROPERTIES
  OUTPUT_NAME "icey"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/icey"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/icey"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/icey"
)

target_include_directories("icey" PRIVATE
  ${CMAKE_BINARY_DIR}/src/shared
)

target_compile_definitions("icey" PRIVATE
  _CRT_SECURE_NO_WARNINGS
  _SCL_SECURE_NO_WARNINGS
)

target_link_directories("icey" PRIVATE
  $<$<CONFIG:x64_debug>:${CMAKE_BINARY_DIR}/src/lib/shared/x64/debug>
  $<$<CONFIG:x86_debug>:${CMAKE_BINARY_DIR}/src/lib/shared/x86/debug>
  $<$<CONFIG:x64_release>:${CMAKE_BINARY_DIR}/src/lib/shared/x64/release>
  $<$<CONFIG:x86_release>:${CMAKE_BINARY_DIR}/src/lib/shared/x86/release>
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
