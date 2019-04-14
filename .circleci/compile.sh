#!/bin/sh

. ./.circleci/common.subr

case "$1" in
    mac|linux)
        print_msg green "Compiling for $1"
        exec ./.circleci/$1/compile.sh
        ;;
    *)
        print_msg red "unsupported OS $1"
        exit 1
        ;;
esac
