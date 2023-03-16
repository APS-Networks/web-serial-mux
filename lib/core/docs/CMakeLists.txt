option(APSNCORE_BUILD_DOC "Build documentation" ON)

if (APSNCORE_BUILD_DOC)
    set(DOX_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/dox)
    create_doxygen_target(core
        XML HTML
        INPUT 
            ${CMAKE_CURRENT_SOURCE_DIR}/../include
        VERSION ${PROJECT_VERSION}
        PROJECT_NAME "Core Library"
        OUTPUT_DIR "${DOX_OUTDIR}"
        MAIN_PAGE ${CMAKE_CURRENT_SOURCE_DIR}/../README.md
    )

    # include(GNUInstallDirs)
    install(DIRECTORY "${DOX_OUTDIR}/html" DESTINATION /usr/share/doc/apsn/core
        COMPONENT core)

endif()

