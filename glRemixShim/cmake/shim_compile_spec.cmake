# reuseable for shim target and intellisense helper target

function(set_shim_compile_specifications proj_name)
    if(WIN32)
        target_compile_definitions(${proj_name} PRIVATE
            NOMINMAX
            UNICODE
            _UNICODE
            WIN32_LEAN_AND_MEAN
            GLREMIX_EXPORTS
        )

        if(MSVC)
            target_compile_options(${proj_name} PRIVATE /MP)
        endif()

        target_link_libraries(${proj_name} PRIVATE
            kernel32
            user32
            advapi32
            ole32
        )
    endif()

    message("")

    if (GLREMIX_OVERRIDE_RENDERER_PATH)
        target_compile_definitions(${proj_name} PRIVATE GLREMIX_CUSTOM_RENDERER_EXE_PATH="${GLREMIX_CUSTOM_RENDERER_EXE_PATH}")
    endif()
endfunction()