# NxfWarnings.cmake - one warning policy for every NXF target.
# Warnings are configuration, not decoration: keep this list short,
# justified, and shared so modules cannot silently drift apart.

function(nxf_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()
