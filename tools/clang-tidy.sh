#!/bin/bash

set -euo pipefail

# usage:
# - lint all the files: `bash tools/clang-tidy.sh all`
# - lint one file: `bash tools/clang-tidy.sh --worker-file=<file>`

root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/.."
build_dir="${root}/build"
compile_commands="${build_dir}/compile_commands.json"

worker_file=""
for arg in "$@"; do
    case "$arg" in
        all)
            ;;
        --worker-file=*)
            worker_file="${arg#--worker-file=}"
            ;;
        *)
            echo "Unknown argument: $arg" >&2
            exit 1
            ;;
    esac
done

jobs="${CLANG_TIDY_JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || getconf _NPROCESSORS_ONLN || echo 4)}"
ignore_files_regex='test_switch\.h$'
clang_tidy_bin="${CLANG_TIDY_BIN:-}"
clang_bin="${CLANG_BIN:-}"
clangxx_bin="${CLANGXX_BIN:-}"

resolve_clang_tidy() {
    local candidate=""

    if [ -n "${clang_tidy_bin}" ] && [ -x "${clang_tidy_bin}" ]; then
        return
    fi

    candidate="$(command -v clang-tidy 2>/dev/null || true)"
    if [ -n "${candidate}" ]; then
        clang_tidy_bin="${candidate}"
        return
    fi

    for candidate in \
        "/opt/homebrew/opt/llvm/bin/clang-tidy" \
        "/usr/local/opt/llvm/bin/clang-tidy" \
        "$(brew --prefix llvm 2>/dev/null)/bin/clang-tidy"
    do
        if [ -n "${candidate}" ] && [ -x "${candidate}" ]; then
            clang_tidy_bin="${candidate}"
            return
        fi
    done

    echo "clang-tidy not found. Set CLANG_TIDY_BIN or add LLVM's bin directory to PATH." >&2
    exit 1
}

resolve_clang_compilers() {
    local brew_prefix=""

    if [ -z "${clang_bin}" ] || [ ! -x "${clang_bin}" ] || [ -z "${clangxx_bin}" ] || [ ! -x "${clangxx_bin}" ]; then
        brew_prefix="$(brew --prefix llvm 2>/dev/null || true)"
    fi

    if [ -z "${clang_bin}" ] || [ ! -x "${clang_bin}" ]; then
        for candidate in \
            "/opt/homebrew/opt/llvm/bin/clang" \
            "/usr/local/opt/llvm/bin/clang" \
            "${brew_prefix}/bin/clang"
        do
            if [ -n "${candidate}" ] && [ -x "${candidate}" ]; then
                clang_bin="${candidate}"
                break
            fi
        done
    fi

    if [ -z "${clangxx_bin}" ] || [ ! -x "${clangxx_bin}" ]; then
        for candidate in \
            "/opt/homebrew/opt/llvm/bin/clang++" \
            "/usr/local/opt/llvm/bin/clang++" \
            "${brew_prefix}/bin/clang++" \
            "$(llvm-config --bindir)/clang++"
        do
            if [ -n "${candidate}" ] && [ -x "${candidate}" ]; then
                clangxx_bin="${candidate}"
                break
            fi
        done
    fi
}

configure_compile_db() {
    mkdir -p "${build_dir}"

    if [ -f "${root}/compile_commands.json" ] && [ ! -f "${compile_commands}" ]; then
        mv "${root}/compile_commands.json" "${compile_commands}"
    fi

    echo "Generating compile_commands.json..."
    if [ -n "${clang_bin}" ] && [ -n "${clangxx_bin}" ]; then
        echo "Using C compiler: ${clang_bin}"
        echo "Using CXX compiler: ${clangxx_bin}"
        CC="${clang_bin}" CXX="${clangxx_bin}" \
            cmake -S "${root}" -B "${build_dir}" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    else
        cmake -S "${root}" -B "${build_dir}" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    fi
}

declare -a clang_tidy_args=(
    -p "${build_dir}"
    --config-file="${root}/.clang-tidy"
    --warnings-as-errors=*
    --header-filter="${root}/(src|tests)/.*"
    --system-headers=0
    --use-color
    -extra-arg=-I"${root}/src"
    -extra-arg=-I"${root}/src/engines"
    -extra-arg=-I"${root}/src/gears"
    -extra-arg=-I"${root}/src/types"
    -extra-arg=-I"${root}/src/misc"
    -extra-arg=-I"${root}/src/patch/quickjs"
    -extra-arg=-I"${root}/tests"
    -extra-arg=-isystem"${root}/../../src"
    -extra-arg=-isystem"${root}/../../build/src"
    -extra-arg=-isystem"${root}/../../include"
    -extra-arg=-isystem"${root}/../../include/glog"
    -extra-arg=-isystem"${root}/thirdparty/quickjs"
    -extra-arg=-isystem"${root}/thirdparty/cpp-subprocess"
    -extra-arg=-isystem/usr/local/include
    -extra-arg=-stdlib=libc++
    -extra-arg=-D_ENABLE_JAVASCRIPTCORE
    -extra-arg=-D_GNU_SOURCE
    '-extra-arg=-DGLOG_EXPORT=__attribute__((visibility("default")))'
    '-extra-arg=-DGLOG_NO_EXPORT=__attribute__((visibility("default")))'
    '-extra-arg=-DGLOG_DEPRECATED=__attribute__((deprecated))'
)

process_file() {
    local file="$1"

    if [[ "${file}" =~ ${ignore_files_regex} ]]; then
        echo "Ignoring ${file}..."
        return
    fi

    echo "Processing ${file}..."
    "${clang_tidy_bin}" "${clang_tidy_args[@]}" "${file}"
}

collect_all_targets() {
    find "${root}/src" "${root}/tests" \
        -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' \) -print0
}

run_all_targets() {
    collect_all_targets | xargs -0 -P "${jobs}" -I {} "$0" --worker-file={}
}

resolve_clang_tidy

if [ -n "${worker_file}" ]; then
    process_file "${worker_file}"
    exit 0
fi

echo "Using clang-tidy: ${clang_tidy_bin}"
"${clang_tidy_bin}" --version
echo "clang-tidy arguments:"
printf '  %q\n' "${clang_tidy_args[@]}"


resolve_clang_compilers

echo "Using clangxx: ${clangxx_bin}"
"${clangxx_bin}" --version

configure_compile_db

echo "Linting all translation units with ${jobs} jobs..."
run_all_targets
