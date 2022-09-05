set(TEST_SOURCES
    TestReport.cpp
)

add_executable(util_test util_test.cpp ${TEST_SOURCES})
target_include_directories(util_test PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(util_test palex_objects)