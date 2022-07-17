#!/bin/bash

CWD=`pwd`

# gRPC C++ 1.47.0 is the first release requiring C++14
#GRPC_RELEASE=v1.47.1 # Requires C++14 at least.
GRPC_RELEASE=v1.46.3

if [ -d .git ] ; then

    git checkout master && git pull origin master \
        && git submodule update --init --recursive \
        && git submodule update --init --recursive --remote -- 3rdparty/portable-target \
        && git submodule update --init --recursive -- 3rdparty/grpc \
        && cd 3rdparty/grpc && git checkout $GRPC_RELEASE \
        && git submodule update --init -- third_party/re2 \
        && cd $CWD

fi

