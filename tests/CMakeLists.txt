enable_testing()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../build)

set(JSC_TEST_FILES "")
if(ENABLE_JAVASCRIPTCORE)
  file(GLOB JSC_TEST_FILES "jsc/*.test.cpp")
endif()

file(GLOB QJS_TEST_FILES "qjs/*.test.cpp")
file(GLOB QJS_GEAR_TEST_FILES "*.test.cpp")
add_executable(librime-qjs-tests ${QJS_TEST_FILES} ${QJS_GEAR_TEST_FILES} ${JSC_TEST_FILES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
  target_compile_definitions(librime-qjs-tests PRIVATE RIME_IMPORTS)
endif()

find_package(GTest REQUIRED)

target_link_libraries(librime-qjs-tests PUBLIC
  ${rime_library}
  qjs
  librime-qjs-objs
  GTest::gtest
  ${Marisa_LIBRARY}
)

if(ENABLE_JAVASCRIPTCORE)
  find_library(JAVASCRIPTCORE JavaScriptCore REQUIRED)
  include_directories(${JAVASCRIPTCORE}/Headers)
  target_link_libraries(librime-qjs-tests PRIVATE ${JAVASCRIPTCORE})
endif()

if(WIN32)
  file(GLOB rime_dll ${CMAKE_SOURCE_DIR}/dist/lib/*.dll)
  file(COPY ${rime_dll} DESTINATION ${EXECUTABLE_OUTPUT_PATH})
endif()

add_test(NAME librime-qjs-tests
  COMMAND librime-qjs-tests
  WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})


# set(benchmark_files "benchmark/dict/load_map_benchmark.cc")
# add_executable(load-dict-benchmark ${benchmark_files})
# target_link_libraries(load-dict-benchmark
#   librime-qjs-objs
#   ${rime_library}
#   ${rime_dict_library}
#   ${rime_gears_library}
#   ${GTEST_LIBRARIES}
# )
