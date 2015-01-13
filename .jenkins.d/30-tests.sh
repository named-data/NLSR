#!/usr/bin/env bash
set -x
set -e

rm -Rf ~/.ndn

./build/unit-tests-nlsr -l test_suite
