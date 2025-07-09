add_executable(test_logger
    main.cpp
)

target_link_libraries(test_logger logger)
#target_compile_definitions(test_logger PUBLIC LOGGER_ASSERT_THROW)

set_target_properties(test_logger
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

