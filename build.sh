#!/bin/bash

COMPILE_MODE="Debug"
C_COMPILER="/usr/bin/gcc"
CXX_COMPILER="/usr/bin/g++"
CMAKE_C_FLAGS_DEBUG=""
CMAKE_CXX_FLAGS_DEBUG=""
EXT_DEFINE=""

case $1 in
  "help")
    echo "Usage:"
    echo "  ./build.sh -- Release build."
    echo "  ./build.sh debug -- Debug build."
    exit 0
  ;;
  "debug")
    COMPILE_MODE="Debug" 
    CMAKE_C_FLAGS_DEBUG="-gdwarf-2"
    CMAKE_CXX_FLAGS_DEBUG="-gdwarf-2"
    EXT_DEFINE="-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
  ;;
  *)
    COMPILE_MODE="Release" 
  ;;
esac

rm -rf ./build ./autobuild && mkdir ./build && mkdir ./autobuild  && cd ./autobuild
cmake -DCMAKE_BUILD_TYPE=${COMPILE_MODE} -DCMAKE_C_COMPILER=${C_COMPILER} -DCMAKE_CXX_COMPILER=${CXX_COMPILER} -DCMAKE_C_FLAGS_DEBUG=${CMAKE_C_FLAGS_DEBUG} -DCMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}  ${EXT_DEFINE}  ../src/qphi
make 
