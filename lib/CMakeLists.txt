add_library(logger STATIC
    logger.cpp
)

target_compile_options(logger PRIVATE -Wall -Wextra)

target_include_directories(logger PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(logger PROPERTIES LINKER_LANGUAGE CXX)

