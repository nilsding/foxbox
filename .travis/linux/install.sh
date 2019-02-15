#!/bin/sh

. ./.travis/common.subr

print_msg green "Installing build-time dependencies"
forcecmd sudo apt update
forcecmd sudo apt install -y qt5-default qt5-qmake libao-dev

print_msg green "Installing libopenmpt from source"
forcecmd git clone https://github.com/OpenMPT/openmpt.git /tmp/openmpt
(
    cd /tmp/openmpt
    forcecmd git checkout "$OPENMPT_COMMIT"

    forcecmd make -j2 $OPENMPT_MAKEARGS
    forcecmd sudo make install $OPENMPT_MAKEARGS
)
