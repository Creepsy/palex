set(TEST_UTILS_SOURCES
    TestReport.cpp
)

set(TEST_UTILS_HEADERS
    test_utils.h
    TestReport.h
)

add_library(test_utils ${TEST_UTILS_SOURCES} ${TEST_UTILS_HEADERS})
target_include_directories(test_utils PRIVATE ${CMAKE_CURRENT_LIST_DIR})

function(generate_test_executable test_name)
    add_executable(${test_name} ${test_name}.cpp)
    target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${test_name} palex_objects test_utils)    
endfunction()

generate_test_executable(util_test)
generate_test_executable(lexer_generator_test)
generate_test_executable(regex_test)
generate_test_executable(templates_test)