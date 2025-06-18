cmake_minimum_required(VERSION 3.16)

# ========================
#  morsewav project
# ========================
project(morsewav VERSION 0.1 LANGUAGES C)

# Generate version header
configure_file(${PROJECT_SOURCE_DIR}/version.h.in ${PROJECT_BINARY_DIR}/version.h @ONLY)

# Options ----------------------------------------------------------
option(BUILD_SHARED_LIBS "Build libmorsewav as a shared library" OFF)
option(ENABLE_SANITIZERS  "Enable Address/Undefined sanitizers"  OFF)
option(ENABLE_WARNINGS    "Enable extra compiler warnings"       ON)

# Sub-directories --------------------------------------------------
add_subdirectory(lib)

# Executable -------------------------------------------------------
add_executable(morsewav morsewav_plus.c)

# Include path for private headers (public headers are exposed by the library)
# Keep executable self-contained: no other include dirs required

target_link_libraries(morsewav PRIVATE morsewav_lib m)

target_include_directories(morsewav PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
)

# Compiler flags ---------------------------------------------------
if(ENABLE_WARNINGS)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
        set(WARN_FLAGS "-Wall" "-Wextra" "-Wpedantic")
        target_compile_options(morsewav PRIVATE ${WARN_FLAGS})
        target_compile_options(morsewav_lib PRIVATE ${WARN_FLAGS})
    endif()
endif()

if(ENABLE_SANITIZERS)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(morsewav PRIVATE -fsanitize=address,undefined)
        target_link_options(morsewav PRIVATE -fsanitize=address,undefined)
        target_compile_options(morsewav_lib PRIVATE -fsanitize=address,undefined)
        target_link_options(morsewav_lib PRIVATE -fsanitize=address,undefined)
    endif()
endif()

# C Standard -------------------------------------------------------
# Use C23 everywhere except Windows (MSVC/clang-cl still lack full support).
if (WIN32)
    set(c_std 17)
else()
    set(c_std 23)
endif()
set_target_properties(morsewav PROPERTIES
    C_STANDARD ${c_std}
    C_STANDARD_REQUIRED YES
)

# Installation -----------------------------------------------------
install(TARGETS morsewav
        EXPORT morsewavTargets
        RUNTIME DESTINATION bin)
install(TARGETS morsewav_lib
        EXPORT morsewavTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
install(DIRECTORY lib/ DESTINATION include FILES_MATCHING PATTERN "*.h")
install(FILES ${PROJECT_BINARY_DIR}/version.h DESTINATION include)

# Package config (export targets) ----------------------------------
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/morsewavConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
export(EXPORT morsewavTargets FILE morsewavTargets.cmake NAMESPACE morsewav::)
install(EXPORT morsewavTargets FILE morsewavTargets.cmake NAMESPACE morsewav:: DESTINATION lib/cmake/morsewav)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/morsewavConfigVersion.cmake DESTINATION lib/cmake/morsewav)
