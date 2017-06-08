#!/usr/bin/env bash
set -x
set -e

JDIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source "$JDIR"/util.sh

pushd /tmp >/dev/null

INSTALLED_VERSION=
if has OSX $NODE_LABELS; then
    BOOST=$(brew ls --versions boost)
    OLD_BOOST=$(cat boost_chrono.txt || :)
    if [[ $OLD_BOOST != $BOOST ]]; then
        echo "$BOOST" > boost_chrono.txt
        INSTALLED_VERSION=NONE
    fi
fi

# if [[ -z $INSTALLED_VERSION ]]; then
#     INSTALLED_VERSION=$((cd ChronoSync && git rev-parse HEAD) 2>/dev/null || echo NONE)
# fi

sudo rm -Rf ChronoSync-latest
## Remove line when #3920 and #4119 merge.
if [ ! -d "ChronoSync-hotfix" ]; then
    git clone git://github.com/named-data/ChronoSync ChronoSync-hotfix
fi

# LATEST_VERSION=$((cd ChronoSync-latest && git rev-parse HEAD) 2>/dev/null || echo UNKNOWN)

# if [[ $INSTALLED_VERSION != $LATEST_VERSION ]]; then
#     sudo rm -Rf ChronoSync
#     mv ChronoSync-latest ChronoSync
# else
#     sudo rm -Rf ChronoSync-latest
# fi

sudo rm -Rf /usr/local/include/ChronoSync
sudo rm -f /usr/local/lib/libChronoSync*
sudo rm -f /usr/local/lib/pkgconfig/ChronoSync*

pushd ChronoSync-hotfix >/dev/null
git checkout 097bb448f46b8bd9a5c1f431e824f8f6a169b650

if has FreeBSD10 $NODE_LABELS; then
    export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/
fi

./waf -j1 --color=yes configure
./waf -j1 --color=yes build
sudo ./waf install -j1 --color=yes

popd >/dev/null
popd >/dev/null

if has Linux $NODE_LABELS; then
    sudo ldconfig
elif has FreeBSD10 $NODE_LABELS; then
    sudo ldconfig -m
fi
