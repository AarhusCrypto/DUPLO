#!/bin/sh
git submodule update --init --recursive
cmake -DCMAKE_BUILD_TYPE=RELEASE .
make