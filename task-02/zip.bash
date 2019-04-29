#!/usr/bin/env bash

fail_with_msg() {
    echo "$@"
    exit 1
}

check() {
    local forbid_content=("todo" "pidr" "govno" "zhopa" "hui" "sosi" "smelyanskiy")

    if [ -d ".git" ]; then
        fail_with_msg "Found .git directory!"
    fi

    for it in "${forbid_content[@]}"; do
        if grep -rn . -ie $it; then
            fail_with_msg "Found ${it}!"
        fi
    done

    local forbid_file_type=(zip pdf)

    for it in "${forbid_file_type[@]}"; do
        if find . -name "*.$it"; then
            fail_with_msg "Found ${it}!"
        fi
    done
}

check

