file(READ "${TEMPLATE_DIR}/${TEMPLATE_NAME}.template" template)
set(output "#pragma once\n\nstatic const char* ${TEMPLATE_NAME} = R\"(${template})\";")
file(WRITE "${OUTPUT_DIR}/${TEMPLATE_NAME}.h" "${output}")
