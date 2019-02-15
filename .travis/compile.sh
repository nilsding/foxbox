#!/bin/sh

. ./.travis/common.subr

case "$TRAVIS_OS_NAME" in
    osx|linux)
        print_msg green "Compiling for $TRAVIS_OS_NAME"
        exec ./.travis/$TRAVIS_OS_NAME/compile.sh
        ;;
    *)
        print_msg red "unsupported OS $TRAVIS_OS_NAME"
        exit 1
        ;;
esac
