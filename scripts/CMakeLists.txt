set(CERT_CREATE_STAMP ${CMAKE_CURRENT_BINARY_DIR}/certs-create.stamp)

add_custom_command(
    OUTPUT ${CERT_CREATE_STAMP}
    COMMAND ./generate_keys.sh
    COMMAND cmake -E touch ${CERT_CREATE_STAMP}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(cert_create DEPENDS ${CERT_CREATE_STAMP})