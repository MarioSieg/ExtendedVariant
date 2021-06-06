cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("ExtendedVariant" CXX)
add_executable("ExtendedVariantTests" "tests.cpp")
target_compile_options("ExtendedVariantTests" PRIVATE "-Xclang -Wall -Xclang -Wextra -Xclang -Werror")