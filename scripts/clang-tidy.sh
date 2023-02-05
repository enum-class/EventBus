#! /bin/bash

run-clang-tidy -fix -p Debug -extra-arg=-std=c++20 -extra-arg=-Wno-unknown-warning-option `git diff --name-only origin/master`
