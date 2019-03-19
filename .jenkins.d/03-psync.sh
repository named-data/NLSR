#!/usr/bin/env bash
set -e

JDIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source "$JDIR"/util.sh

set -x

pushd "${CACHE_DIR:-/tmp}" >/dev/null

INSTALLED_VERSION=
NDN_CXX=$(ndnsec version)
OLD_NDN_CXX=$(cat ndn_cxx_psync.txt || :)
if [[ $OLD_NDN_CXX != $NDN_CXX ]]; then
    echo "$NDN_CXX" > ndn_cxx_psync.txt
    INSTALLED_VERSION=NONE
fi

if [[ -z $INSTALLED_VERSION ]]; then
    INSTALLED_VERSION=$(git -C PSync rev-parse HEAD 2>/dev/null || echo NONE)
fi

sudo rm -Rf PSync-latest

git clone --depth 1 git://github.com/named-data/PSync PSync-latest

LATEST_VERSION=$(git -C PSync-latest rev-parse HEAD 2>/dev/null || echo UNKNOWN)

if [[ $INSTALLED_VERSION != $LATEST_VERSION ]]; then
    sudo rm -Rf PSync
    mv PSync-latest PSync
else
    sudo rm -Rf PSync-latest
fi

sudo rm -fr /usr/local/include/PSync
sudo rm -f /usr/local/lib{,64}/libPSync*
sudo rm -f /usr/local/lib{,64}/pkgconfig/PSync.pc

pushd PSync >/dev/null

if has FreeBSD10 $NODE_LABELS; then
    export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/
fi

./waf configure --color=yes
./waf build --color=yes -j${WAF_JOBS:-1}
sudo_preserve_env PATH -- ./waf install --color=yes

popd >/dev/null
popd >/dev/null

if has Linux $NODE_LABELS; then
    sudo ldconfig
elif has FreeBSD10 $NODE_LABELS; then
    sudo ldconfig -m
fi
