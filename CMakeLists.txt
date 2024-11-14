cmake_minimum_required(VERSION 3.14)
project(logger CXX)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib)
add_subdirectory(src
  EXCLUDE_FROM_ALL
  )
