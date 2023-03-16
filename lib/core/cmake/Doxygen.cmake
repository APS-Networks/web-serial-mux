


function(create_doxygen_target name)
    set(options XML HTML LATEX)
    set(oneValueArgs PRODUCT TEMPLATE PROJECT_NAME OUTPUT_DIR MAIN_PAGE VERSION)
    set(multiValueArgs INPUT)
    cmake_parse_arguments(
        _CRDOX
            "${options}"
            "${oneValueArgs}"
            "${multiValueArgs}"
            ${ARGN}
    )
    include(FindDoxygen)
    find_package(Doxygen REQUIRED)

    if (DOXYGEN_FOUND)
        # set input and output files
        if (_CRDOX_TEMPLATE)
            set(DOXYGEN_IN ${_CRDOX_TEMPLATE})
        else(_CRDOX_TEMPLATE)
            set(DOXYGEN_IN ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/resources/Doxyfile.in)
        endif(_CRDOX_TEMPLATE)

        if (_CRDOX_XML)
            set(DOXYGEN_CONFIGURE_XML YES)
        else(_CRDOX_XML)
            set(DOXYGEN_CONFIGURE_XML NO)
        endif(_CRDOX_XML)

        if (_CRDOX_LATEX)
            find_package(pdflatex REQUIRED)
            set(DOXYGEN_CONFIGURE_LATEX YES)
        else(_CRDOX_LATEX)
            set(DOXYGEN_CONFIGURE_LATEX NO)
        endif(_CRDOX_LATEX)

        if (_CRDOX_PRODUCT)
            set(HTML_CONFIGURE_PRODUCT ${_CRDOX_PRODUCT})
        else(_CRDOX_PRODUCT)
            set(HTML_CONFIGURE_PRODUCT "")
        endif(_CRDOX_PRODUCT)

        if (_CRDOX_VERSION)
            find_package(pdflatex REQUIRED)
            set(DOXYGEN_CONFIGURE_VERSION ${_CRDOX_VERSION})
        else(_CRDOX_VERSION)
            set(DOXYGEN_CONFIGURE_VERSION "")
        endif(_CRDOX_VERSION)

        if (_CRDOX_HTML)
            set(DOXYGEN_CONFIGURE_HTML YES)
        else(_CRDOX_HTML)
            set(DOXYGEN_CONFIGURE_HTML NO)
        endif(_CRDOX_HTML)

        if (_CRDOX_OUTPUT_DIR)
            set(DOXYGEN_CONFIGURE_OUTPUT_DIR ${_CRDOX_OUTPUT_DIR})
        else(_CRDOX_OUTPUT_DIR)
            set(DOXYGEN_CONFIGURE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
        endif(_CRDOX_OUTPUT_DIR)

        set(DOXYGEN_CONFIGURE_INPUT ${_CRDOX_INPUT})
        # set(DOXYGEN_CONFIGURE_INPUT blah)


        if (_CRDOX_MAIN_PAGE)
            get_filename_component(DOXYGEN_CONFIGURE_MAIN_PAGE
                ${_CRDOX_MAIN_PAGE} NAME 
            )
            set(DOXYGEN_CONFIGURE_INPUT "${DOXYGEN_CONFIGURE_INPUT} ${_CRDOX_MAIN_PAGE}")
        else(_CRDOX_MAIN_PAGE)
            set(DOXYGEN_CONFIGURE_MAIN_PAGE "")
        endif(_CRDOX_MAIN_PAGE) 

        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        set(DOXYGEN_CONFIGURE_PROJECT_NAME ${_CRDOX_PROJECT_NAME})

        # message(DOXYGEN_IN: ${DOXYGEN_IN})
        # message(DOXYGEN_CONFIGURE_XML: ${DOXYGEN_CONFIGURE_XML})
        # message(DOXYGEN_CONFIGURE_LATEX: ${DOXYGEN_CONFIGURE_LATEX})
        # message(DOXYGEN_CONFIGURE_HTML: ${DOXYGEN_CONFIGURE_HTML})
        # message(DOXYGEN_CONFIGURE_OUTPUT_DIR: ${DOXYGEN_CONFIGURE_OUTPUT_DIR})

        file(COPY ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/resources/logo.png
            DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

        include(FetchContent)
        FetchContent_Declare(doxawesome
            GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css.git
            GIT_TAG main
        )
        FetchContent_MakeAvailable(doxawesome)

        file(COPY ${doxawesome_SOURCE_DIR}/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/doxawesome)
        file(COPY ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/resources/DoxygenLayout.xml
            DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

        file(COPY ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/resources/_static/Exo2-VariableFont_wght.ttf
            DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        file(COPY ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/resources/_static/OxygenMono-Regular.ttf
            DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        file(COPY ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/resources/_static/mermaid.min.js
            DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        file(COPY ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/resources/mermaid-md.sed
            DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

        configure_file(${CMAKE_CURRENT_FUNCTION_LIST_DIR}/resources/header.html
            ${CMAKE_CURRENT_BINARY_DIR}/header.html @ONLY)

        # TODO Make a command then target to enable dependency between this and
        #      breathe documentation.
        # note the option ALL which allows to build the docs together with the application
        add_custom_target(doxygen_${name} ALL
            COMMAND ${CMAKE_COMMAND} -E copy 
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/resources/custom.css
                ${CMAKE_CURRENT_BINARY_DIR}
            COMMAND ${DOXYGEN_EXECUTABLE} -d extcmd ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation for ${name} with Doxygen"
            DEPENDS
                ${DOXYGEN_IN}
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/resources/custom.css
            VERBATIM)

        if (_CRDOX_LATEX)
            add_custom_target(doxygen_${name}_latex ALL
                COMMAND make
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dox/latex
                COMMENT "Generating Latex API documentation for ${name} with make"
                DEPENDS ${DOXYGEN_IN}
                VERBATIM )
        endif()

        if (NOT TARGET doxygen)
            add_custom_target(doxygen)
        endif()

        add_dependencies(doxygen doxygen_${name})
    else (DOXYGEN_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)
endfunction()


function(create_breathe_target name)
    set(options  "")
    set(oneValueArgs
        PROJECT_NAME
        COPYRIGHT
        AUTHOR
        RELEASE
        DEFAULT_PROJECT
        SPHINX_TEMPLATE)
    set(multiValueArgs "")
    cmake_parse_arguments(
        _CRBRTHE
            "${options}"
            "${oneValueArgs}"
            "${multiValueArgs}"
            ${ARGN}
    )

    include(FindSphinx)

    # include(${CMAKE_CURRENT_FUNCTION_LIST_DIR}/FindSphinx.cmake)
    find_package(Sphinx REQUIRED)

    if (_CRBRTHE_SPHINX_TEMPLATE)
        set(SPHINX_CONF_IN ${_CRBRTHE_SPHINX_TEMPLATE})
    else(_CRBRTHE_SPHINX_TEMPLATE)
        set(SPHINX_CONF_IN ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/resources/conf.py.in)
    endif(_CRBRTHE_SPHINX_TEMPLATE)

    set(SPHINX_CONFIGURE_PROJECT_NAME ${_CRBRTHE_PROJECT_NAME})
    set(SPHINX_CONFIGURE_COPYRIGHT ${_CRBRTHE_COPYRIGHT})
    set(SPHINX_CONFIGURE_AUTHOR ${_CRBRTHE_AUTHOR})
    set(SPHINX_CONFIGURE_RELEASE ${_CRBRTHE_RELEASE})
    set(SPHINX_CONFIGURE_DEFAULT_PROJECT ${_CRBRTHE_DEFAULT_PROJECT})
    set(SPHINX_CONF_OUT ${CMAKE_CURRENT_BINARY_DIR}/conf.py)

    # message(SPHINX_CONF_IN: ${SPHINX_CONF_IN})
    # message(SPHINX_CONF_OUT: ${SPHINX_CONF_OUT})

    # message(SPHINX_CONFIGURE_PROJECT_NAME: ${SPHINX_CONFIGURE_PROJECT_NAME})
    # message(SPHINX_CONFIGURE_COPYRIGHT: ${SPHINX_CONFIGURE_COPYRIGHT})
    # message(SPHINX_CONFIGURE_AUTHOR: ${SPHINX_CONFIGURE_AUTHOR})
    # message(SPHINX_CONFIGURE_RELEASE: ${SPHINX_CONFIGURE_RELEASE})
    # message(SPHINX_CONFIGURE_DEFAULT_PROJECT: ${SPHINX_CONFIGURE_DEFAULT_PROJECT})


    configure_file(${SPHINX_CONF_IN} ${SPHINX_CONF_OUT} @ONLY)

    set(CSS_IN_DEPENDS
        ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/resources/_static/css/blockquote.css
        ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/resources/_static/css/custom.css                        
        ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/resources/_static/css/font.css
    )
    set(CSS_OUT_DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/_static/css/blockquote.css
        ${CMAKE_CURRENT_BINARY_DIR}/_static/css/custom.css
        ${CMAKE_CURRENT_BINARY_DIR}/_static/css/font.css
    )
    add_custom_command(OUTPUT ${CSS_OUT_DEPENDS}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/resources/_static
            ${CMAKE_CURRENT_BINARY_DIR}/_static
        DEPENDS ${CSS_IN_DEPENDS}
    )

    file(COPY ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/resources/Makefile
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    set(SPHINX_SOURCE ${CMAKE_CURRENT_BINARY_DIR})
    set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)
    set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

    set(DOXYGEN_INDEX_FILE ${CMAKE_CURRENT_BINARY_DIR}/dox/xml/index.xml)

    add_custom_command(
        OUTPUT 
            ${SPHINX_INDEX_FILE}
            ${CMAKE_CURRENT_BINARY_DIR}/copy.stamp
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND cmake -E touch ${PROJECT_BINARY_DIR}/copy.stamp
        COMMAND 
            ${SPHINX_EXECUTABLE} -b html
                -Dbreathe_projects.${_CRBRTHE_DEFAULT_PROJECT}=${CMAKE_CURRENT_BINARY_DIR}/dox/xml
                ${SPHINX_SOURCE} ${SPHINX_BUILD}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS
                ${CMAKE_CURRENT_SOURCE_DIR}/index.rst
                ${CSS_OUT_DEPENDS}
                ${DOXYGEN_INDEX_FILE}
                ${SPHINX_SOURCE}/conf.py
        COMMENT "Generating documentation with Sphinx")


    add_custom_target(breathe_${name}
        DEPENDS ${SPHINX_INDEX_FILE} doxygen_${name} ${PROJECT_BINARY_DIR}/copy.stamp)

endfunction()
