#!/bin/sh

. ./.travis/common.subr

export PATH="/usr/local/opt/qt/bin:$PATH"
qmake --version

mkdir -p build
(
    cd build

    forcecmd qmake ..
    forcecmd make -j4
)
