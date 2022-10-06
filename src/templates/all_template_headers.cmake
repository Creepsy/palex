set(TEMPLATE_DIR ${CMAKE_SOURCE_DIR}/templates)

file(GLOB TEMPLATE_FILES ${TEMPLATE_DIR}/*)

foreach(template_file ${TEMPLATE_FILES})
    get_filename_component(name ${template_file} NAME_WLE)
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/${name}.h
        COMMAND ${CMAKE_COMMAND}
            -DOUTPUT_DIR=${CMAKE_BINARY_DIR}
            -DTEMPLATE_DIR=${TEMPLATE_DIR}
            -DTEMPLATE_NAME=${name}
            -P ${CMAKE_SOURCE_DIR}/src/templates/template_generator.cmake
        COMMENT "Generating ${CMAKE_BINARY_DIR}/${name}.h"
        DEPENDS ${TEMPLATE_DIR}/${name}.template
    )
endforeach()