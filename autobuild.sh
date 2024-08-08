#! /bin/bash

set -x
rm -rf $(pwd)/build/*
cd $(pwd)/build &&
    cmake -DCMAKE_BUILD_TYPE=DEBUG .. &&
    make -j 10
cd ..
cp -r $(pwd)/src/include $(pwd)/lib