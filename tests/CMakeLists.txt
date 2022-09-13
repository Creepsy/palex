set(TEST_UTILS_SOURCES
    TestReport.cpp
)

add_library(test_utils ${TEST_UTILS_SOURCES})
target_include_directories(test_utils PRIVATE ${CMAKE_CURRENT_LIST_DIR})

add_executable(util_test util_test.cpp)
target_include_directories(util_test PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(util_test palex_objects test_utils)

add_executable(lexer_generator_test lexer_generator_test.cpp)
target_include_directories(lexer_generator_test PRIVATE ${CMAKE_CURRENT_LIST_DIR}, ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(lexer_generator_test palex_objects test_utils)