#!/bin/bash

set -euo pipefail

# usage:
# - lint the modified files: `bash tools/clang-tidy.sh modified`
# - lint all the files: `bash tools/clang-tidy.sh all`
# - force-refresh compile_commands.json: `bash tools/clang-tidy.sh --refresh-db [all|modified]`

root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/.."
build_dir="${root}/build"
compile_commands="${build_dir}/compile_commands.json"

mode="modified"
refresh_db=0
worker_file=""
base_ref="${CLANG_TIDY_BASE_REF:-origin/main}"
for arg in "$@"; do
    case "$arg" in
        all|modified)
            mode="$arg"
            ;;
        --refresh-db)
            refresh_db=1
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

configure_compile_db() {
    mkdir -p "${build_dir}"

    if [ -f "${root}/compile_commands.json" ] && [ ! -f "${compile_commands}" ]; then
        mv "${root}/compile_commands.json" "${compile_commands}"
    fi

    if [ "${refresh_db}" -eq 1 ] || [ ! -f "${compile_commands}" ]; then
        echo "Generating compile_commands.json..."
        cmake -S "${root}" -B "${build_dir}" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    else
        echo "Reusing ${compile_commands}"
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
    clang-tidy "${clang_tidy_args[@]}" "${file}"
}

collect_all_targets() {
    find "${root}/src" "${root}/tests" \
        -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' \) -print0
}

collect_modified_targets() {
    local file
    local diff_base=""

    if git -C "${root}" rev-parse --verify --quiet "${base_ref}" >/dev/null; then
        diff_base="$(git -C "${root}" merge-base HEAD "${base_ref}")"
    fi

    {
        if [ -n "${diff_base}" ]; then
            git -C "${root}" diff --name-only --diff-filter=ACMR "${diff_base}"...HEAD -- src tests
        else
            git -C "${root}" diff --name-only --diff-filter=ACMR HEAD -- src tests
        fi
        git -C "${root}" ls-files --others --exclude-standard -- src tests
    } | awk 'NF && !seen[$0]++' | while IFS= read -r file; do
        case "$file" in
            *.c|*.cc|*.cpp)
                printf '%s\n' "${root}/${file}"
                ;;
            *.h|*.hpp)
                local header_basename
                header_basename="$(basename "$file")"
                rg -l \
                    --glob '*.{c,cc,cpp}' \
                    -e "#include\\s*[<\"]${header_basename}[>\"]" \
                    -e "#include\\s*[<\"].*${file}[>\"]" \
                    "${root}/src" "${root}/tests" 2>/dev/null || true
                ;;
        esac
    done | awk 'NF && !seen[$0]++'
}

run_target_stream() {
    local first_target

    if ! IFS= read -r first_target; then
        echo "No files to lint."
        return
    fi

    {
        printf '%s\n' "${first_target}"
        cat
    } | xargs -P "${jobs}" -I {} "$0" --worker-file={}
}

run_all_targets() {
    collect_all_targets | xargs -0 -P "${jobs}" -I {} "$0" --worker-file={}
}

if [ -n "${worker_file}" ]; then
    process_file "${worker_file}"
    exit 0
fi

configure_compile_db

if [ "${mode}" = "all" ]; then
    echo "Linting all translation units with ${jobs} jobs..."
    run_all_targets
else
    if git -C "${root}" rev-parse --verify --quiet "${base_ref}" >/dev/null; then
        echo "Linting translation units changed since merge-base with ${base_ref} using ${jobs} jobs..."
    else
        echo "Base ref ${base_ref} not found; linting locally modified translation units with ${jobs} jobs..."
    fi
    collect_modified_targets | run_target_stream
fi
