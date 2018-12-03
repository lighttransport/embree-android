#!/bin/bash

CMAKE_BIN=cmake

rm -rf build

$CMAKE_BIN \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DEMBREE_ADDRESS_SANITIZER=Off \
  -DCMAKE_INSTALL_PREFIX=$HOME/local/embree3 \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DEMBREE_ISPC_SUPPORT=Off \
  -DEMBREE_TASKING_SYSTEM=Internal \
  -DEMBREE_TUTORIALS=Off \
  -DEMBREE_MAX_ISA=SSE2 \
  -DEMBREE_RAY_PACKETS=Off \
  -Bbuild -H.
