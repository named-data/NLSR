#!/usr/bin/env bash
set -e

JDIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source "$JDIR"/util.sh

set -x

pushd /tmp >/dev/null

INSTALLED_VERSION=
if has OSX $NODE_LABELS; then
    BOOST=$(brew ls --versions boost)
    OLD_BOOST=$(cat boost.txt || :)
    if [[ $OLD_BOOST != $BOOST ]]; then
        echo "$BOOST" > boost.txt
        INSTALLED_VERSION=NONE
    fi
fi

## Uncomment when #3920 and #4119 merge.
# if [[ -z $INSTALLED_VERSION ]]; then
#     INSTALLED_VERSION=$((cd ndn-cxx && git rev-parse HEAD) 2>/dev/null || echo NONE)
# fi

sudo rm -Rf ndn-cxx-latest
## Remove this when #3920 and #4119 merge
sudo rm -Rf ndn-cxx-hotfix
git clone git://github.com/named-data/ndn-cxx ndn-cxx-hotfix


## Uncomment when #3920 and #4119 merge.
# LATEST_VERSION=$((cd ndn-cxx-latest && git rev-parse HEAD) 2>/dev/null || echo UNKNOWN)

# if [[ $INSTALLED_VERSION != $LATEST_VERSION ]]; then
#     sudo rm -Rf ndn-cxx
#     mv ndn-cxx-latest ndn-cxx
#     cp ndn-cxx ndn-cxx-hotfix
# else
#     sudo rm -Rf ndn-cxx-latest
# fi

sudo rm -f /usr/local/bin/ndnsec*
sudo rm -fr /usr/local/include/ndn-cxx
sudo rm -f /usr/local/lib/libndn-cxx*
sudo rm -f /usr/local/lib/pkgconfig/libndn-cxx.pc

## Change to the hotfix directory instead of the normal ndn-cxx directory
## Restore below line when #3920 and #4119 merge.
#pushd ndn-cxx >/dev/null
pushd ndn-cxx-hotfix >/dev/null
git checkout b555b00c280b9c9ed46f24a1fbebc73b720601af

./waf configure -j1 --color=yes --enable-shared --disable-static --without-osx-keychain
./waf -j1 --color=yes
sudo env "PATH=$PATH" ./waf install --color=yes

popd >/dev/null
popd >/dev/null

if has Linux $NODE_LABELS; then
    sudo ldconfig
elif has FreeBSD10 $NODE_LABELS; then
    sudo ldconfig -m
fi
