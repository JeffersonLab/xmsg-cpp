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
    brew install ccache
    brew install zeromq
    if [[ "$(brew config | grep macOS)" =~ 10.10 ]]; then
        brew install protobuf@2.6
        brew link --force protobuf@2.6
    else
        brew install protobuf
    fi
fi
