#!/usr/bin/env bash

mkdir build && cd build
if [ `uname` == Darwin ]; then
    ${BUILD_PREFIX}/bin/cmake \
        .. \
        -DCMAKE_INSTALL_PREFIX=${PREFIX} \
        -DCMAKE_OSX_SYSROOT=${CONDA_BUILD_SYSROOT} \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER=${CLANG} \
        -DCMAKE_CXX_COMPILER=${CLANGXX} \
        -DCMAKE_C_FLAGS="${CFLAGS} ${OPTS}" \
        -DCMAKE_CXX_FLAGS="${CXXFLAGS} ${OPTS}" \
        -DCMAKE_VERBOSE_MAKEFILE=TRUE \
        -DUSE_MKL=TRUE \
        -DENABLE_TEST=TRUE
fi
if [ `uname` == Linux ]; then
    ${BUILD_PREFIX}/bin/cmake \
        .. \
        -DCMAKE_INSTALL_PREFIX=${PREFIX} \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER=${GCC} \
        -DCMAKE_CXX_COMPILER=${GXX} \
        -DCMAKE_C_FLAGS="${CFLAGS} ${OPTS}" \
        -DCMAKE_CXX_FLAGS="${CXXFLAGS} ${OPTS}" \
        -DCMAKE_VERBOSE_MAKEFILE=TRUE \
        -DUSE_MKL=TRUE \
        -DENABLE_TEST=TRUE
fi

make -j ${CPU_COUNT}
make test ARGS=-V
make install

