# Clang-Tidy
option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)
if(ENABLE_CLANG_TIDY)
  message(STATUS "Clang-tidy enabled.")
  set(CMAKE_VERBOSE_MAKEFILE ON)
  set(CMAKE_BUILD_TYPE Debug)
  find_program(CLANGTIDY clang-tidy)
  if(CLANGTIDY)
    set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY}
                             -extra-arg=-Wno-unknown-warning-option)
  else()
    message(SEND_ERROR "clang-tidy requested but executable not found")
  endif()
endif()

# CppCheck
option(ENABLE_CPPCHECK "Enable static analysis with cppcheck" OFF)
if(ENABLE_CPPCHECK)
  message(STATUS "CppCheck enabled.")
  set(CMAKE_BUILD_TYPE Debug)
  find_program(CPPCHECK cppcheck)
  if(CPPCHECK)
    set(CMAKE_CXX_CPPCHECK ${CPPCHECK} --suppress=missingInclude --enable=all
                           --inconclusive --force --verbose)
  else()
    message(SEND_ERROR "cppcheck requested but executable not found")
  endif()
endif()
