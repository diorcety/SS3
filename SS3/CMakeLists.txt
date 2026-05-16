cmake_minimum_required(VERSION 3.29)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)


set(SCREEN "SEG7" CACHE STRING "Display type (SEG7 or ST7789)")
option(SIMULATION "Enable simulation mode" OFF)

# ----------------------------------------------------------------------------------------------------------------------
# Detect TI Tools
# ----------------------------------------------------------------------------------------------------------------------
if(NOT SIMULATION)
    set(TI_SDK "mspm0")
    include(TITools)

    set(CMAKE_C_COMPILER ${TI_ARM_CLANG})
    set(CMAKE_CXX_COMPILER ${TI_ARM_CLANG})
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32")
endif()


# ----------------------------------------------------------------------------------------------------------------------
# Project
# ----------------------------------------------------------------------------------------------------------------------
project(SS3 LANGUAGES C CXX)

if(NOT SIMULATION)
    add_compile_options(
        -march=thumbv6m
        -mcpu=cortex-m0plus
        -mfloat-abi=soft
        -mlittle-endian
        -mthumb
        -mmathacl
    )
    add_definitions(-DCORU_NO_ASSERT)
else()
    add_definitions(-DSIMULATION -DCORU_ON_VALGRIND)
endif()

add_definitions(-DCORU_NO_MALLOC -DLITTLE_FOOT_PRINT)
add_compile_definitions(
    $<$<CONFIG:Debug>:NO_WDG>
    $<$<CONFIG:Debug>:NO_EEPROM_SAVE>
)
add_compile_definitions(
    $<$<STREQUAL:${SCREEN},ST7789>:USE_ST7789>
    $<$<STREQUAL:${SCREEN},SEG7>:USE_SEG7>
)

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

    target_compile_definitions(Arduino_GFX PUBLIC U8G2_FONT_SUPPORT ATTINY_CORE WITHOUT_STANDARD_FONT)

    target_include_directories(Arduino_GFX PRIVATE
        ${CMAKE_SOURCE_DIR}/arduino
    )
    target_compile_options(Arduino_GFX PRIVATE -w)

    target_compile_options(Arduino_GFX PRIVATE
        $<$<CONFIG:Debug>:-O0 -gdwarf-3>
        $<$<CONFIG:Release>:-Os -gdwarf-3 -flto>
    )

    target_compile_options(Arduino_GFX PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
    )

    fetchcontent_declare(
        coru
        GIT_REPOSITORY https://github.com/diorcety/coru.git
        GIT_TAG master
    )

    fetchcontent_makeavailable(coru)

    # ------------------------------------------------------------------------------------------------------------------
    # Interface target: coru
    # ------------------------------------------------------------------------------------------------------------------
    add_library(coru_iface INTERFACE)
    target_sources(coru_iface INTERFACE
        ${coru_SOURCE_DIR}/coru.c
        ${coru_SOURCE_DIR}/coru_platform.c
    )
    target_include_directories(coru_iface INTERFACE
        ${coru_SOURCE_DIR}
    )

    # ------------------------------------------------------------------------------------------------------------------
    # Interface target: Arduino_GFX + generated fonts
    # ------------------------------------------------------------------------------------------------------------------
    include(cmake/U8g2Font.cmake)

    u8g2_generate_font(
        TARGET font_inconsolata60
        FONT_NAME inconsolata60

        INPUT
        ${CMAKE_SOURCE_DIR}/resources/InconsolataLGC-Bold-60.bdf

        GLYPHS
        32,45,48-57,141,146

        EXTRA_ARGS
        -f 1
        -b 2
    )

    u8g2_generate_font(
        TARGET font_inconsolata48
        FONT_NAME inconsolata48

        INPUT
        ${CMAKE_SOURCE_DIR}/resources/InconsolataLGC-Bold-48.bdf

        GLYPHS
        32,128-131,140,142

        EXTRA_ARGS
        -f 1
        -b 2
    )

    u8g2_generate_font(
        TARGET font_inconsolata24
        FONT_NAME inconsolata24

        INPUT
        ${CMAKE_SOURCE_DIR}/resources/InconsolataLGC-Bold-24.bdf

        GLYPHS
        32-126,133-138,143-145,150-154,156-159,176

        EXTRA_ARGS
        -f 1
        -b 2
    )

    add_library(arduino_gfx_iface INTERFACE)
    target_link_libraries(arduino_gfx_iface INTERFACE Arduino_GFX coru_iface)
    target_include_directories(arduino_gfx_iface SYSTEM INTERFACE
        ${arduino_gfx_SOURCE_DIR}/src
    )
    target_include_directories(arduino_gfx_iface INTERFACE
        ${CMAKE_SOURCE_DIR}/arduino
        ${font_inconsolata24_INCLUDE}
        ${font_inconsolata48_INCLUDE}
        ${font_inconsolata60_INCLUDE}
    )
    add_dependencies(arduino_gfx_iface font_inconsolata24 font_inconsolata48 font_inconsolata60)

    if(SIMULATION)
        # Arduino_GFX is a compiled static library: it must see SIMULATION at its own
        # compile time, not just at the executable level.  PUBLIC propagates the
        # definition through arduino_gfx_iface to all consumers as well.
        target_compile_definitions(Arduino_GFX PUBLIC SIMULATION)
    endif()
endif()

set(CMAKE_SYSTEM_NAME Generic)
if(NOT SIMULATION)
    set(CMAKE_EXECUTABLE_SUFFIX ".out")
endif()
set(TI_DEVICE "mspm0g150x")

# ----------------------------------------------------------------------------------------------------------------------
# SysConfig
# ----------------------------------------------------------------------------------------------------------------------
if(NOT SIMULATION)
    include(SysConfig)

    set(SYSCONFIG_INPUT ${CMAKE_SOURCE_DIR}/SS3.syscfg)
    set(SYSCONFIG_TARGET SS3_syscfg)

    add_sysconfig_target(
        ${SDK_ROOT}
        ${SYSCONFIG_TARGET}
        ${SYSCONFIG_INPUT}
    )
endif()

# ----------------------------------------------------------------------------------------------------------------------
# Sources
# ----------------------------------------------------------------------------------------------------------------------
# C sources
file(GLOB PROJECT_C_SOURCES
    ${CMAKE_SOURCE_DIR}/*.c
)

if(NOT SIMULATION)
    file(GLOB EEPROM_SOURCES
        ${CMAKE_SOURCE_DIR}/ti/eeprom/emulation_type_b/*.c
    )
    list(APPEND PROJECT_C_SOURCES ${EEPROM_SOURCES})
endif()

# C++ sources
file(GLOB PROJECT_CPP_SOURCES
    ${CMAKE_SOURCE_DIR}/*.cpp
    ${CMAKE_SOURCE_DIR}/arduino/*.cpp
)
list(FILTER PROJECT_CPP_SOURCES EXCLUDE REGEX "test.*.cpp")

if(NOT SIMULATION)
    list(FILTER PROJECT_C_SOURCES EXCLUDE REGEX "simulation.*.c")
    list(FILTER PROJECT_CXX_SOURCES EXCLUDE REGEX "simulation.*.cpp")
endif()

set(TI_STARTUP_FILE ${SDK_ROOT}/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_${TI_DEVICE}_ticlang.c)

# ----------------------------------------------------------------------------------------------------------------------
# Executable
# ----------------------------------------------------------------------------------------------------------------------
add_executable(SS3
    ${PROJECT_C_SOURCES}
)

# Ensure SysConfig runs first
if(NOT SIMULATION)
    target_sources(SS3 PRIVATE $<TARGET_PROPERTY:${SYSCONFIG_TARGET},SYSCONFIG_GEN_CFILES> ${TI_STARTUP_FILE})
    add_dependencies(SS3 ${SYSCONFIG_TARGET})

    get_target_property(SYSCONFIG_FILES
        ${SYSCONFIG_TARGET}
        SYSCONFIG_GEN_CFILES
    )

    set_source_files_properties(
        ${SYSCONFIG_FILES}
        PROPERTIES
        COMPILE_OPTIONS "-Wno-cast-qual"
    )
endif()

# ----------------------------------------------------------------------------------------------------------------------
# Compile options
# ----------------------------------------------------------------------------------------------------------------------
if(NOT SIMULATION)
    target_compile_options(SS3 PRIVATE
        @$<TARGET_PROPERTY:${SYSCONFIG_TARGET},SYSCONFIG_GEN_COMPILER_OPTS>
    )
endif()

# ----------------------------------------------------------------------------------------------------------------------
# Extra compile options (shared between SS3 and test targets)
# ----------------------------------------------------------------------------------------------------------------------
add_library(project_compile_options INTERFACE)
target_compile_options(project_compile_options INTERFACE
    -pedantic
    -pedantic-errors

    -Wall
    -Wextra
    -Werror
    -Wenum-conversion
    -Wassign-enum
    -Wcast-qual
    -Wwrite-strings

    -Wno-gnu-label-as-value
    -Wno-gnu-folding-constant
    -Wno-language-extension-token
    -Wno-c2y-extensions
    -Wno-empty-translation-unit
    -Wno-gnu-zero-variadic-macro-arguments

    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>

    $<$<CONFIG:Debug>:-O0 -gdwarf-3 -fno-omit-frame-pointer>
    $<$<CONFIG:Release>:-Os -gdwarf-3 -flto>
)

target_link_libraries(SS3 PRIVATE project_compile_options)

# ----------------------------------------------------------------------------------------------------------------------
# Include directories
# ----------------------------------------------------------------------------------------------------------------------
target_include_directories(SS3 PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/arduino
)

if(NOT SIMULATION)
    target_include_directories(SS3 PRIVATE
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
endif()

# ----------------------------------------------------------------------------------------------------------------------
# Libraries
# ----------------------------------------------------------------------------------------------------------------------
if(SCREEN STREQUAL "ST7789")
    target_link_libraries(SS3 PRIVATE arduino_gfx_iface)

    # Add c++ sources
    target_sources(SS3 PRIVATE ${PROJECT_CPP_SOURCES})
endif()

# ----------------------------------------------------------------------------------------------------------------------
# Post-build: generate ELF
# ----------------------------------------------------------------------------------------------------------------------
if(NOT SIMULATION)
    add_custom_command(TARGET SS3 POST_BUILD COMMAND
        ${TI_ARM_OBJCOPY}
        --remove-section=.TI.phattrs
        $<TARGET_FILE:SS3>
        $<TARGET_FILE_DIR:SS3>/$<TARGET_FILE_BASE_NAME:SS3>.elf
    )
endif()

# ----------------------------------------------------------------------------------------------------------------------
# Debug vs Release
# ----------------------------------------------------------------------------------------------------------------------
target_link_options(SS3 PRIVATE
    $<$<CONFIG:Release>:LINKER:--define=WITH_NONMAIN>
)

target_link_options(SS3 PRIVATE
    $<$<CONFIG:Release>:-flto>
)

# ----------------------------------------------------------------------------------------------------------------------
# Tests (SIMULATION only)
# ----------------------------------------------------------------------------------------------------------------------
if(SIMULATION)
    include(FetchContent)

    fetchcontent_declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    fetchcontent_makeavailable(googletest)

    enable_testing()

    file(GLOB TEST_SOURCES ${CMAKE_SOURCE_DIR}/test_*.cpp)
    list(FILTER PROJECT_C_SOURCES EXCLUDE REGEX "simulation.*.c")
    list(FILTER PROJECT_C_SOURCES EXCLUDE REGEX ".*/main\\.c$")
    list(FILTER PROJECT_CPP_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")
    list(FILTER PROJECT_CPP_SOURCES EXCLUDE REGEX "simulation.*.cpp")

    foreach(TEST_SOURCE ${TEST_SOURCES})
        get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

        add_executable(${TEST_NAME} ${TEST_SOURCE})

        target_sources(${TEST_NAME} PRIVATE
            ${PROJECT_C_SOURCES}
            ${PROJECT_CPP_SOURCES}
        )

        target_compile_definitions(${TEST_NAME} PRIVATE
            TESTS
        )

        target_link_libraries(${TEST_NAME} PRIVATE project_compile_options)

        if(SCREEN STREQUAL "ST7789")
            target_include_directories(${TEST_NAME} PRIVATE
                ${CMAKE_SOURCE_DIR}
            )
            target_link_libraries(${TEST_NAME} PRIVATE arduino_gfx_iface)
        endif()

        target_link_libraries(${TEST_NAME} PRIVATE
            GTest::gtest_main
        )

        include(GoogleTest)
        gtest_discover_tests(${TEST_NAME})
    endforeach()
endif()
