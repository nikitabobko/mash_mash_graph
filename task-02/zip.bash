#!/usr/bin/env bash
set -e

fail_with_msg() {
    echo "$@"
    exit 1
}

check() {
    local forbid_content=("todo" "pidr" "govno" "zhopa" "hui" "sosi" "smelyanskiy")

    for it in "${forbid_content[@]}"; do
        if grep -rn . -ie $it; then
            fail_with_msg "Found ${it}!"
        fi
    done

    if [ ! -f "screen01.jpg" ]; then
        fail_with_msg "No screenshot found!"
    fi
}

check

init_dir=$(pwd)

work_dir=$(mktemp -d)/323_бобко_никита
mkdir -p $work_dir
cd $work_dir

rm -rf .git zip.bash .idea cmake-build-debug build
find . -name *.pdf | while read file; do rm -rf $file; done
find . -name *.zip | while read file; do rm -rf $file; done
find . -name *.blend | while read file; do rm -rf $file; done

mkdir build
cd build
cmake ..
make
./main
cd ..

rm -rf build
cd ..
zip -r 323_бобко_никита.zip 323_бобко_никита
cp 323_бобко_никита.zip $init_dir
rm -rf *
