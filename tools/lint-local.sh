#!/usr/bin/env bash
set -euo pipefail

repo_root="$(git rev-parse --show-toplevel)"
cd "$repo_root"

mode="fix"
if [ "${1:-}" = "--check" ]; then
    mode="check"
elif [ "${1:-}" = "--fix" ] || [ -z "${1:-}" ]; then
    mode="fix"
elif [ "${1:-}" = "--help" ] || [ "${1:-}" = "-h" ]; then
    echo "Usage: ./tools/lint-local.sh [--check|--fix]"
    echo "  --check : validate formatting only (no file changes)"
    echo "  --fix   : apply clang-format to all target files (default)"
    exit 0
else
    echo "local lint: unknown option '${1:-}'"
    echo "Usage: ./tools/lint-local.sh [--check|--fix]"
    exit 2
fi

uname_output="$(uname -s)"
case "$uname_output" in
    Darwin*)
        os_name="macOS"
        ;;
    Linux*)
        os_name="Linux"
        ;;
    MINGW* | MSYS* | CYGWIN*)
        os_name="Windows (Git Bash)"
        ;;
    *)
        os_name="$uname_output"
        ;;
esac

if ! command -v clang-format >/dev/null 2>&1; then
    echo "local lint: clang-format not found on PATH (detected OS: $os_name)"
    echo "Install LLVM clang-format and retry."
    exit 1
fi

declare -a target_files=()
while IFS= read -r -d '' path; do
    case "$path" in
        *.h | *.hpp | *.hh | *.cpp | *.cc | *.cxx | *.inl)
            target_files+=("$path")
            ;;
    esac
done < <(find Hazel/src Hazel-Test/src Sandbox/src -type f -print0)

if [ "${#target_files[@]}" -eq 0 ]; then
    echo "local lint: no C/C++ source files found (detected OS: $os_name)"
    exit 0
fi

if [ "$mode" = "check" ]; then
    declare -a violations=()
    for file in "${target_files[@]}"; do
        if ! clang-format --dry-run --Werror --style=file --fallback-style=none "$file" >/dev/null 2>&1; then
            violations+=("$file")
        fi
    done

    if [ "${#violations[@]}" -gt 0 ]; then
        echo "local lint(check): formatting violations found (detected OS: $os_name)"
        for file in "${violations[@]}"; do
            echo "  - $file"
        done
        exit 1
    fi

    echo "local lint(check): passed for ${#target_files[@]} file(s) (detected OS: $os_name)"
    exit 0
fi

for file in "${target_files[@]}"; do
    clang-format -i --style=file --fallback-style=none "$file"
done

echo "local lint(fix): formatted ${#target_files[@]} file(s) (detected OS: $os_name)"
exit 0
