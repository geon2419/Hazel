#!/usr/bin/env bash
set -euo pipefail

repo_root="$(git rev-parse --show-toplevel)"
cd "$repo_root"

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
    echo "pre-commit: clang-format not found on PATH (detected OS: $os_name)"
    echo "Install LLVM clang-format and retry."
    exit 1
fi

declare -a target_files=()
while IFS= read -r -d '' path; do
    case "$path" in
        Hazel/src/* | Hazel-Test/src/* | Sandbox/src/*)
            case "$path" in
                *.h | *.hpp | *.hh | *.cpp | *.cc | *.cxx | *.inl)
                    target_files+=("$path")
                    ;;
            esac
            ;;
    esac
done < <(git diff --cached --name-only --diff-filter=ACMR -z)

if [ "${#target_files[@]}" -eq 0 ]; then
    echo "pre-commit: no staged C/C++ source files to lint (detected OS: $os_name)"
    exit 0
fi

declare -a violations=()
for file in "${target_files[@]}"; do
    if [ ! -f "$file" ]; then
        continue
    fi

    if ! clang-format --dry-run --Werror --style=file --fallback-style=none "$file" >/dev/null 2>&1; then
        violations+=("$file")
    fi
done

if [ "${#violations[@]}" -gt 0 ]; then
    echo "pre-commit: formatting violations found (detected OS: $os_name)"
    for file in "${violations[@]}"; do
        echo "  - $file"
    done
    exit 1
fi

echo "pre-commit: lint passed for ${#target_files[@]} staged file(s) (detected OS: $os_name)"
exit 0
