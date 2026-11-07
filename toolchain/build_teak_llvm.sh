#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Build the pinned Teak compiler.
# Needs git, cmake, ninja, python3, clang.
set -e
REPOSITORY=https://github.com/Cellenseres/teak-llvm.git
COMMIT=20941665daada9de2f5a3ba8f62ee71390b198e3
HERE=$(cd "$(dirname "$0")" && pwd)
cd "$HERE"

if [ ! -d teak-llvm ]; then
    git clone --filter=blob:none --no-checkout "$REPOSITORY"
elif ! git -C teak-llvm diff --quiet || ! git -C teak-llvm diff --cached --quiet; then
    echo "Compiler checkout has local changes. Preserve them and use a fresh checkout." >&2
    exit 1
fi
cd teak-llvm
git fetch --depth=1 "$REPOSITORY" "$COMMIT"
git checkout --detach -q "$COMMIT"
sh opendsp/build.sh
