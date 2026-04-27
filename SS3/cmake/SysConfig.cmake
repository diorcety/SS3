# --------------------------------------------------------------
# Sysconfig macro
# --------------------------------------------------------------

function(add_sysconfig_target SDK_ROOT TARGET_NAME SYSCFG_FILE)

    # --------------------------------------------------------------
    # Resolve paths
    # --------------------------------------------------------------
    get_filename_component(SYSCFG_NAME ${SYSCFG_FILE} NAME)
    get_filename_component(SYSCFG_ABS  ${SYSCFG_FILE} ABSOLUTE)

    set(SYSCONFIG_GENDIR ${CMAKE_BINARY_DIR}/sysconfig_generated)

    set(SYSCONFIG_STAMP_FILE
        ${SYSCONFIG_GENDIR}/${SYSCFG_NAME}.stamp
    )

    # --------------------------------------------------------------
    # Find CLI
    # --------------------------------------------------------------
    find_program(SYSCONFIG_CLI sysconfig_cli.bat REQUIRED)

    set(SYSCONFIG_CLI_CMD
        ${SYSCONFIG_CLI}
            --script ${SYSCFG_ABS}
            -o ${SYSCONFIG_GENDIR}
            --compiler ticlang
            -s ${SDK_ROOT}/.metadata/product.json
    )

    # --------------------------------------------------------------
    # Query generated files
    # --------------------------------------------------------------
    execute_process(
        COMMAND ${SYSCONFIG_CLI_CMD} --listGeneratedFiles
        OUTPUT_VARIABLE SYSCONFIG_GEN_FILES_STR
    )

    string(REPLACE "\n" ";" SYSCONFIG_GEN_FILES ${SYSCONFIG_GEN_FILES_STR})

    set(SYSCONFIG_GEN_CFILES        ${SYSCONFIG_GEN_FILES})
    set(SYSCONFIG_GEN_HFILES        ${SYSCONFIG_GEN_FILES})
    set(SYSCONFIG_GEN_LNKCMD        ${SYSCONFIG_GEN_FILES})
    set(SYSCONFIG_GEN_COMPILER_OPTS ${SYSCONFIG_GEN_FILES})

    list(FILTER SYSCONFIG_GEN_CFILES        INCLUDE REGEX ".*\\.c$")
    list(FILTER SYSCONFIG_GEN_HFILES        INCLUDE REGEX ".*\\.h$")
    list(FILTER SYSCONFIG_GEN_LNKCMD        INCLUDE REGEX ".*\\.cmd\\.")
    list(FILTER SYSCONFIG_GEN_COMPILER_OPTS INCLUDE REGEX ".*\\.opt$")

    # Reconfigure if syscfg changes
    set_property(
        DIRECTORY
        APPEND
        PROPERTY CMAKE_CONFIGURE_DEPENDS ${SYSCFG_ABS}
    )

    # --------------------------------------------------------------
    # Build step
    # --------------------------------------------------------------
    add_custom_command(
        DEPENDS    ${SYSCFG_ABS}
        OUTPUT     ${SYSCONFIG_STAMP_FILE}
        BYPRODUCTS ${SYSCONFIG_GEN_FILES}
        COMMAND    ${SYSCONFIG_CLI_CMD}
        COMMAND    ${CMAKE_COMMAND} -E touch ${SYSCONFIG_STAMP_FILE}
        COMMENT    "Running SysConfig on ${SYSCFG_NAME}"
        VERBATIM
    )

    add_custom_target(${TARGET_NAME}
        DEPENDS ${SYSCONFIG_STAMP_FILE}
    )

    # --------------------------------------------------------------
    # Export to parent scope
    # --------------------------------------------------------------
    set_property(TARGET ${TARGET_NAME} PROPERTY
        SYSCONFIG_GENDIR "${SYSCONFIG_GENDIR}"
    )

    set_property(TARGET ${TARGET_NAME} PROPERTY
        SYSCONFIG_GEN_CFILES "${SYSCONFIG_GEN_CFILES}"
    )

    set_property(TARGET ${TARGET_NAME} PROPERTY
        SYSCONFIG_GEN_HFILES "${SYSCONFIG_GEN_HFILES}"
    )

    set_property(TARGET ${TARGET_NAME} PROPERTY
        SYSCONFIG_GEN_LNKCMD "${SYSCONFIG_GEN_LNKCMD}"
    )

    set_property(TARGET ${TARGET_NAME} PROPERTY
        SYSCONFIG_GEN_COMPILER_OPTS "${SYSCONFIG_GEN_COMPILER_OPTS}"
    )

endfunction()
