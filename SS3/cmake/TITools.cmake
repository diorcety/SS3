# --------------------------------------------------------------
# TI Tools auto-discovery with version control
# --------------------------------------------------------------

# Ensure TI_SDK is defined and not empty
if(NOT DEFINED TI_SDK OR TI_SDK STREQUAL "")
    message(FATAL_ERROR "TI_SDK is not set. Please define it (e.g. -DTI_SDK=msp0m or set(TI_SDK msp0m))")
endif()

set(TI_ROOT "C:/ti" CACHE PATH "Root directory of TI tools")

# Optional overrides
set(TI_COMPILER_VERSION "" CACHE STRING "Force TI ARM Clang version (e.g. 4.0.4.LTS)")
string(REPLACE "." "_" TI_COMPILER_VERSION "${TI_COMPILER_VERSION}")
set(TI_SYSCONFIG_VERSION "" CACHE STRING "Force SysConfig version (e.g. 1.27.0)")
string(REPLACE "." "_" TI_SYSCONFIG_VERSION "${TI_SYSCONFIG_VERSION}")
set(TI_SDK_VERSION "" CACHE STRING "Force ${TI_SDK} SDK version (e.g. 2.10.00.04)")
string(REPLACE "." "_" TI_SDK_VERSION "${TI_SDK_VERSION}")


# Constants
set(_TI_COMPILER_PREFIX "ti[_-]*cgt[_-]*arm[_-]*llvm[_-]")
set(_TI_SYSCONFIG_PREFIX "sysconfig[_-]")
set(_TI_SDK_PREFIX "${TI_SDK}[_-]sdk[_-]")

# --------------------------------------------------------------
# Extract version from path
# --------------------------------------------------------------
function(_ti_extract_version input prefix output)
    # Normalize to forward slashes
    file(TO_CMAKE_PATH "${input}" norm_path)

    # Split into components
    string(REPLACE "/" ";" parts "${norm_path}")

    foreach(part ${parts})
        if(part MATCHES "^${prefix}(.+)$")
            set(version "${CMAKE_MATCH_1}")

            # Normalize TI version format:
            # 2_10_00_04 -> 2.10.00.04
            string(REPLACE "_" "." version "${version}")

            set(${output} "${version}" PARENT_SCOPE)
            return()
        endif()
    endforeach()
endfunction()

# --------------------------------------------------------------
# Sorts a list of version indices using VERSION_LESS comparison.
# --------------------------------------------------------------
macro(sort_version_indices versions out_indices)
    set(_indices "")
    list(LENGTH ${versions} _len)

    if(_len EQUAL 0)
        set(${out_indices} "")
        return()
    endif()

    math(EXPR _last "${_len} - 1")

    foreach(i RANGE 0 ${_last})
        list(APPEND _indices ${i})
    endforeach()

    set(${out_indices} "")

    foreach(i IN LISTS _indices)
        list(GET ${versions} ${i} _vi)

        set(_inserted FALSE)
        set(_tmp "")

        foreach(j IN LISTS ${out_indices})
            list(GET ${versions} ${j} _vj)

            if(NOT _inserted AND _vi VERSION_LESS _vj)
                list(APPEND _tmp ${i})
                set(_inserted TRUE)
            endif()

            list(APPEND _tmp ${j})
        endforeach()

        if(NOT _inserted)
            list(APPEND _tmp ${i})
        endif()

        set(${out_indices} ${_tmp})
    endforeach()
endmacro()

# --------------------------------------------------------------
# Select directory by version
# --------------------------------------------------------------
function(_ti_select out_var mode force_version prefix)
    # mode = LATEST or OLDEST
    set(candidates ${ARGN})
    set(filtered_versions "")
    set(filtered_dirs "")

    foreach(dir ${candidates})
        unset(version)
        _ti_extract_version("${dir}" "${prefix}" version)

        if(DEFINED version)
            if(force_version)
                if(ver STREQUAL force_version)
                    set(${out_var} ${dir} PARENT_SCOPE)
                    return()
                endif()
            else()
                list(APPEND filtered_versions "${version}")
                list(APPEND filtered_dirs "${dir}")
            endif()
        endif()
    endforeach()

    if(force_version)
        message(FATAL_ERROR "Requested version '${force_version}' not found in: ${candidates}")
    endif()

    if(NOT filtered_versions)
        message(FATAL_ERROR "No directories matched prefix '${prefix}' in: ${candidates}")
    endif()

    # Sort indices based on VERSION comparison
    sort_version_indices(filtered_versions sorted_indices)

    if(mode STREQUAL "LATEST")
        list(GET sorted_indices -1 idx)
    else()
        list(GET sorted_indices 0 idx)
    endif()

    list(GET filtered_dirs ${idx} result)
    set(${out_var} ${result} PARENT_SCOPE)
endfunction()


# List directories inside TI_ROOT
file(GLOB TI_DIRS "${TI_ROOT}/*")

# --------------------------------------------------------------
# Discover ARM Clang
# --------------------------------------------------------------
_ti_select(TI_ARM_COMPILER_DIR "LATEST" "${TI_COMPILER_VERSION}" ${_TI_COMPILER_PREFIX} ${TI_DIRS})
set(TI_ARM_CLANG
    "${TI_ARM_COMPILER_DIR}/bin/tiarmclang.exe"
    CACHE FILEPATH "TI ARM Clang compiler"
)
set(TI_ARM_OBJCOPY
    "${TI_ARM_COMPILER_DIR}/bin/tiarmobjcopy.exe"
    CACHE FILEPATH "TI ARM objcopy"
)
set(TI_ARM_CLANG_LIB_DIR
    "${TI_ARM_COMPILER_DIR}/lib"
    CACHE PATH "TI ARM Clang libraries"
)

# --------------------------------------------------------------
# Discover SysConfig (default: OLDEST)
# --------------------------------------------------------------
_ti_select(TI_SYSCONFIG_DIR "OLDEST" "${TI_SYSCONFIG_VERSION}" ${_TI_SYSCONFIG_PREFIX} ${TI_DIRS})
set(SYSCONFIG_CLI
    "${TI_SYSCONFIG_DIR}/sysconfig_cli.bat"
    CACHE FILEPATH "SysConfig CLI"
)

# --------------------------------------------------------------
# Discover SDK (default: LATEST)
# --------------------------------------------------------------
_ti_select(TI_SDK_ROOT "LATEST" "${TI_SDK_VERSION}" ${_TI_SDK_PREFIX} ${TI_DIRS})
set(SDK_ROOT
    "${TI_SDK_ROOT}"
    CACHE PATH "${TI_SDK} SDK root"
)

# --------------------------------------------------------------
# Pretty print versions
# --------------------------------------------------------------
_ti_extract_version("${TI_ARM_COMPILER_DIR}" ${_TI_COMPILER_PREFIX} TI_COMPILER_VER)
_ti_extract_version("${TI_SYSCONFIG_DIR}" ${_TI_SYSCONFIG_PREFIX} TI_SYSCONFIG_VER)
_ti_extract_version("${TI_SDK_ROOT}" ${_TI_SDK_PREFIX} TI_SDK_VER)

message(STATUS "TI Compiler  : ${TI_COMPILER_VER} -> ${TI_ARM_CLANG}")
message(STATUS "SysConfig    : ${TI_SYSCONFIG_VER} -> ${SYSCONFIG_CLI}")
message(STATUS "${TI_SDK} SDK    : ${TI_SDK_VER} -> ${SDK_ROOT}")
