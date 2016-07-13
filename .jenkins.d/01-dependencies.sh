#!/usr/bin/env bash
set -e

JDIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source "$JDIR"/util.sh

if has OSX $NODE_LABELS && ! has OSX-10.8-c++11-64bit $NODE_LABELS; then
    # OSX 10.8 requires special handling of dependencies
    set -x
    brew update
    brew upgrade
    brew install boost pkg-config sqlite cryptopp log4cxx protobuf openssl
    brew link --force openssl log4cxx protobuf
    brew cleanup
fi

if has Ubuntu $NODE_LABELS; then
    BOOST_PKG=libboost-all-dev
    if has Ubuntu-12.04 $NODE_LABELS; then
        BOOST_PKG=libboost1.48-all-dev
    fi

    set -x
    sudo apt-get update -qq -y
    sudo apt-get -qq -y install build-essential pkg-config $BOOST_PKG \
                                libcrypto++-dev libsqlite3-dev \
                                liblog4cxx10-dev protobuf-compiler libprotobuf-dev libssl-dev
fi
