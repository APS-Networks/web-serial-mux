# TODO Custom or different out directory for status files
function(create_application_fixture)
    set(options SUDO)
    set(oneValueArgs TARGET SIGNAL FIXTURE)
    set(multiValueArgs ARGUMENTS)
    cmake_parse_arguments(
        _APPFIX
            "${options}"
            "${oneValueArgs}"
            "${multiValueArgs}"
            ${ARGN}
    )

    # TODO: Signal

    add_test(NAME fixture_${_APPFIX_TARGET}_launch
        COMMAND ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scripts/application_launch.sh
            $<$<BOOL:_APPFIX_SUDO>:--sudo>
            $<TARGET_FILE:${_APPFIX_TARGET}>
            -- 
            "$<JOIN:${_APPFIX_ARGUMENTS}, >"
    )
    add_test(NAME fixture_${_APPFIX_TARGET}_kill
        COMMAND ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scripts/application_kill.sh
            $<$<BOOL:_APPFIX_SUDO>:--sudo>
            $<TARGET_FILE:gearboxd>
    )

    if (_APPFIX_FIXTURE)
        set_tests_properties(fixture_${_APPFIX_TARGET}_launch
            PROPERTIES
                FIXTURES_SETUP ${_APPFIX_FIXTURE}
        )
        set_tests_properties(fixture_${_APPFIX_TARGET}_kill
            PROPERTIES
                FIXTURES_CLEANUP ${_APPFIX_FIXTURE}
        )
    endif()
endfunction ()
