#!/bin/sh

. ./.circleci/common.subr

qmake --version

mkdir -p build
(
    cd build

    forcecmd qmake ..
    forcecmd make -j4
)
