#!/bin/sh

# This file builds, runs, execute test.
# Usages
# 1. `./x build`
#    Builds all files
# 2. `./x run <target>`
#    Builds the target and run the binary.
# 3. `./x test`
#    Executes all test.
# 4. `./x test <target>`
#    Executes all test in the `<target>_test` file.

set -eux

# Buils executable, it has one optional argument
# 1. target: if this argment is set, only build this executable
build() {
    local target_option=""
    if [ $# -eq 1 ]; then
        target_option="--target $1"
    fi

    cmake -S . -B build
    cmake --build build $target_option
}

cmd=$1

if [ $cmd = "build" ]; then
    build
elif [ $cmd = "run" ]; then
    target=$2
    build $target
    ./build/src/$target
elif [ $cmd = "test" ]; then
    if [ $# -eq 1 ]; then
        build 
        cd build/src && ctest
    else 
        target_with_directory=$2
        target=${target_with_directory##*/}
        build "${target}_test"
        cd build/src && ./${target_with_directory}_test
    fi
fi