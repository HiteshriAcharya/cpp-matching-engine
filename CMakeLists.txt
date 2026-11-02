cmake_minimum_required(VERSION 3.16)

project(cpp_matching_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(order_book_tests
    tests/test_order_book.cpp
)

target_include_directories(order_book_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)