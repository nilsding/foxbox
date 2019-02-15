#!/bin/sh

. ./.travis/common.subr

export OPENMPT_COMMIT=565602b7fde099344ea9145ea855c3072636e8b7
export OPENMPT_MAKEARGS="EXAMPLES=0 OPENMPT123=0 TEST=0"

case "$TRAVIS_OS_NAME" in
    osx|linux)
        exec ./.travis/$TRAVIS_OS_NAME/install.sh
    ;;
    *)
        print_msg red "unsupported OS $TRAVIS_OS_NAME"
        exit 1
    ;;
esac
