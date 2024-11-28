add_library(
    contrib-malloc STATIC
    EXCLUDE_FROM_ALL

    dlea_malloc/malloc.c
    dlea_malloc/wrapper.cpp
)

set_source_files_properties(dlea_malloc/malloc.c PROPERTIES COMPILE_FLAGS -Wno-maybe-uninitialized)

target_compile_definitions(
    contrib-malloc PRIVATE

    USE_DL_PREFIX USE_LOCKS=1
    MORECORE_CONTIGUOUS HAVE_MMAP=0
    LACKS_TIME_H LACKS_SCHED_H
)

add_library(
    contrib-printf STATIC
    EXCLUDE_FROM_ALL

    mpaland_printf/printf.c
)

target_link_libraries(contrib-printf gcc)
