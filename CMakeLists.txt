cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" ON)

enable_testing()

project(banking)

set(Sources
    banking/Account.cpp
    banking/Transaction.cpp
)
set(Headers
    banking/Account.h
    banking/Transaction.h
)

add_library(banking STATIC ${Sources} ${Headers})
target_include_directories(banking PUBLIC banking)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(third-party/gtest)
    add_executable(myTest tests/tests.cpp)
    target_link_libraries(myTest
        gtest_main
        gmock_main
        banking
    )
    add_test(NAME myTest COMMAND myTest)
endif()