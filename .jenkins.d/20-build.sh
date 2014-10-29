#!/usr/bin/env bash
set -x
set -e

git submodule init
git submodule sync
git submodule update

COVERAGE=$( python -c "print '--with-coverage' if 'code-coverage' in '$JOB_NAME' else ''" )

# Cleanup
sudo ./waf -j1 --color=yes distclean

# Configure/build in debug mode
./waf -j1 --color=yes configure --with-tests --debug
./waf -j1 --color=yes build

# Cleanup
sudo ./waf -j1 --color=yes distclean

# Configure/build in optimized mode without tests
./waf -j1 --color=yes configure
./waf -j1 --color=yes build

# Cleanup
sudo ./waf -j1 --color=yes distclean

# Configure/build in optimized mode with tests
./waf -j1 --color=yes configure --with-tests $COVERAGE
./waf -j1 --color=yes build

# (tests will be run against optimized version)

./waf configure --color=yes --with-tests $COVERAGE

./waf -j1 --color=yes
sudo ./waf install --color=yes
