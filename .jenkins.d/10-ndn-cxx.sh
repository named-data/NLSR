#!/usr/bin/env bash
set -x
set -e

JDIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source "$JDIR"/util.sh

pushd /tmp >/dev/null

NDN_CXX_FOLDER=ndn-cxx-0.4.0-beta2-11-ge3e2505

sudo rm -Rf "$NDN_CXX_FOLDER"

sudo rm -Rf /usr/local/include/ndn-cxx
sudo rm -f /usr/local/lib/libndn-cxx*
sudo rm -f /usr/local/lib/pkgconfig/libndn-cxx*

mkdir "$NDN_CXX_FOLDER"
pushd "$NDN_CXX_FOLDER" > /dev/null

git init
git fetch https://github.com/named-data/ndn-cxx master && git checkout e3e2505aa03e0b298e1a8dfc9876f1f8dafcaaba

./waf configure -j1 --color=yes --enable-shared --disable-static --without-osx-keychain
./waf -j1 --color=yes
sudo ./waf install -j1 --color=yes

popd >/dev/null
popd >/dev/null

if has Linux $NODE_LABELS; then
    sudo ldconfig
elif has FreeBSD $NODE_LABELS; then
    sudo ldconfig -a
fi
