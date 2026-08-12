include(CMakeParseArguments)

function(add_slang_shader_target TARGET_NAME)
    cmake_parse_arguments(
            SHADER
            ""
            "OUTPUT"
            "SOURCES"
            ${ARGN}
    )

    if (NOT SHADER_SOURCES)
        message(FATAL_ERROR
                "add_slang_shader_target(${TARGET_NAME}) requires SOURCES"
        )
    endif()

    if (NOT SHADER_OUTPUT)
        message(FATAL_ERROR
                "add_slang_shader_target(${TARGET_NAME}) requires OUTPUT"
        )
    endif()

    add_custom_command(
            OUTPUT
            "${SHADER_OUTPUT}"

            COMMAND
            "${SLANGC_EXECUTABLE}"
            ${SHADER_SOURCES}
            -target spirv
            -profile spirv_1_4
            -emit-spirv-directly
            -fvk-use-entrypoint-name
            -entry vertMain
            -entry fragMain
            -o "${SHADER_OUTPUT}"

            DEPENDS
            ${SHADER_SOURCES}
            "${SLANGC_EXECUTABLE}"

            COMMENT
            "Compiling Slang shader ${TARGET_NAME}"

            VERBATIM
    )

    add_custom_target(
            "${TARGET_NAME}"
            DEPENDS
            "${SHADER_OUTPUT}"
    )
endfunction()