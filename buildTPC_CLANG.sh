#!/bin/bash

build_tpc_llvm_helper()
{
        echo -e "\nusage: buildTPC_LLVM.sh [options]\n"

        echo -e "options:\n"
        echo -e "  -r,  --release              Build only release build"
        echo -e "  -j,  --jobs <val>           Overwrite number of jobs"
        echo -e "  -h,  --help                 Prints this help"
}

build_tpc_llvm ()
{
    SECONDS=0

    local __jobs=$NUMBER_OF_JOBS
    local __color="ON"
    local __debug="yes"
    local __release=""
    local __all=""
    local __org_configure=""
    local __build_res=""
    local __linker="-DLLVM_USE_LINKER=gold"
    local __build_command="clang"
    local __makefile_gen="make"
    __targetToBuild="TPC"
    (cmake\
        -G "Unix Makefiles" \
        -DLLVM_TARGETS_TO_BUILD=TPC \
        -DLLVM_BUILD_EXAMPLES=OFF \
        -DLLVM_INCLUDE_EXAMPLES=OFF \
        -DCLANG_ENABLE_ARCMT=OFF \
        -DCLANG_BUILD_EXAMPLES=OFF \
        -DCMAKE_BUILD_TYPE="Release" \
        -DLLVM_ENABLE_PROJECTS=clang \
        $SCRIPT_DIR"/llvm")

        make -j ${__jobs} clang llvm-objdump
        cd ${BUILD_DIR}
        __build_res=$?

    printf "\nElapsed time: %02u:%02u:%02u \n\n" $(($SECONDS / 3600)) $((($SECONDS / 60) % 60)) $(($SECONDS % 60))
    return 0
}

BUILD_DIR=$(pwd)
SCRIPT_DIR=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)
__debug="Debug"
EXIT="No"
while [ -n "$1" ];
do
    case $1 in
    -r  | --release )
        __debug="Release"
        ;;
    -h  | --help )
        build_tpc_llvm_helper
        EXIT="Yes"
        ;;
    -j  | --jobs )
        shift
        __jobs=$1
        ;;
    *)
        echo "The parameter $1 is not allowed"
        build_tpc_llvm_helper
        EXIT="Yes"
        ;;
    esac
    shift
done
if [ $EXIT == "No" ]; then
    echo "Starting build TPC-LLVM"
    build_tpc_llvm
    echo "End build TPC-LLVM"
fi
