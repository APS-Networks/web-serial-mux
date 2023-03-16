
set(NPM_INSTALL_STAMP ${CMAKE_CURRENT_BINARY_DIR}/npm-install.stamp)
add_custom_command(
    OUTPUT ${NPM_INSTALL_STAMP}
    COMMAND npm install
    COMMAND cmake -E touch ${NPM_INSTALL_STAMP}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(npm_install DEPENDS ${NPM_INSTALL_STAMP})

add_custom_target(npm_build 
    DEPENDS ${NPM_INSTALL_STAMP}
    COMMAND npm run build
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

include(GNUInstallDirs)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist
    DESTINATION /opt/apsn/webserial/html
)