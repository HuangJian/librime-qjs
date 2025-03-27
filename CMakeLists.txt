# CMakeLists.txt

cmake_minimum_required(VERSION 3.12)

# Set the project name and version
project(LibrimeQjs VERSION 0.1)

option(BUILD_TOOLS "Build tools" ON)

add_definitions(-DRIME_QJS_VERSION="${PROJECT_VERSION}")

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

if (BUILD_TOOLS)
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

set(plugin_name "rime-qjs" PARENT_SCOPE)
set(plugin_objs $<TARGET_OBJECTS:librime-qjs-objs> PARENT_SCOPE)
set(plugin_deps ${rime_library} ${rime_gears_library} qjs PARENT_SCOPE)
set(plugin_modules "qjs" PARENT_SCOPE)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/gears SRC_GEARS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/types SRC_TYPES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/helpers SRC_HELPERS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/patch/quickjs SRC_PATCHES)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/yas/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gears
    ${CMAKE_CURRENT_SOURCE_DIR}/src/types
    ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/patch/quickjs
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/
)

# create a dynamic library
add_library(librime-qjs-objs OBJECT ${SRC} ${SRC_GEARS} ${SRC_TYPES} ${SRC_HELPERS} ${SRC_PATCHES})
if(BUILD_SHARED_LIBS)
  set_target_properties(librime-qjs-objs PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

if(WIN32)
  target_compile_definitions(librime-qjs-objs PRIVATE RIME_IMPORTS)
endif()

# Link QuickJS to librime-qjs
target_link_libraries(librime-qjs-objs PRIVATE qjs)

if(BUILD_SHARED_LIBS)
  if(BUILD_TEST)
    add_subdirectory(tests)
  endif()
endif()
