cmake_minimum_required(VERSION 3.29)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# ----------------------------------------------------------------------------------------------------------------------
# Detect TI Tools
# ----------------------------------------------------------------------------------------------------------------------
set(TI_SDK "mspm0")
include(TITools)

set(CMAKE_C_COMPILER ${TI_ARM_CLANG})

# ----------------------------------------------------------------------------------------------------------------------
# Project
# ----------------------------------------------------------------------------------------------------------------------
project(SS3 LANGUAGES C)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_EXECUTABLE_SUFFIX ".out")
set(TI_DEVICE "mspm0g150x")

# ----------------------------------------------------------------------------------------------------------------------
# SysConfig
# ----------------------------------------------------------------------------------------------------------------------
include(SysConfig)

set(SYSCONFIG_INPUT ${CMAKE_SOURCE_DIR}/SS3.syscfg)
set(SYSCONFIG_TARGET SS3_syscfg)

add_sysconfig_target(
    ${SDK_ROOT}
    ${SYSCONFIG_TARGET}
    ${SYSCONFIG_INPUT}
)

# ----------------------------------------------------------------------------------------------------------------------
# Sources
# ----------------------------------------------------------------------------------------------------------------------
file(GLOB PROJECT_SOURCES
    ${CMAKE_SOURCE_DIR}/*.c
    ${CMAKE_SOURCE_DIR}/ti/eeprom/emulation_type_b/*.c
)

set(TI_STARTUP_FILE ${SDK_ROOT}/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_${TI_DEVICE}_ticlang.c)

# ----------------------------------------------------------------------------------------------------------------------
# Executable
# ----------------------------------------------------------------------------------------------------------------------
add_executable(SS3
    ${PROJECT_SOURCES}
    $<TARGET_PROPERTY:${SYSCONFIG_TARGET},SYSCONFIG_GEN_CFILES>
    ${TI_STARTUP_FILE}
)

# Ensure SysConfig runs first
add_dependencies(SS3 ${SYSCONFIG_TARGET})

# ----------------------------------------------------------------------------------------------------------------------
# Compile definitions
# ----------------------------------------------------------------------------------------------------------------------
target_compile_definitions(SS3 PRIVATE
    USE_SEG7
)

# ----------------------------------------------------------------------------------------------------------------------
# Compile options
# ----------------------------------------------------------------------------------------------------------------------
target_compile_options(SS3 PRIVATE
    -march=thumbv6m
    -mcpu=cortex-m0plus
    -mfloat-abi=soft
    -mlittle-endian
    -mthumb
    -mmathacl

    @$<TARGET_PROPERTY:${SYSCONFIG_TARGET},SYSCONFIG_GEN_COMPILER_OPTS>
)

# ----------------------------------------------------------------------------------------------------------------------
# Extra compile options
# ----------------------------------------------------------------------------------------------------------------------
target_compile_options(SS3 PRIVATE
    -pedantic
    -pedantic-errors

    -Wall
    -Wextra
    -Werror
    -Wenum-conversion

    -Wno-gnu-label-as-value
    -Wno-gnu-folding-constant
    -Wno-language-extension-token
    -Wno-c2y-extensions
)

# ----------------------------------------------------------------------------------------------------------------------
# Include directories
# ----------------------------------------------------------------------------------------------------------------------
target_include_directories(SS3 PRIVATE
    ${CMAKE_SOURCE_DIR}
    $<TARGET_PROPERTY:${SYSCONFIG_TARGET},SYSCONFIG_GENDIR>
)

target_include_directories(SS3 SYSTEM PRIVATE
    ${SDK_ROOT}/source/third_party/CMSIS/Core/Include
    ${SDK_ROOT}/source
)

# ----------------------------------------------------------------------------------------------------------------------
# Link directories
# ----------------------------------------------------------------------------------------------------------------------
target_link_directories(SS3 PRIVATE
    ${SDK_ROOT}/source
    $<TARGET_PROPERTY:${SYSCONFIG_TARGET},SYSCONFIG_GENDIR>
    ${TI_ARM_CLANG_LIB_DIR}
)

# ----------------------------------------------------------------------------------------------------------------------
# Link options
# ----------------------------------------------------------------------------------------------------------------------
target_link_options(SS3 PRIVATE
    -Wl,-m=SS3.map

    -Wl,--diag_wrap=off
    -Wl,--display_error_number
    -Wl,--warn_sections
    -Wl,--rom_model

    -Wl,--xml_link_info=SS3_linkInfo.xml
)

# ----------------------------------------------------------------------------------------------------------------------
# Linker script + libraries
# ----------------------------------------------------------------------------------------------------------------------
target_link_libraries(SS3 PRIVATE
    ${CMAKE_SOURCE_DIR}/device_linker.cmd
    $<TARGET_PROPERTY:${SYSCONFIG_TARGET},SYSCONFIG_GEN_LNKCMD>
)

# ----------------------------------------------------------------------------------------------------------------------
# Post-build: generate ELF
# ----------------------------------------------------------------------------------------------------------------------
add_custom_command(TARGET SS3 POST_BUILD COMMAND
    ${TI_ARM_OBJCOPY}
    --remove-section=.TI.phattrs
    $<TARGET_FILE:SS3>
    $<TARGET_FILE_DIR:SS3>/$<TARGET_FILE_BASE_NAME:SS3>.elf
)

# ----------------------------------------------------------------------------------------------------------------------
# Debug vs Release
# ----------------------------------------------------------------------------------------------------------------------
target_compile_definitions(SS3 PRIVATE
    $<$<CONFIG:Debug>:NO_WDG NO_EEPROM_SAVE>
)
target_link_options(SS3 PRIVATE
    $<$<CONFIG:Release>:LINKER:--define=WITH_NONMAIN>
)

target_compile_options(SS3 PRIVATE
    $<$<CONFIG:Debug>:-O0 -gdwarf-3>
    $<$<CONFIG:Release>:-Os -gdwarf-3 -flto>
)

target_link_options(SS3 PRIVATE
    $<$<CONFIG:Release>:-flto>
)
