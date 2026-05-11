#
# U8g2Font.cmake
#
# Helper for generating U8g2 fonts from BDF files
#

function(u8g2_generate_font)

    set(options)

    set(oneValueArgs
        TARGET
        FONT_NAME
        INPUT
        OUTPUT_DIR
        BDFCONV
    )

    set(multiValueArgs
        GLYPHS
        EXTRA_ARGS
    )

    cmake_parse_arguments(U8G2
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    if(NOT U8G2_TARGET)
        message(FATAL_ERROR "u8g2_generate_font: TARGET missing")
    endif()

    if(NOT U8G2_FONT_NAME)
        message(FATAL_ERROR "u8g2_generate_font: FONT_NAME missing")
    endif()

    if(NOT U8G2_INPUT)
        message(FATAL_ERROR "u8g2_generate_font: INPUT missing")
    endif()

    # -----------------------------
    # Default bdfconv per platform
    # -----------------------------
    if(NOT U8G2_BDFCONV)
        if(WIN32)
            set(U8G2_BDFCONV
                ${CMAKE_SOURCE_DIR}/tools/bdfconv.exe
            )
        else()
            set(U8G2_BDFCONV
                ${CMAKE_SOURCE_DIR}/tools/bdfconv
            )
        endif()
    endif()

    if(NOT U8G2_OUTPUT_DIR)
        set(U8G2_OUTPUT_DIR
            ${CMAKE_BINARY_DIR}/generated_fonts
        )
    endif()

    file(MAKE_DIRECTORY ${U8G2_OUTPUT_DIR})

    set(OUTPUT_C
        ${U8G2_OUTPUT_DIR}/${U8G2_FONT_NAME}.c
    )

    set(OUTPUT_H
        ${U8G2_OUTPUT_DIR}/${U8G2_FONT_NAME}.h
    )

    string(REPLACE ";" "," GLYPH_MAP "${U8G2_GLYPHS}")

    add_custom_command(
        OUTPUT ${OUTPUT_C}

        COMMAND ${U8G2_BDFCONV}
        -f 1
        -r
        -m "${GLYPH_MAP}"
        ${U8G2_EXTRA_ARGS}
        ${U8G2_INPUT}
        -o ${OUTPUT_C}
        -n ${U8G2_FONT_NAME}

        DEPENDS ${U8G2_INPUT}

        COMMENT "Generating U8g2 font: ${U8G2_FONT_NAME}"
        VERBATIM
    )

    file(GENERATE
        OUTPUT ${OUTPUT_H}
        CONTENT
        "#pragma once
#include <stdint.h>
#include \"${U8G2_FONT_NAME}.c\"
"
    )

    add_custom_target(${U8G2_TARGET}
        DEPENDS ${OUTPUT_C} ${OUTPUT_H}
    )

    set(${U8G2_TARGET}_OUTPUT
        ${OUTPUT_C}
        ${OUTPUT_H}
        PARENT_SCOPE
    )

    set(${U8G2_TARGET}_INCLUDE
        ${U8G2_OUTPUT_DIR}
        PARENT_SCOPE
    )

    set_source_files_properties(${OUTPUT_C} PROPERTIES GENERATED TRUE)
    set_source_files_properties(${OUTPUT_H} PROPERTIES GENERATED TRUE)

endfunction()
