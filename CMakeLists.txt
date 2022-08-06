cmake_minimum_required(VERSION 3.21)

project(tinyfseq
        VERSION 1.0.1
        DESCRIPTION "A tiny library for decoding FSEQ (.fseq) v2.0+ sequence files"
        HOMEPAGE_URL "https://github.com/Cryptkeeper/libtinyfseq"
        LANGUAGES C)

set(CMAKE_C_STANDARD 90)

set(TF_INCLUDE_ERR_STRINGS true)

configure_file(${PROJECT_SOURCE_DIR}/build.h.in ${PROJECT_SOURCE_DIR}/build.h)

add_library(tinyfseq tinyfseq.h tinyfseq.c)

set_target_properties(tinyfseq PROPERTIES PUBLIC_HEADER "tinyfseq.h;build.h")

install(TARGETS tinyfseq
        LIBRARY DESTINATION lib/tinyfseq
        PUBLIC_HEADER DESTINATION include/tinyfseq)
