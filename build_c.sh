#!/bin/sh
export LD_PRELOAD=
CFLAG=""
CFLAG+=" -DNDEBUG"
CFLAG+=" -O3"
CFLAG+=" -mlzcnt"
CFLAG+=" -mbmi"
CFLAG+=" -lpthread"
CFLAG+=" -latomic"
CFLAG+=" -shared"
CFLAG+=" -fpic"
CFLAG+=" -fno-builtin-malloc"
CFLAG+=" -fno-builtin-free"
CFLAG+=" -fno-builtin-calloc"
CFLAG+=" -fno-builtin-realloc"
CFLAG+=" -fno-builtin-posix_memalign"
CFLAG+=" -fno-builtin-aligned_alloc"
CFLAG+=" -fno-builtin-valloc"
CFLAG+=" -fno-builtin-memalign"
CFLAG+=" -fno-builtin-pvalloc"
CFLAG+=" -fno-builtin-malloc_usable_size"
CFLAG+=" -std=c++14"
clang++ -DKANAMESHIKI_HEAP_SPECIALIZATION=1 src/KanameShiki.cpp ${CFLAG} -o KanameShiki1_c.so
clang++ -DKANAMESHIKI_HEAP_SPECIALIZATION=0 src/KanameShiki.cpp ${CFLAG} -o KanameShiki0_c.so
