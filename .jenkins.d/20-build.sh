#!/usr/bin/env bash
set -x

git submodule init
git submodule sync
git submodule update

# actual build
./waf distclean --color=yes

COVERAGE=`python -c "print '--with-coverage' if 'code-coverage' in '$JOB_NAME' else ''"`

CXXFLAGS="-Wall -Wno-long-long -O2 -g -Werror" ./waf configure --color=yes --with-tests $COVERAGE

./waf -j1 --color=yes
sudo ./waf install --color=yes
