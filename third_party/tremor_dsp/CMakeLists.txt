cmake_minimum_required(VERSION 3.13)
project(tremor_dsp C)
option(TREMOR_WORD_STORAGE "Use word storage" OFF)
add_library(tremor_dsp STATIC bitwise.c codebook.c dsp.c floor0.c floor1.c
    floor_lookup.c info.c mdct.c res012.c mapping0.c)
target_include_directories(tremor_dsp PUBLIC . ../config/ogg)
if(TREMOR_WORD_STORAGE)
    target_compile_definitions(tremor_dsp PUBLIC TREMOR_WORD_STORAGE=1)
endif()
