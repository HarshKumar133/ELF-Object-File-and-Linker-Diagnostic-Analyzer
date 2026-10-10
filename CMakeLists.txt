cmake_minimum_required(VERSION 3.20)
project(LinkMap VERSION 0.1.0 LANGUAGES CXX)

include(CTest)
option(LINKMAP_BUILD_TESTS "Build LinkMap tests" ${BUILD_TESTING})

add_library(linkmap_core
  src/core.cpp
  src/checked_reader.cpp
  src/object_parser.cpp
  src/section_parser.cpp
  src/symbol_parser.cpp
  src/relocation_parser.cpp
  src/object_file.cpp
  src/analyzer.cpp
  src/report.cpp
)
add_library(LinkMap::Core ALIAS linkmap_core)
target_include_directories(linkmap_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(linkmap_core PUBLIC cxx_std_20)
target_compile_options(linkmap_core PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wsign-conversion>
)

add_executable(linkmap src/main.cpp)
target_link_libraries(linkmap PRIVATE LinkMap::Core)
target_compile_features(linkmap PRIVATE cxx_std_20)

if(LINKMAP_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
