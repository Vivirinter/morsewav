set(LIB_SOURCES
    cli.c
    dsp.c
    morse.c
    wav.c
    progress.c
)

add_library(morsewav_lib ${LIB_SOURCES})

# Decide on static/shared based on BUILD_SHARED_LIBS
# Use C23 unless building on Windows (MSVC/clang-cl)
if (WIN32)
    set(lib_c_std 17)
else()
    set(lib_c_std 23)
endif()
set_target_properties(morsewav_lib PROPERTIES
    OUTPUT_NAME morsewav
    C_STANDARD ${lib_c_std}
    C_STANDARD_REQUIRED YES
    POSITION_INDEPENDENT_CODE ON
)

# Public include directory so that users can #include <cli.h>, etc.

target_include_directories(morsewav_lib
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# Compile warnings
if(ENABLE_WARNINGS)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(morsewav_lib PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()
