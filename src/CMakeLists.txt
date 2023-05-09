add_executable(test_logger
    main.cpp
)

target_link_libraries(test_logger logger)

set_target_properties(test_logger
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

