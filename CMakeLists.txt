cmake_minimum_required(VERSION 3.21)

project(tinyfseq
        VERSION 1.2.0
        DESCRIPTION "A tiny library for decoding FSEQ (.fseq) v2.0+ sequence files"
        HOMEPAGE_URL "https://github.com/Cryptkeeper/libtinyfseq"
        LANGUAGES C)

set(CMAKE_C_STANDARD 90)

# enables you to disable replace all error strings returned by `tf_err_str` with "NULL"
# this reduces the compiled size of the object + memory footprint
#
# you may also `#define TF_STRIP_ERR_STRINGS` prior to including `tinyfseq.h` to disable
#       error strings without using CMake
set(TF_INCLUDE_ERR_STRINGS true)

if (NOT ${TF_INCLUDE_ERR_STRINGS})
        add_compile_definitions(TF_STRIP_ERR_STRINGS)
endif ()

add_library(tinyfseq tinyfseq.h tinyfseq.c)

set_target_properties(tinyfseq PROPERTIES PUBLIC_HEADER "tinyfseq.h")

install(TARGETS tinyfseq
        LIBRARY DESTINATION lib/tinyfseq
        PUBLIC_HEADER DESTINATION include/tinyfseq)
