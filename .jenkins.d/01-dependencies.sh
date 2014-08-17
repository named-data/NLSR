#!/usr/bin/env bash
set -x

sudo apt-get -y install liblog4cxx10-dev protobuf-compiler libprotobuf-dev pkg-config || true
sudo brew update || true
brew install log4cxx protobuf pkg-config || true

