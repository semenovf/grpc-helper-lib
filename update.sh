#!/bin/bash

CWD=`pwd`

if [ -d .git ] ; then

    git pull \
        && git submodule update --init -- 3rdparty/portable-target \
        && git submodule update --remote -- 3rdparty/portable-target \
        && cd 3rdparty/portable-target && git checkout master && git pull \
        && cd $CWD \

fi

