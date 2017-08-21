#!/usr/bin/env bash
set -e

JDIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source "$JDIR"/util.sh

set -x

pushd "${CACHE_DIR:-/tmp}" >/dev/null

INSTALLED_VERSION=
if has OSX $NODE_LABELS; then
    BOOST=$(brew ls --versions boost)
    OLD_BOOST=$(cat boost_chrono.txt || :)
    if [[ $OLD_BOOST != $BOOST ]]; then
        echo "$BOOST" > boost_chrono.txt
        INSTALLED_VERSION=NONE
    fi
fi

if [[ -z $INSTALLED_VERSION ]]; then
    INSTALLED_VERSION=$(git -C ChronoSync rev-parse HEAD 2>/dev/null || echo NONE)
fi

sudo rm -Rf ChronoSync-latest

git clone --depth 1 git://github.com/named-data/ChronoSync ChronoSync-latest

LATEST_VERSION=$(git -C ChronoSync-latest rev-parse HEAD 2>/dev/null || echo UNKNOWN)

if [[ $INSTALLED_VERSION != $LATEST_VERSION ]]; then
    sudo rm -Rf ChronoSync
    mv ChronoSync-latest ChronoSync
else
    sudo rm -Rf ChronoSync-latest
fi

sudo rm -fr /usr/local/include/ChronoSync
sudo rm -f /usr/local/lib/libChronoSync*
sudo rm -f /usr/local/lib/pkgconfig/ChronoSync*

pushd ChronoSync >/dev/null

if has FreeBSD10 $NODE_LABELS; then
    export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/
fi

./waf configure --color=yes
./waf build --color=yes -j${WAF_JOBS:-1}
sudo env "PATH=$PATH" ./waf install --color=yes

popd >/dev/null
popd >/dev/null

if has Linux $NODE_LABELS; then
    sudo ldconfig
elif has FreeBSD10 $NODE_LABELS; then
    sudo ldconfig -m
fi
