option(APSNHTTP_BUILD_DOC "Build documentation" ON)


if (APSNHTTP_BUILD_DOC)
    set(DOX_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/dox)
    create_doxygen_target(apsnhttp
        XML HTML
        INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../include
        VERSION ${PROJECT_VERSION}
        PROJECT_NAME "HTTP"
        OUTPUT_DIR "${DOX_OUTDIR}"
        MAIN_PAGE ${CMAKE_CURRENT_SOURCE_DIR}/../README.md
    )

    # include(GNUInstallDirs)
    # install(DIRECTORY "${DOX_OUTDIR}/html" DESTINATION /usr/share/doc/apsn/mvlhal
    #     COMPONENT mvlhaldev)

        
    # create_breathe_target(mvlhal
    #     PROJECT_NAME "Marvell PX HAL"
    #     COPYRIGHT "Copyright (c) 2022"
    #     AUTHOR "APS Networks bv"
    #     RELEASE 0.9.0
    #     DEFAULT_PROJECT "apsnmvlhal"
    #     DEPENDS index.rst
    # )
endif()

