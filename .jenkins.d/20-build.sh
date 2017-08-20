#!/usr/bin/env bash
set -e

JDIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source "$JDIR"/util.sh

set -x

git submodule init
git submodule sync
git submodule update

# Cleanup
sudo env "PATH=$PATH" ./waf --color=yes distclean

# Configure/build in debug mode
./waf -j1 --color=yes configure --with-tests --debug
./waf -j1 --color=yes build

# Cleanup
sudo env "PATH=$PATH" ./waf --color=yes distclean

# Configure/build in optimized mode without tests
./waf -j1 --color=yes configure
./waf -j1 --color=yes build

# Cleanup
sudo env "PATH=$PATH" ./waf --color=yes distclean

if [[ $JOB_NAME == *"code-coverage" ]]; then
    COVERAGE="--with-coverage"
elif [[ -n $BUILD_WITH_ASAN || -z $TRAVIS ]]; then
    ASAN="--with-sanitizer=address"
fi

# Configure/build in optimized mode with tests
./waf -j1 --color=yes configure --with-tests $COVERAGE $ASAN
./waf -j1 --color=yes build

# (tests will be run against optimized version)

# Install
sudo env "PATH=$PATH" ./waf --color=yes install
