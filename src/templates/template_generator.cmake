file(READ "${TEMPLATE_DIR}/${TEMPLATE_NAME}.tpl" template)
set(output "#pragma once\n\nconst char* ${TEMPLATE_NAME} = R\"(${template})\";")
file(WRITE "${OUTPUT_DIR}/${TEMPLATE_NAME}.h" "${output}")
