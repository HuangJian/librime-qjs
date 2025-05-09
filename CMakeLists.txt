# CMakeLists.txt

cmake_minimum_required(VERSION 3.12)

# Set the project name and version
project(LibrimeQjs VERSION 1.1.0)

option(BUILD_TOOLS "Build tools" ON)
option(BUILD_BENCHMARKS "Run benchmarks" OFF)
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

option(ENABLE_JAVASCRIPTCORE "Enable JavaScriptCore" OFF)
if(APPLE)
  set(ENABLE_JAVASCRIPTCORE ON)
  add_definitions(-D_ENABLE_JAVASCRIPTCORE)
endif()
message(STATUS "ENABLE_JAVASCRIPTCORE: ${ENABLE_JAVASCRIPTCORE}")

add_definitions(-DRIME_QJS_VERSION="${PROJECT_VERSION}")
message(STATUS "LibrimeQjs version: ${PROJECT_VERSION}") # to update the version in releases

# Specify C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Specify C11 standard for QuickJS
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Add _GNU_SOURCE definition for Boost.Stacktrace
add_definitions(-D_GNU_SOURCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

list(APPEND qjs_defines _GNU_SOURCE)
if(WIN32)
  # NB: Windows 7 is EOL and we are only supporting in so far as it doesn't interfere with progress.
  list(APPEND qjs_defines WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0601)
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/quickjs.h")
  message(FATAL_ERROR "QuickJS-NG source files not found. \nPlease clone it by `git submodule update --init --recursive` and try again.")
endif()

# QuickJS source files
set(QUICKJS_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/quickjs.c
  ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/libregexp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/libunicode.c
  ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/libbf.c
  ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/cutils.c
)

include_directories(include ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs)

# Create QuickJS library
add_library(qjs STATIC ${QUICKJS_SOURCES})

if(UNIX AND NOT APPLE)
  # Position-Independent Code (PIC) Required for later linking to librime-qjs (a shared library)
  set_target_properties(qjs PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

target_compile_definitions(qjs PRIVATE ${qjs_defines})
target_include_directories(qjs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs)

if(BUILD_TOOLS)
  # Create QuickJS executable
  add_executable(qjs_exe
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/gen/repl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/gen/standalone.c
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/qjs.c
  )
  set_target_properties(qjs_exe PROPERTIES
    OUTPUT_NAME "qjs"
  )
  target_compile_definitions(qjs_exe PRIVATE ${qjs_defines})
  if(APPLE)
    target_link_options(qjs_exe PRIVATE "-Wl,-force_load")
  else()
    target_link_options(qjs_exe PRIVATE "-Wl,--allow-multiple-definition")
  endif()

  # Create libraries
  add_library(node_module_loader STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/patch/quickjs/node_module_loader.c
  )
  target_compile_definitions(node_module_loader PRIVATE BUILD_FOR_QJS_EXE)

  add_library(quickjs_libc STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/quickjs-libc.c
  )

  # Add compile definition to rename the js_module_loader symbol, to replace it with the patched
  target_compile_definitions(quickjs_libc PRIVATE -Djs_module_loader=js_module_loader_default)

  # Link both libraries
  target_link_libraries(qjs_exe
    PRIVATE
    node_module_loader
    quickjs_libc
    qjs
  )

  if(UNIX AND NOT APPLE)
    # link the math library (libm) to your executable
    target_link_libraries(qjs_exe PRIVATE m)
  endif()

  if(NOT WIN32)
    set_target_properties(qjs_exe PROPERTIES ENABLE_EXPORTS TRUE)
  endif()
endif()

set(CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/list)
set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/engines
  ${CMAKE_CURRENT_SOURCE_DIR}/src/engines/quickjs
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gears
  ${CMAKE_CURRENT_SOURCE_DIR}/src/types
  ${CMAKE_CURRENT_SOURCE_DIR}/src/misc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/patch/quickjs
)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/gears SRC_GEARS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/dicts SRC_DICTS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/types SRC_TYPES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/patch/quickjs SRC_PATCHES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/engines SRC_ENGINES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/engines/quickjs SRC_ENGINE_QUICKJS)

set(SRC_ENGINE_JAVASCRIPTCORE "")
if(ENABLE_JAVASCRIPTCORE)
  aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/engines/javascriptcore SRC_ENGINE_JAVASCRIPTCORE)
endif()

# create a dynamic library
add_library(librime-qjs-objs OBJECT
  ${SRC}
  ${SRC_GEARS}
  ${SRC_DICTS}
  ${SRC_TYPES}
  ${SRC_PATCHES}
  ${SRC_ENGINES}
  ${SRC_ENGINE_QUICKJS}
  ${SRC_ENGINE_JAVASCRIPTCORE}
)

if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND APPLE)
  message(STATUS "Enabling LLVM code coverage instrumentation")
  target_compile_options(librime-qjs-objs PRIVATE
    -fprofile-instr-generate
    -fcoverage-mapping
    -fno-elide-constructors
    -fcoverage-mcdc
  )
  target_link_options(librime-qjs-objs PRIVATE
    -fprofile-instr-generate
    -fcoverage-mapping
    -fno-elide-constructors
    -fcoverage-mcdc
  )

  # file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/types/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/types/*.hpp")
  # message(STATUS "Header files: ${HEADER_FILES}")
  # set_source_files_properties(${HEADER_FILES}
  #   PROPERTIES
  #   LANGUAGE CXX
  #   COMPILE_FLAGS "-fprofile-instr-generate -fcoverage-mapping -fno-elide-constructors -fcoverage-mcdc"
  # )

  # Find the LLVM compiler runtime library path
  execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -print-runtime-dir
    OUTPUT_VARIABLE CLANG_RT_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if(CLANG_RT_PATH)
    message(STATUS "Found LLVM compiler runtime path: ${CLANG_RT_PATH}")
    set(CLANG_RT_PROFILE "${CLANG_RT_PATH}/libclang_rt.profile_osx.a")
  else()
    message(WARNING "Could not find LLVM compiler runtime path. Coverage may not work properly.")
    # Fallback to direct linking, which may still fail if the library is not in the search path
    set(CLANG_RT_PROFILE "clang_rt.profile_osx")
  endif()
endif()

if(BUILD_SHARED_LIBS)
  set_target_properties(librime-qjs-objs PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

if(WIN32)
  target_compile_definitions(librime-qjs-objs PRIVATE RIME_IMPORTS)
endif()

if(ENABLE_JAVASCRIPTCORE)
  find_library(JAVASCRIPTCORE JavaScriptCore REQUIRED)
  include_directories(${JAVASCRIPTCORE}/Headers)
  target_link_libraries(librime-qjs-objs PRIVATE ${JAVASCRIPTCORE})
endif()

# Link QuickJS to librime-qjs
target_link_libraries(librime-qjs-objs PRIVATE qjs)

if(BUILD_SHARED_LIBS)
  if(BUILD_TEST)
    add_subdirectory(tests)
  endif()
endif()

# cd <librime>
# cmake -S . -B build -DBUILD_BENCHMARKS=ON
# make && ./plugins/qjs/build/qjs-benchmark
if(BUILD_BENCHMARKS)
  add_subdirectory(benchmark)
endif()

set(plugin_name "rime-qjs" PARENT_SCOPE)
set(plugin_objs $<TARGET_OBJECTS:librime-qjs-objs> PARENT_SCOPE)
set(plugin_deps ${rime_library} ${rime_gears_library} qjs ${JAVASCRIPTCORE} ${CLANG_RT_PROFILE} PARENT_SCOPE)
set(plugin_modules "qjs" PARENT_SCOPE)
