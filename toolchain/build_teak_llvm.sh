#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Builds teak-llvm with the OpenDSP patches into toolchain/teak-llvm/build.
# Needs git, cmake, ninja, python3, clang.
set -e
COMMIT=bda74e1c0e3e6f04ae08fc5356fb4208aebb0f9e
HERE=$(cd "$(dirname "$0")" && pwd)
cd "$HERE"

if [ ! -d teak-llvm ]; then
    git clone --filter=blob:none https://github.com/Gericom/teak-llvm.git
fi
cd teak-llvm
git checkout -q "$COMMIT"
for p in "$HERE"/patches/*.patch; do
    if git apply --check "$p" 2>/dev/null; then
        git apply "$p"
    fi
done

mkdir -p build
cd build
cmake -G Ninja ../llvm -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_TARGETS_TO_BUILD="" -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=Teak \
    -DLLVM_ENABLE_PROJECTS="clang;lld" \
    -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_CXX_FLAGS="-include cstdint -include limits -Wno-everything" \
    -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF \
    -DLLVM_PARALLEL_LINK_JOBS=2
ninja clang lld llvm-readelf
