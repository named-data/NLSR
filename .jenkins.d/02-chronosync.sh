#!/usr/bin/env bash
set -exo pipefail

PROJ=ChronoSync

pushd "$CACHE_DIR" >/dev/null

INSTALLED_VERSION=
NDN_CXX=$(ndnsec version)
OLD_NDN_CXX=$(cat "$PROJ-ndn-cxx.txt" || :)
if [[ $OLD_NDN_CXX != $NDN_CXX ]]; then
    echo "$NDN_CXX" > "$PROJ-ndn-cxx.txt"
    INSTALLED_VERSION=NONE
fi

if [[ -z $INSTALLED_VERSION ]]; then
    INSTALLED_VERSION=$(git -C "$PROJ" rev-parse HEAD 2>/dev/null || echo NONE)
fi

sudo rm -rf "$PROJ-latest"
git clone --depth 1 "https://github.com/named-data/$PROJ.git" "$PROJ-latest"
LATEST_VERSION=$(git -C "$PROJ-latest" rev-parse HEAD 2>/dev/null || echo UNKNOWN)

if [[ $INSTALLED_VERSION != $LATEST_VERSION ]]; then
    sudo rm -rf "$PROJ"
    mv "$PROJ-latest" "$PROJ"
else
    sudo rm -rf "$PROJ-latest"
fi

sudo rm -fr /usr/local/include/"$PROJ"
sudo rm -f /usr/local/lib{,64}/lib"$PROJ"*
sudo rm -f /usr/local/lib{,64}/pkgconfig/"$PROJ".pc

pushd "$PROJ" >/dev/null

./waf --color=yes configure
./waf --color=yes build
sudo ./waf --color=yes install

popd >/dev/null
popd >/dev/null

if [[ $ID_LIKE == *linux* ]]; then
    if [[ $(uname -m) == x86_64 && -d /usr/lib64 ]]; then
        sudo tee /etc/ld.so.conf.d/ndn.conf >/dev/null <<< /usr/local/lib64
    fi
    sudo ldconfig
fi
