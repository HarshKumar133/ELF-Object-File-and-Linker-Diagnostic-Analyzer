add_executable(linkmap_tests phase1_tests.cpp)
target_link_libraries(linkmap_tests PRIVATE LinkMap::Core)
add_test(NAME linkmap_unit_tests COMMAND linkmap_tests)
add_test(NAME linkmap_help COMMAND linkmap help)
add_test(NAME linkmap_version COMMAND linkmap version)

find_package(Python3 COMPONENTS Interpreter QUIET)
find_program(LINKMAP_C_COMPILER NAMES clang)
if(Python3_Interpreter_FOUND AND LINKMAP_C_COMPILER)
  add_test(NAME linkmap_compiler_elf_integration
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/cli_tests.py
      $<TARGET_FILE:linkmap> ${LINKMAP_C_COMPILER}
      ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/source)
else()
  message(STATUS "Skipping compiler-generated ELF tests: Python 3 or clang is unavailable")
endif()
