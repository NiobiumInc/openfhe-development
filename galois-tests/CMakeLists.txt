cmake_minimum_required(VERSION 3.14)
project(my_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_subdirectory(.. openfhe)

enable_testing()

add_executable(
  galois_tests
  src/hello_test.cpp
)

target_link_libraries(
  galois_tests
  PUBLIC
    GTest::gtest_main
    OPENFHEpke
    OPENFHEcore
    OPENFHEbinfhe
    
)

target_include_directories(
  galois_tests
  PUBLIC
    ../src/pke/include
    ../src/core/include
    ../src/binfhe/include
    ${CMAKE_CURRENT_BINARY_DIR}/openfhe/src/core
    ../third-party/cereal/include
)

include(GoogleTest)
gtest_discover_tests(galois_tests)
