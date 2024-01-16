#!/bin/bash

if ! [ -f cppcode/build/cppex ];
then
    mkdir -p cppcode/build
    cd cppcode/build/
    cmake .. -DSCIP_DIR=/home/krunal/scipoptsuite-8.0.2/build/scip/
    make
    cd ../../
fi

./cppcode/build/cppex $1