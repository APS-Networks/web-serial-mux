

add_executable(webserial_main
    main.cpp
    options.cpp)

find_package(OpenSSL REQUIRED)
target_compile_features(webserial_main PRIVATE cxx_std_23)
target_link_libraries(webserial_main PRIVATE 
    apsn::core
    apsn::http
    apsn::webserial
    Boost::program_options
    nlohmann_json::nlohmann_json
    contrib::md5
    )

install(TARGETS webserial_main
    RUNTIME DESTINATION /opt/apsn/webserial_main
)
set_target_properties(webserial_main PROPERTIES OUTPUT_NAME webserial)


add_executable(wspasswd wspasswd.cpp)
target_compile_features(wspasswd PRIVATE cxx_std_23)
target_link_libraries(wspasswd PRIVATE 
    apsn::core
    Boost::program_options
    contrib::md5
    fmt::fmt
)