#!/bin/sh

# This file builds, runs, executes test.
# Usages
# 1. `./x build <option>`
#    Builds all files
# 2. `./x build <target> <option>`
#    Buils a target file
# 2. `./x run <target> <option>`
#    Builds the target and run the binary.
# 3. `./x test <option>`
#    Executes all test.
# 4. `./x test <target> <option>`
#    Executes all test in the `<target>_test` file.
#
# In all commands, <option> is the following option
# <option> = `-d` | ``
# If you set `-d` option, run the script as debug mode.

set -eux

# Buils executable, it has one necesary argument and one optional argument.
# 1. is_debug: if this is `debug` build as debug mode. 
# 2. target: if this argment is set, only build this executable
build() {
    local is_debug=""
    if [ $1 = "debug" ]; then
        is_debug="debug"
    fi
    local target_option=""
    if [ $# -eq 2 ]; then
        target_option="--target $2"
    fi

    cmake -DCMAKE_BUILD_TYPE=$is_debug -S . -B build
    cmake --build build $target_option
}

cmd=$1

if [ $cmd = "build" ]; then
    if [ $# -eq 1 ]; then
        build ""
    else
        target_or_option=$2
        if [ $target_or_option = "-d" ]; then
            build "debug"
        elif [ $# -eq 2 ]; then
            build "" $target_or_option
        else
            build "debug" $target_or_option
        fi
    fi
elif [ $cmd = "run" ]; then
    target=$2
    if [ $# -eq 2 ]; then
        build "" $target
    else
        build "debug" $target 
    fi
    ./build/src/$target
elif [ $cmd = "test" ]; then
    if [ $# -eq 1 ]; then
        build ""
        cd build/src && ctest
    else 
        target_with_directory_or_option=$2
        if [ $target_with_directory_or_option = "-d" ]; then
            build "debug"
            cd build/src && ctest
        elif [ $# -eq 2 ]; then
            target=${target_with_directory_or_option##*/}
            build "" "${target}_test"
            cd build/src && ./${target_with_directory_or_option}_test
        else
            target=${target_with_directory_or_option##*/}
            build "debug" "${target}_test"
            cd build/src && ./${target_with_directory_or_option}_test
        fi
    fi
fi