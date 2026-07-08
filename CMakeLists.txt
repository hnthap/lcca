# Custom macros (disabled by default):
#
# - LCCA_DEVELOPER_MODE
# 

cmake_minimum_required(VERSION 3.15)

project(Lunar_Calendar_C_API C)

# --- 1. Standard ---
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF) # Disables GCC extensions (typeof, __attribute__, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- 2. Directories ---
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/tests)
set(BUILD_BIN_DIR ${CMAKE_BINARY_DIR}/bin)

# --- 3. Core Library ---
file(GLOB SOURCES CONFIGURE_DEPENDS "${SOURCE_DIR}/*.c")
add_library(lcca SHARED ${SOURCES})
target_include_directories(lcca PUBLIC ${INCLUDE_DIR})

# --- 4. The Paranoia Engine ---
include(CheckCCompilerFlag)

function(add_flag_if_supported target flag)
    string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" safe_flag ${flag})
    check_c_compiler_flag("${flag}" HAS_${safe_flag})
    if(HAS_${safe_flag})
        target_compile_options(${target} PRIVATE ${flag})
    endif()
endfunction()

# --- Build Type Configuration ---
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# Developer mode enables maximum strictness and sanitizers
option(LCCA_DEVELOPER_MODE "Enable all warnings and sanitizers (for development)" OFF)

# --- 5. COMPILER FLAGS: THE FINAL LIST ---
if(MSVC)
    target_compile_options(lcca PRIVATE
        /W4     # Warning Level 4 (Highest sensible)
        /WX     # Treat warnings as errors
        /permissive- # Enable Standards Conformance Mode
        /w14242 # Conversion checks (partial equivalent to -Wconversion)
        /w14905 # Wide string cast checks
    )
    # Stop MSVC from crying about scanf
    target_compile_definitions(lcca PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    if(CMAKE_C_COMPILER_ID MATCHES "Clang")
        set(CMAKE_REQUIRED_FLAGS "-Werror -Werror=unknown-warning-option -Werror=unknown-argument")
    else()
        # GCC throws a fatal error if given Clang's unknown-warning-option
        set(CMAKE_REQUIRED_FLAGS "-Werror")
    endif()

    # === A. The Foundation ===
    add_flag_if_supported(lcca "-Wall")
    add_flag_if_supported(lcca "-Wextra")
    add_flag_if_supported(lcca "-Werror")                # Warnings are Errors
    add_flag_if_supported(lcca "-pedantic")              # No Extensions

    # === B. Static Analysis & Security (Run-time Hardening) ===
    add_flag_if_supported(lcca "-fanalyzer")             # (GCC 10+) Deep static analysis
    add_flag_if_supported(lcca "-Xanalyzer")             # (Clang) Deep static analysis
    add_flag_if_supported(lcca "-fstack-protector-strong") # Stack smashing protection
    # Note: _FORTIFY_SOURCE requires optimization (-O1 or higher) to work
    target_compile_definitions(lcca PRIVATE _FORTIFY_SOURCE=2)

    # === C. The Compatibility Chain ===
    add_flag_if_supported(lcca "-Wc90-c99-compat")
    add_flag_if_supported(lcca "-Wc99-c11-compat")
    add_flag_if_supported(lcca "-Wc11-c23-compat")

    # === D. Strict C90 ===
    add_flag_if_supported(lcca "-Wdeclaration-after-statement")
    add_flag_if_supported(lcca "-Wvla")
    add_flag_if_supported(lcca "-Wbad-function-cast")    # C90 quirk check

    # === E. Types, Math & Conversion ===
    add_flag_if_supported(lcca "-Wconversion")           # The "Boss" flag
    add_flag_if_supported(lcca "-Wsign-conversion")
    add_flag_if_supported(lcca "-Wfloat-equal")
    add_flag_if_supported(lcca "-Wdouble-promotion")
    add_flag_if_supported(lcca "-Wshift-overflow=2")     # Catch x << 31 bugs
    add_flag_if_supported(lcca "-Wstrict-overflow=5")    # Catch optimization assumptions

    # === F. Memory & Pointers ===
    add_flag_if_supported(lcca "-Wstrict-aliasing=3")
    add_flag_if_supported(lcca "-Wwrite-strings")
    add_flag_if_supported(lcca "-Wcast-qual")
    add_flag_if_supported(lcca "-Wcast-align=strict")
    add_flag_if_supported(lcca "-Wpointer-arith")
    add_flag_if_supported(lcca "-Wuse-after-free=3")
    add_flag_if_supported(lcca "-Wnull-dereference")     # If (p == NULL) *p = x;
    add_flag_if_supported(lcca "-Walloc-zero")
    add_flag_if_supported(lcca "-Warray-bounds=2")       # Stricter array checks
    add_flag_if_supported(lcca "-Wstringop-overflow=4")  # Max string buffer protection

    # === G. Functions ===
    add_flag_if_supported(lcca "-Wstrict-prototypes")
    add_flag_if_supported(lcca "-Wmissing-prototypes")
    add_flag_if_supported(lcca "-Wmissing-declarations")
    add_flag_if_supported(lcca "-Wold-style-definition")
    add_flag_if_supported(lcca "-Wnested-externs")
    add_flag_if_supported(lcca "-Wredundant-decls")
    add_flag_if_supported(lcca "-Wsuggest-attribute=noreturn") # Opt hint
    # add_flag_if_supported(lcca "-Wsuggest-attribute=format")   # Opt hint

    # === H. Logic & Code Quality ===
    add_flag_if_supported(lcca "-Wshadow")
    add_flag_if_supported(lcca "-Wjump-misses-init")
    add_flag_if_supported(lcca "-Wlogical-op")
    add_flag_if_supported(lcca "-Wduplicated-branches")
    add_flag_if_supported(lcca "-Wduplicated-cond")
    add_flag_if_supported(lcca "-Wswitch-default")
    add_flag_if_supported(lcca "-Wswitch-enum")
    add_flag_if_supported(lcca "-Winit-self")
    add_flag_if_supported(lcca "-Wmisleading-indentation") # if (a) b; c; (c runs always)
    # add_flag_if_supported(lcca "-Wunused-macros")
    add_flag_if_supported(lcca "-Wformat=2")
    add_flag_if_supported(lcca "-fno-common")            # Prevent global var merging

    # === I. The "Sanity Breakers" ===
    # add_flag_if_supported(lcca "-Wpadded")               # Struct padding warning
    add_flag_if_supported(lcca "-Wstack-usage=4096")     # 4KB Stack Limit
    # add_flag_if_supported(lcca "-Waggregate-return")     # Don't return structs

    unset(CMAKE_REQUIRED_FLAGS)
endif()

# --- 6. RUNTIME SANITIZERS ---
if (LCCA_DEVELOPER_MODE OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    # 1. Define the flags we WANT to use
    if (MSVC)
        set(WANT_SANITIZERS "/fsanitize=address")
    else()
        set(WANT_SANITIZERS "-fsanitize=address" "-fsanitize=undefined")
        if(NOT WIN32 AND NOT APPLE)
            list(APPEND WANT_SANITIZERS "-fsanitize=leak")
        endif()
    endif()

    # 2. THE CRITICAL CHECK: Can we actually link?
    include(CMakePushCheckState)
    include(CheckCSourceCompiles)
    
    cmake_push_check_state()
        # We try to compile a tiny dummy program WITH the flags
        set(CMAKE_REQUIRED_FLAGS "${WANT_SANITIZERS}")
        # MSVC requires different handling for check_c_source_compiles, 
        # but for MinGW/GCC this works perfectly.
        check_c_source_compiles("int main() { return 0; }" COMPILER_HAS_SANITIZER_LIB)
    cmake_pop_check_state()

    # 3. Apply ONLY if the check passed
    if(COMPILER_HAS_SANITIZER_LIB)
        message(STATUS "Sanitizers detected and enabled.")
        target_compile_options(lcca PRIVATE ${WANT_SANITIZERS})
        target_link_options(lcca PRIVATE ${WANT_SANITIZERS})
        target_compile_options(lcca PUBLIC ${WANT_SANITIZERS})
        target_link_options(lcca PUBLIC ${WANT_SANITIZERS})
    else()
        message(WARNING "Compiler supports sanitizer flags, but linker libraries (libasan/libubsan) are missing. Sanitizers DISABLED.")
    endif()
endif()

# --- 7. Testing Setup ---
enable_testing()
file(GLOB TEST_SOURCES "${TEST_DIR}/*.c")

foreach(test_source ${TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_executable(${test_name} ${test_source})
    
    # Link Library AND Math
    target_link_libraries(${test_name} PRIVATE lcca)
    if(NOT MSVC)
        target_link_libraries(${test_name} PRIVATE m)
    endif()
    
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
