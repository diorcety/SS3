cmake_minimum_required(VERSION 3.29)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)


set(SCREEN "SEG7" CACHE STRING "Display type (SEG7 or ST7789)")

# ----------------------------------------------------------------------------------------------------------------------
# Detect TI Tools
# ----------------------------------------------------------------------------------------------------------------------
set(TI_SDK "mspm0")
include(TITools)

set(CMAKE_C_COMPILER ${TI_ARM_CLANG})
set(CMAKE_CXX_COMPILER ${TI_ARM_CLANG})


# ----------------------------------------------------------------------------------------------------------------------
# Project
# ----------------------------------------------------------------------------------------------------------------------
project(SS3 LANGUAGES C CXX)

add_compile_options(
    -march=thumbv6m
    -mcpu=cortex-m0plus
    -mfloat-abi=soft
    -mlittle-endian
    -mthumb
    -mmathacl
)

add_compile_options(
    -fno-exceptions
    -fno-rtti
    -fno-threadsafe-statics
)

add_link_options(
    -fno-exceptions
    -fno-rtti
)

add_definitions(-DCORU_NO_ASSERT -DCORU_NO_MALLOC -DLITTLE_FOOT_PRINT)

# ----------------------------------------------------------------------------------------------------------------------
# Libraries
# ----------------------------------------------------------------------------------------------------------------------

include(FetchContent)

if(SCREEN STREQUAL "ST7789")
    fetchcontent_declare(
        Arduino_GFX
        GIT_REPOSITORY https://github.com/diorcety/Arduino_GFX.git
        GIT_TAG my_v1.6.5
    )

    fetchcontent_makeavailable(Arduino_GFX)

    target_compile_definitions(Arduino_GFX PUBLIC U8G2_FONT_SUPPORT)

    target_include_directories(Arduino_GFX PRIVATE
        ${CMAKE_SOURCE_DIR}/arduino
    )
    target_compile_options(Arduino_GFX PRIVATE -w)

    target_compile_options(Arduino_GFX PRIVATE
        $<$<CONFIG:Debug>:-O0 -gdwarf-3>
        $<$<CONFIG:Release>:-Os -gdwarf-3 -flto>
    )

    fetchcontent_declare(
        coru
        GIT_REPOSITORY https://github.com/geky/coru.git
        GIT_TAG master
    )

    fetchcontent_makeavailable(coru)
endif()

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
# C sources
file(GLOB PROJECT_C_SOURCES
    ${CMAKE_SOURCE_DIR}/*.c
    ${CMAKE_SOURCE_DIR}/ti/eeprom/emulation_type_b/*.c
)

# C++ sources
file(GLOB PROJECT_CPP_SOURCES
    ${CMAKE_SOURCE_DIR}/*.cpp
    ${CMAKE_SOURCE_DIR}/arduino/*.cpp
)

set(TI_STARTUP_FILE ${SDK_ROOT}/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_${TI_DEVICE}_ticlang.c)

# ----------------------------------------------------------------------------------------------------------------------
# Executable
# ----------------------------------------------------------------------------------------------------------------------
add_executable(SS3
    ${PROJECT_C_SOURCES}
    $<TARGET_PROPERTY:${SYSCONFIG_TARGET},SYSCONFIG_GEN_CFILES>
    ${TI_STARTUP_FILE}
)

# Ensure SysConfig runs first
add_dependencies(SS3 ${SYSCONFIG_TARGET})

# ----------------------------------------------------------------------------------------------------------------------
# Compile definitions
# ----------------------------------------------------------------------------------------------------------------------
target_compile_definitions(SS3 PRIVATE
    $<$<STREQUAL:${SCREEN},ST7789>:USE_ST7789>
    $<$<STREQUAL:${SCREEN},SEG7>:USE_SEG7>
)

# ----------------------------------------------------------------------------------------------------------------------
# Compile options
# ----------------------------------------------------------------------------------------------------------------------
target_compile_options(SS3 PRIVATE
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
    -Wno-empty-translation-unit
)

# ----------------------------------------------------------------------------------------------------------------------
# Include directories
# ----------------------------------------------------------------------------------------------------------------------
target_include_directories(SS3 PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/arduino
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
# Libraries
# ----------------------------------------------------------------------------------------------------------------------
if(SCREEN STREQUAL "ST7789")
    target_link_libraries(SS3 PRIVATE Arduino_GFX)
    target_include_directories(SS3 SYSTEM PRIVATE
        ${arduino_gfx_SOURCE_DIR}/src
    )

    # coru has no CMakeLists.txt, so add sources manually
    target_sources(SS3 PRIVATE
        ${coru_SOURCE_DIR}/coru.c
        ${coru_SOURCE_DIR}/coru_platform.c
    )
    set_source_files_properties(
        ${coru_SOURCE_DIR}/coru.c
        ${coru_SOURCE_DIR}/coru_platform.c
    )

    target_include_directories(SS3 PRIVATE
        ${coru_SOURCE_DIR}
    )

    include(cmake/U8g2Font.cmake)

    u8g2_generate_font(
        TARGET font_logisoso160
        FONT_NAME logisoso160

        INPUT
        ${CMAKE_SOURCE_DIR}/resources/Logisoso-160.bdf

        GLYPHS
        45,48-57,141
    )


    u8g2_generate_font(
        TARGET font_logisoso64
        FONT_NAME logisoso64

        INPUT
        ${CMAKE_SOURCE_DIR}/resources/Logisoso-64.bdf

        GLYPHS
        32,128-131,140-142
    )

    u8g2_generate_font(
        TARGET font_logisoso32
        FONT_NAME logisoso32

        INPUT
        ${CMAKE_SOURCE_DIR}/resources/Logisoso-32.bdf

        GLYPHS
        32,48-57,64-90,97-122,143-145,150-154,176
    )

    target_include_directories(SS3 PRIVATE
        ${font_logisoso64_INCLUDE} ${font_logisoso160_INCLUDE} ${font_logisoso32_INCLUDE}
    )

    add_dependencies(SS3 font_logisoso64 font_logisoso160 font_logisoso32)

    # Add c++ sources
    target_sources(SS3 PRIVATE ${PROJECT_CPP_SOURCES})
endif()

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
