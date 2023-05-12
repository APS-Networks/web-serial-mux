add_library(apsnhttp STATIC 
    src/handlers.cpp
    src/headers.cpp
    src/listener.cpp
    src/middleware.cpp
    src/request.cpp
    src/router.cpp
    src/session.cpp
    src/ssl.cpp
    # src/websocket.cpp
)
add_library(apsn::http ALIAS apsnhttp)

target_compile_features(apsnhttp PUBLIC cxx_std_23)

target_include_directories(apsnhttp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include/apsn/http
)


find_package(OpenSSL REQUIRED)
target_link_libraries(apsnhttp PUBLIC
    apsn::core
    Boost::asio
    Boost::beast
    Boost::program_options
    fmt::fmt
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    contrib::md5
)

if (WEBSERIAL_BUILD_DOCS)
    add_subdirectory(docs)
endif()
add_subdirectory(test)
