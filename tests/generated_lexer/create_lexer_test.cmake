function (create_lexer_test_dependencies TEST_NAME TEST_SRC PALEX_RULEFILE ADDITIONAL_FLAGS)
    if ( TARGET ${TEST_NAME})
        return()
    endif()
    get_filename_component(NAME ${PALEX_RULEFILE} NAME_WLE)
    set(LEXER_FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${NAME}Lexer.h
        ${CMAKE_CURRENT_BINARY_DIR}/${NAME}Lexer.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/${NAME}Token.h
        ${CMAKE_CURRENT_BINARY_DIR}/${NAME}Token.cpp
    )
    add_custom_command(
        OUTPUT ${LEXER_FILES}
        COMMAND ${CMAKE_BINARY_DIR}/lexergen ${PALEX_RULEFILE} -lang c++ -output-path ${CMAKE_CURRENT_BINARY_DIR} --lexer ${ADDITIONAL_FLAGS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating ${LEXER_FILES}"
        DEPENDS ${PALEX_RULEFILE}
    )
    add_executable(${TEST_NAME} ${TEST_SRC} ${LEXER_FILES})
    target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/util ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(${TEST_NAME} palex_objects lexer_autogen)
endfunction()

function(create_lexer_output_test TEST_NAME TEST_SRC PALEX_RULEFILE ADDITIONAL_FLAGS TEST_OUTPUT)
    create_lexer_test_dependencies(${TEST_NAME} ${TEST_SRC} ${PALEX_RULEFILE} ${ADDITIONAL_FLAGS})
    add_test(NAME "${TEST_NAME}Output" COMMAND bash -c "${CMAKE_BINARY_DIR}/${TEST_NAME} | diff ${TEST_OUTPUT} -") 
endfunction()

function(create_lexer_test TEST_NAME TEST_SRC PALEX_RULEFILE ADDITIONAL_FLAGS)
    create_lexer_test_dependencies(${TEST_NAME} ${TEST_SRC} ${PALEX_RULEFILE} ${ADDITIONAL_FLAGS})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME}) 
endfunction()