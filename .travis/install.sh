#!/bin/bash

set -eu
set -o pipefail

mkdir -p "${INSTALL_DIR}"

############################################################################
# Update Homebrew (OS X)
############################################################################
if [[ "${TRAVIS_OS_NAME}" == "osx" ]]; then
    brew update;
fi

############################################################################
# Install a recent CMake (unless already installed on OS X)
############################################################################
if [[ "${TRAVIS_OS_NAME}" == "linux" ]]; then
    echo "Downloading CMake binary..."
    CMAKE_URL=https://cmake.org/files/v3.1/cmake-3.1.3-Linux-x86_64.tar.gz
    wget --no-check-certificate --quiet -O - "${CMAKE_URL}" | tar --strip-components=1 -xz -C "${INSTALL_DIR}"
else
    if ! brew ls --version cmake &>/dev/null; then
        brew install cmake
    fi
fi

############################################################################
# Install the right versions of Protocol Buffers and 0MQ
############################################################################
if [[ "${TRAVIS_OS_NAME}" == "linux" ]]; then
    PROTO_VERSION=2.6.1
    ZMQ_VERSION=4.1.6
    PROTO_URL=https://github.com/google/protobuf/releases/download/v${PROTO_VERSION}/protobuf-${PROTO_VERSION}.tar.gz
    ZMQ_URL=https://github.com/zeromq/zeromq4-1/releases/download/v${ZMQ_VERSION}/zeromq-${ZMQ_VERSION}.tar.gz

    PROTO_SRC_DIR=${TRAVIS_BUILD_DIR}/deps/proto
    ZMQ_SRC_DIR=${TRAVIS_BUILD_DIR}/deps/zmq
    mkdir -p "${PROTO_SRC_DIR}" "${ZMQ_SRC_DIR}"

    echo "Installing Protoco Buffers from sources..."
    wget --no-check-certificate --quiet -O - "${PROTO_URL}" | tar --strip-components=1 -xz -C "${PROTO_SRC_DIR}"
    (cd "${PROTO_SRC_DIR}" && ./configure --prefix="${INSTALL_DIR}" && make && make install)

    echo "Installing ZMQ from sources..."
    wget --no-check-certificate --quiet -O - "${ZMQ_URL}"   | tar --strip-components=1 -xz -C "${ZMQ_SRC_DIR}"
    (cd "${ZMQ_SRC_DIR}"   && ./configure --prefix="${INSTALL_DIR}" && make && make install)
else
    brew install zeromq
    brew install protobuf
fi
