# Could use CMAKE_CURRENT_FUNCTION_LIST_DIR since 3.17, but for posterity
set(_THIS_MODULE_BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(create_source_package name)
    set(options "")
    set(oneValueArgs )
    set(multiValueArgs INCLUDE EXCLUDE)
    cmake_parse_arguments(
        _CRSRC
            "${options}"
            "${oneValueArgs}"
            "${multiValueArgs}"
            ${ARGN}
    )

    # Will regenerate if file added in depends directory, or if a depends file
    # has been changed, but, changes to files within a depends directory will
    # not.
    set(_CRSRC_INCLUDE_ABS ${_CRSRC_INCLUDE})
    list(TRANSFORM _CRSRC_INCLUDE_ABS PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)
    list(TRANSFORM _CRSRC_EXCLUDE REPLACE "(.+)" "--exclude=\"\\1\"")

    set(_CRSRC_ARCHIVE ${CMAKE_BINARY_DIR}/packages/${name}.src.tar.gz)
    add_custom_command(OUTPUT ${_CRSRC_ARCHIVE}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/packages
        COMMAND ${CMAKE_COMMAND} -E env 
            tar ${_CRSRC_EXCLUDE}
                --transform 's,^,${name}/,'
                -czvf ${_CRSRC_ARCHIVE}
                --
                ${_CRSRC_INCLUDE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${_CRSRC_INCLUDE_ABS}
        COMMAND_EXPAND_LISTS # Needed to properly process file list
    )
    add_custom_target(package_source_${name} DEPENDS ${_CRSRC_ARCHIVE})
    if (NOT TARGET package_sources)
        add_custom_target(package_sources)
    endif()
    add_dependencies(package_sources package_source_${name})
endfunction()