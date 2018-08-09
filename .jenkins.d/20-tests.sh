#!/usr/bin/env bash
set -e
set -x

# Prepare environment
rm -Rf ~/.ndn

ut_log_args() {
    echo --log_level=test_suite
}

ASAN_OPTIONS="color=always"
ASAN_OPTIONS+=":detect_leaks=false" #4682
ASAN_OPTIONS+=":detect_stack_use_after_return=true"
ASAN_OPTIONS+=":check_initialization_order=true"
ASAN_OPTIONS+=":strict_init_order=true"
ASAN_OPTIONS+=":detect_invalid_pointer_pairs=1"
ASAN_OPTIONS+=":detect_container_overflow=false"
ASAN_OPTIONS+=":strict_string_checks=true"
ASAN_OPTIONS+=":strip_path_prefix=${PWD}/"
export ASAN_OPTIONS

# Run unit tests
./build/unit-tests-nlsr $(ut_log_args)
