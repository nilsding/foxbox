#!/bin/sh

. ./.circleci/common.subr

print_msg green "Installing build-time dependencies"
forcecmd brew install qt libao

print_msg green "Installing libopenmpt from source"
forcecmd git clone https://github.com/OpenMPT/openmpt.git /tmp/openmpt
(
    cd /tmp/openmpt
    forcecmd git checkout "$OPENMPT_COMMIT"

    forcecmd make -j2 $OPENMPT_MAKEARGS
    forcecmd make install $OPENMPT_MAKEARGS
)
