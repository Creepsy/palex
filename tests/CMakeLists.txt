set(TEST_UTILS_SOURCES
    TestReport.cpp
)

set(TEST_UTILS_HEADERS
    test_utils.h
    TestReport.h
)

add_library(test_utils ${TEST_UTILS_SOURCES} ${TEST_UTILS_HEADERS})
target_include_directories(test_utils PRIVATE ${CMAKE_CURRENT_LIST_DIR})

add_executable(util_test util_test.cpp)
target_include_directories(util_test PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(util_test palex_objects test_utils)

add_executable(lexer_generator_test lexer_generator_test.cpp)
target_include_directories(lexer_generator_test PRIVATE ${CMAKE_CURRENT_LIST_DIR}, ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(lexer_generator_test palex_objects test_utils)

add_executable(regex_test regex_test.cpp)
target_include_directories(regex_test PRIVATE ${CMAKE_CURRENT_LIST_DIR}, ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(regex_test palex_objects test_utils)