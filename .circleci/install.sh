#!/bin/sh

. ./.circleci/common.subr

export OPENMPT_COMMIT=565602b7fde099344ea9145ea855c3072636e8b7
export OPENMPT_MAKEARGS="EXAMPLES=0 OPENMPT123=0 TEST=0"

case "$1" in
    mac|linux)
        exec ./.circleci/$1/install.sh
    ;;
    *)
        print_msg red "unsupported OS $1"
        exit 1
    ;;
esac
