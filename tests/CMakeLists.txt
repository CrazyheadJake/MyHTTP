# Unit tests
add_executable(unit_tests
    TestParser.cpp
    # Add other unit test files here
)
target_link_libraries(unit_tests PRIVATE MyHTTP gtest gtest_main pthread)
add_test(NAME UnitTests COMMAND unit_tests)

# Optional integration tests
add_executable(integration_tests
    TestIntegration.cpp
)
target_link_libraries(integration_tests PRIVATE MyHTTP gtest gtest_main pthread)
add_test(NAME IntegrationTests COMMAND integration_tests)
