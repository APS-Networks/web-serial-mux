add_library(webserial STATIC 
    src/cli/base_state.cpp
    src/cli/control_state.cpp
    src/cli/serial_state.cpp
    src/context.cpp
    src/error.cpp
    src/history.cpp
    src/logo.cpp
    src/port.cpp
    src/serial.cpp
    src/strings.cpp
    src/utility.cpp
    # smux/websocket.cpp
    src/cli_handler.cpp
    )

# target_precompile_headers(smux PUBLIC src/websocket.hpp)

add_library(apsn::webserial ALIAS webserial)
target_include_directories(webserial 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_compile_options(webserial PRIVATE -Wno-unused-parameter)
target_link_libraries(webserial PUBLIC 
    apsn::core
    apsn::http
    cli::cli
    fmt::fmt
    nlohmann_json::nlohmann_json
    contrib::md5
    )
