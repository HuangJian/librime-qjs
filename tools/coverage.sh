#!/bin/bash

dirQjs="$(cd "$(dirname "$0")" && pwd)/.."
dirRime="$dirQjs/../.."

rm -rf ${dirQjs}/build/qjs.*
rm -rf ${dirQjs}/build/coverage

cd $dirRime

cmake . -Bcoverage \
    -G Ninja \
    -DENABLE_COVERAGE=ON \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_MERGED_PLUGINS=OFF \
    -DALSO_LOG_TO_STDERR=OFF \
    -DENABLE_EXTERNAL_PLUGINS=ON

cmake --build coverage -j

profile="qjs"

cd ${dirQjs}/build

## FIXME: a lot of the files under "src/types" are not shown in the coverage report.
LLVM_PROFILE_FILE="${profile}.profraw" ./librime-qjs-tests

llvm-profdata merge -sparse ${profile}.profraw -o ${profile}.profdata

llvm-cov show \
    -instr-profile=${profile}.profdata \
    ./librime-qjs-tests \
    -format=html \
    -ignore-filename-regex='.*/librime/(include|src)/.*' \
    -ignore-filename-regex='.*boost.*' \
    -ignore-filename-regex='.*(tests|thirdparty).*' \
    --output-dir=${dirQjs}/build/coverage
