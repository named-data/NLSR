#!/usr/bin/env bash
set -x
set -e

sudo apt-get -y install liblog4cxx10-dev protobuf-compiler libprotobuf-dev pkg-config || true

# Disabled because OSX 10.8 requires special handling of dependencies
# brew install log4cxx protobuf pkg-config || true
